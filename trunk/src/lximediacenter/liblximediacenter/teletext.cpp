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

#include <liblximediacenter/teletext.h>

namespace LXiMediaCenter {


void Teletext::drawGraphics(QPainter &p, const QRect &r, int mode, char code)
{
  const int bw = (r.width() + 1) / 2, bh = (r.height() + 1) / 3;
  const QColor color = p.pen().color();

  if (mode == 1)
  {
    if (code & 0x01)    p.fillRect(QRect(r.x()     , r.y()          , bw, bh), color);
    if (code & 0x02)    p.fillRect(QRect(r.x() + bw, r.y()          , bw, bh), color);
    if (code & 0x04)    p.fillRect(QRect(r.x()     , r.y() + bh     , bw, bh), color);
    if (code & 0x08)    p.fillRect(QRect(r.x() + bw, r.y() + bh     , bw, bh), color);
    if (code & 0x10)    p.fillRect(QRect(r.x()     , r.y() + bh + bh, bw, bh), color);
    if (code & 0x20)    p.fillRect(QRect(r.x() + bw, r.y() + bh + bh, bw, bh), color);
  }
  else if (mode == 2)
  {
    if (code & 0x01)    p.fillRect(QRect(r.x()     , r.y()          , bw, bh).adjusted(1, 1, -1, -1), color);
    if (code & 0x02)    p.fillRect(QRect(r.x() + bw, r.y()          , bw, bh).adjusted(1, 1, -1, -1), color);
    if (code & 0x04)    p.fillRect(QRect(r.x()     , r.y() + bh     , bw, bh).adjusted(1, 1, -1, -1), color);
    if (code & 0x08)    p.fillRect(QRect(r.x() + bw, r.y() + bh     , bw, bh).adjusted(1, 1, -1, -1), color);
    if (code & 0x10)    p.fillRect(QRect(r.x()     , r.y() + bh + bh, bw, bh).adjusted(1, 1, -1, -1), color);
    if (code & 0x20)    p.fillRect(QRect(r.x() + bw, r.y() + bh + bh, bw, bh).adjusted(1, 1, -1, -1), color);
  }
}


} // End of namespace
