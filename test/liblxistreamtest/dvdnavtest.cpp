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

#include "dvdnavtest.h"
#include <QtTest>
#include "lxistream/plugins/dvdnav/bufferreader.h"
#include "lxistream/plugins/dvdnav/module.h"
#include "lxistream/plugins/ffmpeg/module.h"


void DVDNavTest::initTestCase(void)
{
  mediaApp = SApplication::createForQTest(this);

  QVERIFY(mediaApp->loadModule(new DVDNavBackend::Module()));
  QVERIFY(mediaApp->loadModule(new FFMpegBackend::Module()));

  //SMediaInfo info("");
  //qDebug() << info.programs().count();
}

void DVDNavTest::cleanupTestCase(void)
{
  delete mediaApp;
  mediaApp = NULL;
}
