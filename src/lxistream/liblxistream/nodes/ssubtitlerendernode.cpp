/***************************************************************************
 *   Copyright (C) 2010 by A.J. Admiraal                                   *
 *   code@admiraal.dds.nl                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include "nodes/ssubtitlerendernode.h"
#include "ssubtitlebuffer.h"
#include "svideobuffer.h"

// Implemented in ssubtitlerendernode.mix.c
extern "C" void LXiStream_SSubtitleRenderNode_mixSubtitle8(
    void * srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
    void * srcU, unsigned uStride, void * srcV, unsigned vStride,
    unsigned wf, unsigned hf,
    const void *lines, const void *characters);

extern "C" void LXiStream_SSubtitleRenderNode_mixSubtitle8_stretch(
    void * srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
    float srcAspect, void * srcU, unsigned uStride, void * srcV,
    unsigned vStride, unsigned wf, unsigned hf,
    const void *lines, const void *characters);

extern "C" void LXiStream_SSubtitleRenderNode_mixSubtitle32(
    void * srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
    const void *lines, const void *characters);

extern "C" void LXiStream_SSubtitleRenderNode_mixSubtitle32_stretch(
    void * srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
    float srcAspect,
    const void *lines, const void *characters);

#if defined(_MSC_VER)
#pragma warning (disable : 4200)
#endif

namespace LXiStream {

// Keep these structures in sync with the ones defined in ssubtitlerendernode.mix.c
struct SSubtitleRenderNode::Lines
{
  quint8                        l[4][160];
};

struct SSubtitleRenderNode::Char
{
  unsigned                      advance, width, height;
  quint8                        pixels[0];
};

struct SSubtitleRenderNode::Data
{
  static int                    instances;
  static QByteArray             fontData;
  static QMap<int, QVector<const Char *> > characters;

  unsigned                      ratio;
  volatile bool                 enabled;
  QMap<STime, SSubtitleBuffer>  subtitles;
  QMap<int, QVector<const Char *> >::ConstIterator font;
  Lines                       * subtitle;
  STime                         subtitleTime;
  bool                          subtitleVisible;

  QMutex                        mutex;
  QFuture<void>                 future;
};

int SSubtitleRenderNode::Data::instances = 0;
QByteArray SSubtitleRenderNode::Data::fontData;
QMap<int, QVector<const SSubtitleRenderNode::Char *> > SSubtitleRenderNode::Data::characters;

SSubtitleRenderNode::SSubtitleRenderNode(SGraph *parent)
  : QObject(parent),
    SGraph::Node(parent),
    d(new Data())
{
  if (d->instances++ <= 0)
    loadFonts();

  Q_ASSERT(d->instances > 0);
  Q_ASSERT(!d->fontData.isEmpty());
  Q_ASSERT(!d->characters.isEmpty());

  d->ratio = 16;
  d->enabled = false;
  d->font = d->characters.end();
  d->subtitle = new Lines();
  d->subtitleVisible = false;
}

SSubtitleRenderNode::~SSubtitleRenderNode()
{
  d->future.waitForFinished();
  delete d->subtitle;

  if (--d->instances <= 0)
  {
    d->fontData.clear();
    d->characters.clear();
  }

  Q_ASSERT(d->instances >= 0);

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

unsigned SSubtitleRenderNode::fontRatio(void) const
{
  return d->ratio;
}

void SSubtitleRenderNode::setFontRatio(unsigned r)
{
  d->ratio = r;
}

bool SSubtitleRenderNode::start(void)
{
  return true;
}

void SSubtitleRenderNode::stop(void)
{
  d->future.waitForFinished();
}

void SSubtitleRenderNode::input(const SSubtitleBuffer &subtitleBuffer)
{
  LXI_PROFILE_FUNCTION;

  QMutexLocker l(&d->mutex);

  if (subtitleBuffer.duration().toSec() <= 10)
  {
    d->subtitles.insert(subtitleBuffer.timeStamp(), subtitleBuffer);
  }
  else // Prevent showing subtitles too long.
  {
    SSubtitleBuffer corrected = subtitleBuffer;
    corrected.setDuration(STime::fromSec(10));
    d->subtitles.insert(subtitleBuffer.timeStamp(), corrected);
  }

  d->enabled = true;
}

void SSubtitleRenderNode::input(const SVideoBuffer &videoBuffer)
{
  LXI_PROFILE_FUNCTION;

  d->future.waitForFinished();

  if (!videoBuffer.isNull() && d->enabled)
    d->future = QtConcurrent::run(this, &SSubtitleRenderNode::processTask, videoBuffer);
  else
    emit output(videoBuffer);
}

SVideoBuffer SSubtitleRenderNode::renderSubtitles(const SVideoBuffer &videoBuffer, const QStringList &lines, unsigned ratio)
{
  QMap<int, QVector<const Char *> >::ConstIterator font =
      Data::characters.lowerBound(videoBuffer.format().size().height() / ratio);
  if (font == Data::characters.end())
    font--;

  Lines subtitle;
  memset(&subtitle, 0, sizeof(subtitle));
  for (int j=0; (j<lines.count()) && (j<4); j++)
  {
    QString line = lines[j];
    if (line.contains("<i>", Qt::CaseInsensitive))
      font = Data::characters.find(-int(font->first()->height));

    line.replace("<i>", "", Qt::CaseInsensitive);
    line.replace("</i>", "", Qt::CaseInsensitive);
    line.replace("<b>", "", Qt::CaseInsensitive);
    line.replace("</b>", "", Qt::CaseInsensitive);
    line.replace("<u>", "", Qt::CaseInsensitive);
    line.replace("</u>", "", Qt::CaseInsensitive);

    const int ln = j + qMax(0, 4 - lines.count());
    const QByteArray latin1 = line.toLatin1();
    memset(subtitle.l[ln], 0, sizeof(subtitle.l[ln]));
    qstrncpy(reinterpret_cast<char *>(subtitle.l[ln]),
             latin1.data(),
             sizeof(subtitle.l[ln]));
  }

  SVideoBuffer buffer = videoBuffer;
  renderSubtitles(buffer, &subtitle, &(font->first()));

  return buffer;
}

void SSubtitleRenderNode::processTask(const SVideoBuffer &videoBuffer)
{
  LXI_PROFILE_FUNCTION;

  QMutexLocker l(&d->mutex);

  for (QMap<STime, SSubtitleBuffer>::Iterator i=d->subtitles.begin(); i!=d->subtitles.end(); )
  {
    const STime timeStamp = videoBuffer.timeStamp();

    // Can the current subtitle be removed.
    if (d->subtitleVisible && ((i.key() + i->duration()) < timeStamp))
    {
      i = d->subtitles.erase(i);
      d->subtitleTime = STime();
      d->subtitleVisible = false;
      continue;
    }

    // Can the next subtitle be displayed yet.
    QMap<STime, SSubtitleBuffer>::Iterator n = i + 1;
    if (n != d->subtitles.end())
    if ((n.key() != d->subtitleTime) && (n.key() <= timeStamp))
    {
      i = d->subtitles.erase(i);
      d->subtitleTime = STime();
      d->subtitleVisible = false;
      continue;
    }

    // Render the next subtitle.
    if ((i.key() != d->subtitleTime) && (i.key() <= timeStamp))
    {
      d->font = d->characters.lowerBound(videoBuffer.format().size().height() / d->ratio);
      if (d->font == d->characters.end())
        d->font--;

      const QStringList lines = i->subtitle();

      memset(d->subtitle, 0, sizeof(*d->subtitle));
      for (int j=0; (j<lines.count()) && (j<4); j++)
      {
        QString line = lines[j];
        if (line.contains("<i>", Qt::CaseInsensitive))
          d->font = d->characters.find(-int(d->font->first()->height));

        line.replace("<i>", "", Qt::CaseInsensitive);
        line.replace("</i>", "", Qt::CaseInsensitive);
        line.replace("<b>", "", Qt::CaseInsensitive);
        line.replace("</b>", "", Qt::CaseInsensitive);
        line.replace("<u>", "", Qt::CaseInsensitive);
        line.replace("</u>", "", Qt::CaseInsensitive);

        const int ln = j + qMax(0, 4 - lines.count());
        const QByteArray latin1 = line.toLatin1();
        memset(d->subtitle->l[ln], 0, sizeof(d->subtitle->l[ln]));
        qstrncpy(reinterpret_cast<char *>(d->subtitle->l[ln]),
                 latin1.data(),
                 sizeof(d->subtitle->l[ln]));
      }

      d->subtitleVisible = true;
      d->subtitleTime = i.key();
    }

    break;
  }

  if (d->subtitleVisible)
  {
    SVideoBuffer buffer = videoBuffer;
    renderSubtitles(buffer, d->subtitle, &(d->font->first()));

    emit output(buffer);
  }
  else
    emit output(videoBuffer);
}

void SSubtitleRenderNode::renderSubtitles(SVideoBuffer &buffer, const Lines *subtitle, const Char * const *font)
{
  const SSize size = buffer.format().size();

  if (buffer.format().numPlanes() >= 3)
  {
    int wf = 1, hf = 1;
    buffer.format().planarYUVRatio(wf, hf);

    if (qFuzzyCompare(size.aspectRatio(), 1.0f))
    {
      LXiStream_SSubtitleRenderNode_mixSubtitle8(
          buffer.data() + buffer.offset(0),
          buffer.lineSize(0), size.width(), size.height(),
          buffer.data() + buffer.offset(1), buffer.lineSize(1),
          buffer.data() + buffer.offset(2), buffer.lineSize(2),
          wf, hf, subtitle, font);
    }
    else
    {
      LXiStream_SSubtitleRenderNode_mixSubtitle8_stretch(
          buffer.data() + buffer.offset(0),
          buffer.lineSize(0), size.width(), size.height(), size.aspectRatio(),
          buffer.data() + buffer.offset(1), buffer.lineSize(1),
          buffer.data() + buffer.offset(2), buffer.lineSize(2),
          wf, hf, subtitle, font);
    }
  }
  else if (buffer.format().sampleSize() == sizeof(quint32))
  {
    if (qFuzzyCompare(size.aspectRatio(), 1.0f))
    {
      LXiStream_SSubtitleRenderNode_mixSubtitle32(
          buffer.data() + buffer.offset(0),
          buffer.lineSize(0) / sizeof(quint32), size.width(), size.height(),
          subtitle, font);
    }
    else
    {
      LXiStream_SSubtitleRenderNode_mixSubtitle32_stretch(
          buffer.data() + buffer.offset(0),
          buffer.lineSize(0) / sizeof(quint32), size.width(), size.height(), size.aspectRatio(),
          subtitle, font);
    }
  }
}

/*! This loads the pre-rendered "DejaVu Sans" characters, see
    http://dejavu-fonts.org/ for more information on this font.

Copyright (c) 2003 by Bitstream, Inc. All Rights Reserved. Bitstream Vera is a
trademark of Bitstream, Inc. Permission is hereby granted, free of charge, to
any person obtaining a copy of the fonts accompanying this license ("Fonts") and
associated documentation files (the "Font Software"), to reproduce and
distribute the Font Software, including without limitation the rights to use,
copy, merge, publish, distribute, and/or sell copies of the Font Software, and
to permit persons to whom the Font Software is furnished to do so, subject to
the following conditions:
The above copyright and trademark notices and this permission notice shall be
included in all copies of one or more of the Font Software typefaces. The Font
Software may be modified, altered, or added to, and in particular the designs of
glyphs or characters in the Fonts may be modified and additional glyphs or
characters may be added to the Fonts, only if the fonts are renamed to names not
containing either the words "Bitstream" or the word "Vera".
This License becomes null and void to the extent applicable to Fonts or Font
Software that has been modified and is distributed under the "Bitstream Vera"
names. The Font Software may be sold as part of a larger software package but no
copy of one or more of the Font Software typefaces may be sold by itself.

THE FONT SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF COPYRIGHT, PATENT, TRADEMARK, OR
OTHER RIGHT. IN NO EVENT SHALL BITSTREAM OR THE GNOME FOUNDATION BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, INCLUDING ANY GENERAL, SPECIAL, INDIRECT,
INCIDENTAL, OR CONSEQUENTIAL DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR
FROM OTHER DEALINGS IN THE FONT SOFTWARE.

Except as contained in this notice, the names of Gnome, the Gnome Foundation,
and Bitstream Inc., shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Font Software without prior written
authorization from the Gnome Foundation or Bitstream Inc., respectively. For
further information, contact: fonts at gnome dot org.

Copyright (c) 2006 by Tavmjong Bah. All Rights Reserved. Permission is hereby
granted, free of charge, to any person obtaining a copy of the fonts
accompanying this license ("Fonts") and associated documentation files (the
"Font Software"), to reproduce and distribute the modifications to the
Bitstream Vera Font Software, including without limitation the rights to use,
copy, merge, publish, distribute, and/or sell copies of the Font Software, and
to permit persons to whom the Font Software is furnished to do so, subject to
the following conditions:
The above copyright and trademark notices and this permission notice shall be
included in all copies of one or more of the Font Software typefaces. The Font
Software may be modified, altered, or added to, and in particular the designs of
glyphs or characters in the Fonts may be modified and additional glyphs or
characters may be added to the Fonts, only if the fonts are renamed to names not
containing either the words "Tavmjong Bah" or the word "Arev".
This License becomes null and void to the extent applicable to Fonts or Font
Software that has been modified and is distributed under the "Tavmjong Bah Arev"
names. The Font Software may be sold as part of a larger software package but no
copy of one or more of the Font Software typefaces may be sold by itself.

THE FONT SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF COPYRIGHT, PATENT, TRADEMARK, OR
OTHER RIGHT. IN NO EVENT SHALL TAVMJONG BAH BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, INCLUDING ANY GENERAL, SPECIAL, INDIRECT, INCIDENTAL, OR
CONSEQUENTIAL DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR FROM OTHER
DEALINGS IN THE FONT SOFTWARE.

Except as contained in this notice, the name of Tavmjong Bah shall not be used
in advertising or otherwise to promote the sale, use or other dealings in this
Font Software without prior written authorization from Tavmjong Bah. For further
information, contact: tavmjong @ free . fr.
*/
void SSubtitleRenderNode::loadFonts(void)
{
  QFile file(":/lxistream/nodes/subtitlefont.bin");
  if (file.open(QFile::ReadOnly))
  {
    Data::fontData = qUncompress(file.readAll());
    Data::characters.clear();

    if (!Data::fontData.isEmpty())
    {
      const char * const bytes = Data::fontData.constData();
      for (int pos=0, size=Data::fontData.size(); pos<size; )
      {
        const qint32 * const ce = reinterpret_cast<const qint32 *>(bytes + pos);
        const qint32 * const m = reinterpret_cast<const qint32 *>(bytes + pos + sizeof(qint32));
        const Char * const c = reinterpret_cast<const Char *>(bytes + pos + (sizeof(qint32) * 2));

        const int fid = (*m == 0) ? int(c->height) : -int(c->height);
        QMap<int, QVector<const SSubtitleRenderNode::Char *> >::Iterator i = Data::characters.find(fid);
        if (i == Data::characters.end())
        {
          QVector<const SSubtitleRenderNode::Char *> v;
          v.resize(256);
          memset(&(v.first()), 0, 256 * sizeof(Char *));
          i = Data::characters.insert(fid, v);
        }

        if (i->size() > *ce)
          (*i)[*ce] = c;

        pos += (sizeof(qint32) * 2) + sizeof(Char) + (c->width * c->height);
      }
    }
  }
}

} // End of namespace
