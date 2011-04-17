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

#ifndef LXSTREAM_SSUBTITLERENDERNODE_H
#define LXSTREAM_SSUBTITLERENDERNODE_H

#include <QtCore>
#include <LXiCore>
#include "../sgraph.h"
#include "../spixels.h"
#include "../ssubtitlebuffer.h"
#include "../svideobuffer.h"
#include "../export.h"

namespace LXiStream {

class LXISTREAM_PUBLIC SSubtitleRenderNode : public QObject,
                                             public SGraph::Node
{
Q_OBJECT
private:
  struct                        FontLoader { FontLoader(void); ~FontLoader(); };
  struct                        Lines;
  struct                        Char;

public:
  explicit                      SSubtitleRenderNode(SGraph *);
  virtual                       ~SSubtitleRenderNode();

  unsigned                      fontRatio(void) const;
  void                          setFontRatio(unsigned r);

public slots:
  void                          input(const SSubtitleBuffer &);
  void                          input(const SVideoBuffer &);

signals:
  void                          output(const SVideoBuffer &);

public:
  static SVideoBuffer           renderSubtitles(const SVideoBuffer &, const QStringList &, unsigned ratio = 16);

private:
  _lxi_internal void            processTask(const SSubtitleBuffer &);
  _lxi_internal void            processTask(const SVideoBuffer &);
  _lxi_internal static void     renderSubtitles(SVideoBuffer &, const Lines *, const Char * const *);

private:
  _lxi_internal static const unsigned char subFontsData[];
  _lxi_internal static QMap<int, QVector<Char *> > characters;
  _lxi_internal static FontLoader  fontLoader;

  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
