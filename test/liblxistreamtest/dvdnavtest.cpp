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
#include <LXiStream>
#include "lxistream/plugins/dvdnav/discreader.h"
#include "lxistream/plugins/dvdnav/module.h"
#include "lxistream/plugins/ffmpeg/module.h"


void DVDNavTest::initTestCase(void)
{
  // We only want to initialize common and gui here, not probe for plugins.
  QVERIFY(SSystem::initialize(SSystem::Initialize_Devices |
                              SSystem::Initialize_LogToConsole, 0));

  QVERIFY(SSystem::loadModule(new DVDNavBackend::Module()));
  QVERIFY(SSystem::loadModule(new FFMpegBackend::Module()));

//  SDiscInfo discInfo("");
//  qDebug() << discInfo.titles().count();

//  SDiscInputNode discInputNode(NULL, "");
//  qDebug() << discInputNode.numTitles();
//  qDebug() << discInputNode.openTitle(0);
}

void DVDNavTest::cleanupTestCase(void)
{
  SSystem::shutdown();
}
