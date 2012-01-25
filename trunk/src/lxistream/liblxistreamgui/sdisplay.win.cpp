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
#include <windows.h>

namespace LXiStreamGui {


struct SDisplay::Private
{
  int                         * sysparamValue;
};


SDisplay::SDisplay(void)
         :p(new Private())
{
  p->sysparamValue = NULL;
}

SDisplay::~SDisplay()
{
  if (p->sysparamValue)
    blockScreenSaver(false);

  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SDisplay::blockScreenSaver(bool block)
{
  static const UINT getList[] = { SPI_GETLOWPOWERTIMEOUT, SPI_GETPOWEROFFTIMEOUT, SPI_GETSCREENSAVETIMEOUT };
  static const UINT setList[] = { SPI_SETLOWPOWERTIMEOUT, SPI_SETPOWEROFFTIMEOUT, SPI_SETSCREENSAVETIMEOUT };
  static const unsigned listSize = sizeof(getList)/sizeof(getList[0]);

  if (block && (p->sysparamValue == NULL))
  {
    p->sysparamValue = new int[listSize];

    for (unsigned i=0; i<listSize; i++)
    {
      ::SystemParametersInfo(getList[i], 0, p->sysparamValue + i, 0);
      ::SystemParametersInfo(setList[i], 0, NULL, 0);
    }
  }
  else if (!block && (p->sysparamValue != NULL))
  {
    for (unsigned i=0; i<listSize; i++)
      ::SystemParametersInfo(setList[i], 0, p->sysparamValue + i, 0);

    delete [] p->sysparamValue;
    p->sysparamValue = NULL;
  }
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
