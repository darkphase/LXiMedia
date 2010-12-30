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

#include <QtCore>
#include <QtGui>
#include <LXiStream>
#include <iostream>
#include "lxistream/plugins/ffmpeg/module.h"
#include "filetester.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  if (app.arguments().count() >= 2)
  {
    SSystem::initialize(SSystem::Initialize_Devices | SSystem::Initialize_LogToConsole, 0);
    SSystem::loadModule(new FFMpegBackend::Module());

    QDir dir(app.arguments()[1]);
    std::cout << "Testing all files in directory " << dir.absolutePath().toAscii().data() << std::endl;

    foreach (const QFileInfo &fileInfo, dir.entryInfoList(QDir::NoFilter, QDir::Name))
      QtConcurrent::run(&FileTester::testFile, fileInfo.absoluteFilePath());
      //FileTester::testFile(fileInfo.absoluteFilePath());

    // Wait for all tasks to finish
    QThreadPool::globalInstance()->waitForDone();

    SSystem::shutdown();
    return 0;
  }
  else
  {
    std::cerr << "Usage: lxistreamfiletester <directory>" << std::endl;

    return 1;
  }
}
