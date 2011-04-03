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
    const void *lines, const void *characters) __attribute__((nonnull));

extern "C" void LXiStream_SSubtitleRenderNode_mixSubtitle8_stretch(
    void * srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
    float srcAspect, void * srcU, unsigned uStride, void * srcV,
    unsigned vStride, unsigned wf, unsigned hf,
    const void *lines, const void *characters) __attribute__((nonnull));

extern "C" void LXiStream_SSubtitleRenderNode_mixSubtitle32(
    void * srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
    const void *lines, const void *characters) __attribute__((nonnull));

extern "C" void LXiStream_SSubtitleRenderNode_mixSubtitle32_stretch(
    void * srcData, unsigned srcStride, unsigned srcWidth, unsigned srcHeight,
    float srcAspect,
    const void *lines, const void *characters) __attribute__((nonnull));

namespace LXiStream {

// Keep these structures in sync with the ones defined in ssubtitlerendernode.mix.c
struct SSubtitleRenderNode::Lines
{
  quint8                      l[4][160];
};

struct SSubtitleRenderNode::Char
{
  unsigned                    advance, width, height;
  quint8                      pixels[0];
};

const unsigned char SSubtitleRenderNode::subFontsData[] = {
#include "subtitlefont.h"
};

QMap<int, QVector<SSubtitleRenderNode::Char *> > SSubtitleRenderNode::characters;
SSubtitleRenderNode::FontLoader SSubtitleRenderNode::fontLoader;

struct SSubtitleRenderNode::Data
{
  SScheduler::Dependency      * dependency;
  QMutex                        mutex;
  unsigned                      ratio;
  volatile bool                 enabled;
  QMap<STime, SSubtitleBuffer>  subtitles;
  QMap<int, QVector<Char *> >::ConstIterator font;
  Lines                       * subtitle;
  STime                         subtitleTime;
  bool                          subtitleVisible;
};

SSubtitleRenderNode::SSubtitleRenderNode(SGraph *parent)
  : QObject(parent),
    SGraph::Node(parent),
    d(new Data())
{
  // loadFont() should be invoked before a SSubtitleRenderNode can be constructed.
  Q_ASSERT(!characters.isEmpty());

  d->dependency = parent ? new SScheduler::Dependency(parent) : NULL;
  d->ratio = 16;
  d->enabled = false;
  d->font = characters.end();
  d->subtitle = new Lines();
  d->subtitleVisible = false;
}

SSubtitleRenderNode::~SSubtitleRenderNode()
{
  delete d->dependency;
  delete d->subtitle;
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

void SSubtitleRenderNode::input(const SSubtitleBuffer &subtitleBuffer)
{
  if (!subtitleBuffer.isNull())
    schedule(this, &SSubtitleRenderNode::processTask, subtitleBuffer, d->dependency);
}

void SSubtitleRenderNode::input(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull() && d->enabled)
    schedule(this, &SSubtitleRenderNode::processTask, videoBuffer, d->dependency);
  else
    emit output(videoBuffer);
}

SVideoBuffer SSubtitleRenderNode::renderSubtitles(const SVideoBuffer &videoBuffer, const QStringList &lines, unsigned ratio)
{
  QMap<int, QVector<Char *> >::ConstIterator font =
      characters.lowerBound(videoBuffer.format().size().height() / ratio);
  if (font == characters.end())
    font--;

  Lines subtitle;
  memset(&subtitle, 0, sizeof(subtitle));
  for (int j=0; (j<lines.count()) && (j<4); j++)
  {
    QString line = lines[j];
    if (line.contains("<i>", Qt::CaseInsensitive))
      font = characters.find(-(font->first()->height));

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

void SSubtitleRenderNode::processTask(const SSubtitleBuffer &subtitleBuffer)
{
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

void SSubtitleRenderNode::processTask(const SVideoBuffer &videoBuffer)
{
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
      d->font = characters.lowerBound(videoBuffer.format().size().height() / d->ratio);
      if (d->font == characters.end())
        d->font--;

      const QStringList lines = i->subtitle();

      memset(d->subtitle, 0, sizeof(*d->subtitle));
      for (int j=0; (j<lines.count()) && (j<4); j++)
      {
        QString line = lines[j];
        if (line.contains("<i>", Qt::CaseInsensitive))
          d->font = characters.find(-(d->font->first()->height));

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


SSubtitleRenderNode::FontLoader::FontLoader(void)
{
  foreach (const QByteArray &fontData,
           qUncompress(QByteArray::fromRawData((const char *)subFontsData, sizeof(subFontsData))).split(':'))
  {
    const QList<QByteArray> elms = fontData.split(';');
    if (elms.count() >= 6)
    {
      const unsigned width = elms[3].toUInt(), height = elms[4].toUInt();
      Char * const c = reinterpret_cast<Char *>(new quint8[sizeof(Char) + (width * height * sizeof(quint8))]);

      c->advance = elms[2].toUInt();
      c->width = width;
      c->height = height;

      for (unsigned i=0, n=width*height; i<n; i++)
      {
        const char e = elms[5][i];
        c->pixels[i] = (e != ' ') ? ((quint8(e - '0') * 28) + 3) : 0;
      }

      const int fid = (elms[1].toInt() == 0) ? c->height : -c->height;
      QMap<int, QVector<SSubtitleRenderNode::Char *> >::Iterator i = characters.find(fid);
      if (i == characters.end())
      {
        QVector<SSubtitleRenderNode::Char *> v;
        v.reserve(256);
        v.resize(256);
        memset(&(v.first()), 0, 256 * sizeof(Char *));
        i = characters.insert(fid, v);
      }

      const int ce = elms[0].toInt();
      if (i->size() > ce)
        (*i)[ce] = c;
    }
  }
}

SSubtitleRenderNode::FontLoader::~FontLoader()
{
  // This code may crash on old Qt implementations. The only reason to clean
  // this up is for memory leak tests, so it can be safely skipped.
#if QT_VERSION >= 0x040600
  for (QMap<int, QVector<Char *> >::Iterator i=characters.begin(); i!=characters.end(); )
  {
    foreach (Char *c, *i)
      delete [] reinterpret_cast<quint8 *>(c);

    i = characters.erase(i);
  }
#endif
}


} // End of namespace
