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

#include <QtGui>
#include <QTest>
#include "coretest.h"
#include "dvdnavtest.h"
#include "ffmpegtest.h"
#include "iotest.h"
#ifdef ENABLE_GLSL
#include "opengltest.h"
#endif
#ifdef ENABLE_ALSA
#include "alsatest.h"
#endif


int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  if (QTest::qExec(new CoreTest(&app), app.arguments()) != 0)     return 1;
  if (QTest::qExec(new IOTest(&app), app.arguments()) != 0)       return 1;
  if (QTest::qExec(new DVDNavTest(&app), app.arguments()) != 0)   return 1;
  if (QTest::qExec(new FFMpegTest(&app), app.arguments()) != 0)   return 1;
#ifdef ENABLE_GLSL
  if (QTest::qExec(new OpenGLTest(&app), app.arguments()) != 0)   return 1;
#endif
#ifdef ENABLE_ALSA
  if (QTest::qExec(new AlsaTest(&app), app.arguments()) != 0)     return 1;
#endif

  return 0;
}
