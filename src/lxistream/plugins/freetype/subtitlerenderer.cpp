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

#include "subtitlerenderer.h"
#include "../../algorithms/subtitles.h"

namespace LXiStream {
namespace FreeTypeBackend {

int        SubtitleRenderer::instanceCount = 0;
QByteArray SubtitleRenderer::faceData;

SubtitleRenderer::SubtitleRenderer(const QString &, QObject *parent)
  : SInterfaces::SubtitleRenderer(parent),
    ratio(0.05f),
    initialized(false),
    rendering(false)
{
  memset(&library, 0, sizeof(library));
  memset(&face, 0, sizeof(face));

  if (instanceCount++ == 0)
  {
    QFile file(":/freetype/LiberationSans-Bold.ttf");
    if (file.open(QFile::ReadOnly))
      faceData = file.readAll();
  }

  while (!faceData.isEmpty())
  {
    if (::FT_Init_FreeType(&library) == 0)
    {
      if (FT_New_Memory_Face(
              library,
              reinterpret_cast<const FT_Byte *>(faceData.data()),
              faceData.size(),
              0,
              &face) == 0)
      {
        initialized = true;
        break;
      }

      ::FT_Done_FreeType(library);
    }

    qWarning() << "Failed to initialize FreeType";
    break;
  }
}

SubtitleRenderer::~SubtitleRenderer()
{
  if (initialized)
  {
    ::FT_Done_Face(face);
    ::FT_Done_FreeType(library);
  }

  Q_ASSERT(instanceCount > 0);
  if (--instanceCount == 0)
    faceData.clear();
}

void SubtitleRenderer::setFontRatio(float r)
{
  ratio = r;
}

float SubtitleRenderer::fontRatio(void) const
{
  return ratio;
}

void SubtitleRenderer::prepareSubtitle(const SSubtitleBuffer &subtitleBuffer, const SSize &size)
{
  if (rendering)
    renderFuture.waitForFinished();

  renderFuture = QtConcurrent::run(this, &SubtitleRenderer::renderSubtitle, subtitleBuffer, size);
  rendering = true;
}

SVideoBuffer SubtitleRenderer::processBuffer(const SVideoBuffer &videoBuffer, const SSubtitleBuffer &subtitleBuffer)
{
  if (!videoBuffer.isNull() && !subtitleBuffer.isNull())
  {
    // Render pre-rendered subtitle.
    foreach (const RenderedSubtitle &renderedSubtitle, renderedSubtitles)
    if ((renderedSubtitle.uid == subtitleBuffer.memory()->uid) &&
        (renderedSubtitle.videoSize == videoBuffer.format().size()))
    {
      return blendSubtitle(videoBuffer, renderedSubtitle);
    }

    // Wait for subtitle rendering in the background.
    if (rendering)
    {
      const RenderedSubtitle renderedSubtitle = renderFuture.result();
      renderFuture = QFuture<RenderedSubtitle>();
      rendering = false;

      renderedSubtitles.enqueue(renderedSubtitle);
      while (renderedSubtitles.count() > 3)
        renderedSubtitles.dequeue();

      if ((renderedSubtitle.uid == subtitleBuffer.memory()->uid) &&
          (renderedSubtitle.videoSize == videoBuffer.format().size()))
      {
        return blendSubtitle(videoBuffer, renderedSubtitle);
      }
    }

    // Render subtitle now.
    const RenderedSubtitle renderedSubtitle =
        renderSubtitle(subtitleBuffer, videoBuffer.format().size());

    renderedSubtitles.enqueue(renderedSubtitle);
    while (renderedSubtitles.count() > 3)
      renderedSubtitles.dequeue();

    return blendSubtitle(videoBuffer,renderedSubtitle);
  }

  return videoBuffer;
}

SubtitleRenderer::RenderedSubtitle SubtitleRenderer::renderSubtitle(const SSubtitleBuffer &subtitleBuffer, const SSize &size)
{
  RenderedSubtitle renderedSubtitle;
  renderedSubtitle.uid = subtitleBuffer.memory()->uid;
  renderedSubtitle.margin = int((float(size.height()) * 0.05f) + 0.5f);
  renderedSubtitle.videoSize = size;

  if (initialized)
  {
    const float height = float(size.height()) * ratio;
    const float width = (height / size.aspectRatio()) * 0.85f;
    const int shadowHeight = qMax(1, int((height / 16.0f) + 0.5f));
    const int shadowWidth = qMax(1, int(((height / size.aspectRatio()) / 16.0f) + 0.5f));

    if (::FT_Set_Pixel_Sizes(face, int(width + 0.5f), int(height + 0.5f)) == 0)
    {
      const int lineHeight = (face->size->metrics.height + 31) >> 6;
      const int lineOffset = (-face->size->metrics.descender + 31) >> 6;
      const int charWidth = (face->size->metrics.max_advance + 31) >> 6;
      const QStringList lines = subtitleBuffer.subtitle();

      const SVideoFormat format(
          SVideoFormat::Format_GRAY8,
          SSize(size.width(), lineHeight + shadowHeight * 2));

      bool oblique = false, bold = false;
      foreach (QString line, lines)
      {
        RenderedSubtitle::Line renderedLine;
        renderedLine.height = lineHeight + shadowHeight;
        renderedLine.text = SVideoBuffer(format);
        memset(renderedLine.text.data(), 0, renderedLine.text.size());
        renderedLine.shadow = SVideoBuffer(format);
        memset(renderedLine.shadow.data(), 0, renderedLine.shadow.size());

        line.replace("<i>", "\a", Qt::CaseInsensitive);
        line.replace("</i>", "\b", Qt::CaseInsensitive);
        line.replace("<b>", "\t", Qt::CaseInsensitive);
        line.replace("</b>", "\n", Qt::CaseInsensitive);
        line.replace("<u>", "", Qt::CaseInsensitive);
        line.replace("</u>", "", Qt::CaseInsensitive);

        int xpos = ((charWidth / 2) + shadowWidth) << 6;
        ::FT_UInt previousGlyph = 0;

        foreach (const uint c, line.toUcs4())
        if (c >= ' ')
        {
          const FT_UInt glyph = ::FT_Get_Char_Index(face, c);

          if ((previousGlyph != 0) && (glyph != 0) && FT_HAS_KERNING(face))
          {
            ::FT_Vector delta;
            if (::FT_Get_Kerning(face, previousGlyph, glyph, FT_KERNING_DEFAULT, &delta) == 0)
              xpos += delta.x;
          }

          // The matrix to make the charachters italic.
          ::FT_Matrix matrix;
          matrix.xx = 0x10000 + (bold ? (0x10000 * 3 / 10) : 0);
          matrix.yy = 0x10000;
          matrix.xy = oblique ? (0x10000 * 3 / 10) : 0;
          matrix.yx = 0;

          // The vector for sub-pixel precise positioning.
          ::FT_Vector vector;
          vector.x = xpos & 63;
          vector.y = 0;

          ::FT_Set_Transform(face, &matrix, &vector);

          if (::FT_Load_Glyph(face, glyph, FT_LOAD_RENDER) == 0)
          {
            drawChar(
                renderedLine.text,
                face->glyph->bitmap,
                (xpos >> 6) + face->glyph->bitmap_left,
                (lineHeight + shadowHeight - lineOffset) - face->glyph->bitmap_top);

            for (int y=-shadowHeight; y<=shadowHeight; y+=shadowHeight*2)
            for (int x=-shadowWidth+1; x<shadowWidth; x++)
            {
              drawChar(
                  renderedLine.shadow,
                  face->glyph->bitmap,
                  (xpos >> 6) + x + face->glyph->bitmap_left,
                  (lineHeight + shadowHeight + y - lineOffset) - face->glyph->bitmap_top);
            }

            for (int y=-shadowHeight+1; y<shadowHeight; y++)
            for (int x=-shadowWidth; x<=shadowWidth; x+=shadowWidth*2)
            {
              drawChar(
                  renderedLine.shadow,
                  face->glyph->bitmap,
                  (xpos >> 6) + x + face->glyph->bitmap_left,
                  (lineHeight + shadowHeight + y - lineOffset) - face->glyph->bitmap_top);
            }

            xpos += face->glyph->advance.x;
          }

          previousGlyph = glyph;
        }
        else if (c == '\a')
          oblique = true;
        else if (c == '\b')
          oblique = false;
        else if (c == '\t')
          bold = true;
        else if (c == '\n')
          bold = false;

        renderedLine.width = ((xpos + 63) >> 6) + (charWidth / 2) + shadowWidth;
        renderedSubtitle.lines += renderedLine;
      }
    }
  }

  return renderedSubtitle;
}

SVideoBuffer SubtitleRenderer::blendSubtitle(const SVideoBuffer &videoBuffer, const RenderedSubtitle &renderedSubtitle)
{
  switch (videoBuffer.format().format())
  {
  case SVideoFormat::Format_Invalid:
    return videoBuffer;

  case SVideoFormat::Format_RGB555:
  case SVideoFormat::Format_BGR555:
  case SVideoFormat::Format_RGB565:
  case SVideoFormat::Format_BGR565:
  case SVideoFormat::Format_RGB24:
  case SVideoFormat::Format_BGR24:
  case SVideoFormat::Format_RGB32:
  case SVideoFormat::Format_BGR32:
  case SVideoFormat::Format_GRAY8:
  case SVideoFormat::Format_GRAY16BE:
  case SVideoFormat::Format_GRAY16LE:
    return videoBuffer;

  case SVideoFormat::Format_YUYV422:
  case SVideoFormat::Format_UYVY422:
  case SVideoFormat::Format_YUV410P:
  case SVideoFormat::Format_YUV411P:
  case SVideoFormat::Format_YUV420P:
  case SVideoFormat::Format_YUV422P:
  case SVideoFormat::Format_YUV444P:
    {
      SVideoBuffer result = videoBuffer;
      result.detach();

      int height = renderedSubtitle.margin;
      foreach (const RenderedSubtitle::Line &line, renderedSubtitle.lines)
        height += line.height;

      QVector< QFuture<void> > futures;
      futures.reserve(renderedSubtitle.lines.count());

      if (height < result.format().size().height())
      foreach (const RenderedSubtitle::Line &line, renderedSubtitle.lines)
      {
        futures += QtConcurrent::run(
            &SubtitleRenderer::blendLine,
            &result, &line,
            result.format().size().height() - height);

        height -= line.height;
      }

      for (int i=0; i<futures.count(); i++)
        futures[i].waitForFinished();

      return result;
    }

  case SVideoFormat::Format_BGGR8:
  case SVideoFormat::Format_GBRG8:
  case SVideoFormat::Format_GRBG8:
  case SVideoFormat::Format_RGGB8:
  case SVideoFormat::Format_BGGR10:
  case SVideoFormat::Format_GBRG10:
  case SVideoFormat::Format_GRBG10:
  case SVideoFormat::Format_RGGB10:
  case SVideoFormat::Format_BGGR16:
  case SVideoFormat::Format_GBRG16:
  case SVideoFormat::Format_GRBG16:
  case SVideoFormat::Format_RGGB16:
    return videoBuffer;
  }

  return videoBuffer;
}

void SubtitleRenderer::drawChar(SVideoBuffer &buffer, const FT_Bitmap &bitmap, int xo, int yo)
{
  if ((xo >= 0) && (xo + bitmap.width < buffer.format().size().width()) &&
      (yo >= 0) && (yo + bitmap.rows < buffer.format().size().height()))
  {
    for (int y=0; y<bitmap.rows; y++)
    {
      const quint8 * const inLine = bitmap.buffer + (y * bitmap.width);
      quint8 * const outLine = reinterpret_cast<quint8 *>(buffer.scanLine(y + yo, 0)) + xo;

      for (int x=0; x<bitmap.width; x++)
        outLine[x] = qMax(outLine[x], inLine[x]);
    }
  }
}

void SubtitleRenderer::blendLine(SVideoBuffer *buffer, const RenderedSubtitle::Line *line, int yo)
{
  const int width = qMin(line->width, buffer->format().size().width());
  const int height = qMin(line->text.format().size().height(), buffer->format().size().height() - yo);
  const int xo = (buffer->format().size().width() / 2) - (width / 2);

  int wf = 1, hf = 1;
  buffer->format().planarYUVRatio(wf, hf);

  for (int y=0; y<height; y++)
  {
    Algorithms::Subtitles::blendLineY(
        reinterpret_cast<quint8 *>(buffer->scanLine(y + yo, 0)) + xo,
        reinterpret_cast<const quint8 *>(line->shadow.scanLine(y, 0)),
        reinterpret_cast<const quint8 *>(line->text.scanLine(y, 0)),
        width);

    if ((y % hf) == 0)
    {
      const int ys = (y + yo) / hf, xs = xo / wf;

      Algorithms::Subtitles::blendLineUV(
          reinterpret_cast<quint8 *>(buffer->scanLine(ys, 1)) + xs,
          reinterpret_cast<quint8 *>(buffer->scanLine(ys, 2)) + xs,
          wf,
          reinterpret_cast<const quint8 *>(line->text.scanLine(y, 0)),
          width);
    }
  }
}

} } // End of namespaces
