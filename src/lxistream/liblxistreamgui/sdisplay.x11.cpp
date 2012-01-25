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
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/XTest.h>

namespace LXiStreamGui {


struct SDisplay::Private
{
  Display                     * display;
  Window                        rootWin;
  int                           blockTimer;
};


SDisplay::SDisplay(void)
         :p(new Private())
{
  p->display = XOpenDisplay(NULL);
  p->rootWin = RootWindow(p->display, 0);
  p->blockTimer = -1;
}

SDisplay::~SDisplay()
{
  XCloseDisplay(p->display);

  delete p;
  *const_cast<Private **>(&p) = NULL;
}

void SDisplay::blockScreenSaver(bool block)
{
  if (block && (p->blockTimer == -1))
  {
    p->blockTimer = startTimer(30000);
  }
  else if (!block && (p->blockTimer != -1))
  {
    killTimer(p->blockTimer);
    p->blockTimer = -1;
  }
}

QList<SDisplay::Mode> SDisplay::allModes(void) const
{
  QList<Mode> modes;

  int numScreenSizes = 0;
  const XRRScreenSize * const screenSizes = XRRSizes(p->display, 0, &numScreenSizes);
  for (int i=0; i<numScreenSizes; i++)
  {
    Mode m;
    m.size = QSize(screenSizes[i].width, screenSizes[i].height);

    int numRates = 0;
    const short * const rates = XRRRates(p->display, 0, i, &numRates);
    for (int j=0; j<numRates; j++)
    {
      m.rate = rates[j];
      modes += m;
    }
  }

  return modes;
}

SDisplay::Mode SDisplay::mode(void) const
{
  int numScreenSizes = 0;
  const XRRScreenSize * const screenSizes = XRRSizes(p->display, 0, &numScreenSizes);
  XRRScreenConfiguration * const screenConfig = XRRGetScreenInfo(p->display, p->rootWin);
  Rotation rotate = 0;
  const SizeID sizeId = XRRConfigCurrentConfiguration(screenConfig, &rotate);

  Mode m;
  m.size = QSize(screenSizes[sizeId].width, screenSizes[sizeId].height);
  m.rate = XRRConfigCurrentRate(screenConfig);

  return m;
}

bool SDisplay::setMode(const Mode &mode)
{
  XRRScreenConfiguration * const screenConfig = XRRGetScreenInfo(p->display, p->rootWin);
  Rotation rotate = 0;
  XRRConfigCurrentConfiguration(screenConfig, &rotate);

  int numScreenSizes = 0;
  const XRRScreenSize * const screenSizes = XRRSizes(p->display, 0, &numScreenSizes);
  for (int i=0; i<numScreenSizes; i++)
  if ((mode.size.width() == screenSizes[i].width) && (mode.size.height() == screenSizes[i].height))
  {
    short bestRate = 0;
    int numRates = 0;
    const short * const rates = XRRRates(p->display, 0, i, &numRates);
    for (int j=0; j<numRates; j++)
    if (qAbs(rates[j] - mode.rate) < qAbs(bestRate - mode.rate))
      bestRate = rates[j];

    const Status status = XRRSetScreenConfigAndRate(p->display, screenConfig,
                                                    p->rootWin, i, rotate,
                                                    bestRate, CurrentTime);

    return status != 0;
  }

  return false;
}

bool SDisplay::setSize(const QSize &size)
{
  XRRScreenConfiguration * const screenConfig = XRRGetScreenInfo(p->display, p->rootWin);
  Rotation rotate = 0;
  XRRConfigCurrentConfiguration(screenConfig, &rotate);

  int numScreenSizes = 0;
  const XRRScreenSize * const screenSizes = XRRSizes(p->display, 0, &numScreenSizes);
  for (int i=0; i<numScreenSizes; i++)
  if ((size.width() == screenSizes[i].width) && (size.height() == screenSizes[i].height))
  {
    const Status status = XRRSetScreenConfig(p->display, screenConfig,
                                             p->rootWin, i, rotate,
                                             CurrentTime);

    return status != 0;
  }

  return false;
}

QList<qreal> SDisplay::allRates(void) const
{
  QList<qreal> r;

  XRRScreenConfiguration * const screenConfig = XRRGetScreenInfo(p->display, p->rootWin);
  Rotation rotate = 0;
  const SizeID sizeId = XRRConfigCurrentConfiguration(screenConfig, &rotate);

  int numRates = 0;
  const short * const rates = XRRRates(p->display, 0, sizeId, &numRates);
  for (int i=0; i<numRates; i++)
    r += rates[i];

  return r;
}

bool SDisplay::setRate(qreal rate)
{
  XRRScreenConfiguration * const screenConfig = XRRGetScreenInfo(p->display, p->rootWin);
  Rotation rotate = 0;
  const SizeID sizeId = XRRConfigCurrentConfiguration(screenConfig, &rotate);

  short bestRate = 0;
  int numRates = 0;
  const short * const rates = XRRRates(p->display, 0, sizeId, &numRates);
  for (int i=0; i<numRates; i++)
  if (qAbs(rates[i] - rate) < qAbs(bestRate - rate))
    bestRate = rates[i];

  if (bestRate != XRRConfigCurrentRate(screenConfig))
  {
    const Status status = XRRSetScreenConfigAndRate(p->display, screenConfig,
                                                    p->rootWin, sizeId, rotate,
                                                    bestRate, CurrentTime);

    return status != 0;
  }
  else
    return true;
}

void SDisplay::timerEvent(QTimerEvent *)
{
  KeySym keySym = XStringToKeysym("Num_Lock");
  KeyCode keyCode = XKeysymToKeycode(p->display, keySym);

  XTestFakeKeyEvent(p->display, keyCode, True, CurrentTime);
  XTestFakeKeyEvent(p->display, keyCode, False, CurrentTime);
  XTestFakeKeyEvent(p->display, keyCode, True, CurrentTime);
  XTestFakeKeyEvent(p->display, keyCode, False, CurrentTime);
  XFlush(p->display);
}


} // End of namespace
