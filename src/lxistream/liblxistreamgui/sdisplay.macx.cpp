/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#include "sdisplay.h"

namespace LXiStreamGui {


struct SDisplay::Private
{
};


SDisplay::SDisplay(void)
         :p(new Private())
{
}

SDisplay::~SDisplay()
{
  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SDisplay::blockScreenSaver(bool)
{
}

QList<SDisplay::Mode> SDisplay::allModes(void) const
{
  QList<Mode> modes;

  return modes;
}

SDisplay::Mode SDisplay::mode(void) const
{

  Mode m;
  m.size = QSize(0, 0);
  m.rate = 0;

  return m;
}

bool SDisplay::setMode(const Mode &)
{
  return false;
}

bool SDisplay::setSize(const QSize &)
{
  return false;
}

QList<qreal> SDisplay::allRates(void) const
{
  QList<qreal> r;

  return r;
}

bool SDisplay::setRate(qreal)
{
  return false;
}

void SDisplay::timerEvent(QTimerEvent *)
{
}


} // End of namespace
