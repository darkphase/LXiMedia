/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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

#ifndef SUBTITLERENDERER_H
#define SUBTITLERENDERER_H

#include <QtCore>
#include <LXiStream>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace LXiStream {
namespace FreeTypeBackend {

class SubtitleRenderer : public SInterfaces::SubtitleRenderer
{
Q_OBJECT
private:
  struct RenderedSubtitle
  {
    struct Line
    {
      int                       width, height;
      SVideoBuffer              text;
      SVideoBuffer              shadow;
    };

    int                         uid;
    int                         margin;
    SSize                       videoSize;
    QList<Line>                 lines;
  };

public:
                                SubtitleRenderer(const QString &, QObject *);
  virtual                       ~SubtitleRenderer();

public: // From SInterfaces::SubtitleRenderer
  virtual void                  setFontRatio(float r);
  virtual float                 fontRatio(void) const;

  virtual void                  prepareSubtitle(const SSubtitleBuffer &, const SSize &);
  virtual SVideoBuffer          processBuffer(const SVideoBuffer &, const SSubtitleBuffer &);

private:
  RenderedSubtitle              renderSubtitle(const SSubtitleBuffer &, const SSize &);
  SVideoBuffer                  blendSubtitle(const SVideoBuffer &, const RenderedSubtitle &);

  static void                   drawChar(SVideoBuffer &, const FT_Bitmap &, int x, int y);
  static void                   blendLine(SVideoBuffer *, const RenderedSubtitle::Line *, int y);

private:
  static int                    instanceCount;
  static QByteArray             faceData;

  float                         ratio;

  bool                          initialized;
  ::FT_Library                  library;
  ::FT_Face                     face;

  bool                          rendering;
  QFuture<RenderedSubtitle>     renderFuture;
  QQueue<RenderedSubtitle>      renderedSubtitles;
};

} } // End of namespaces

#endif