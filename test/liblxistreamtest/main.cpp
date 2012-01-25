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

#include <QTest>
#include <iostream>
#include "streamtest.h"
#include "dvdnavtest.h"
#include "ffmpegtest.h"
#include "iotest.h"
#ifdef ENABLE_GLSL
#include "opengltest.h"
#endif
#ifdef ENABLE_ALSA
#include "alsatest.h"
#endif
#include "filetester.h"
#include "performancetest.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  if ((app.arguments().count() >= 3) && (app.arguments()[1] == "-batch"))
  {
    SApplication * const mediaApp = SApplication::createForQTest(qApp);
    mediaApp->loadModule("lxistream_dvdnav");
    mediaApp->loadModule("lxistream_ffmpeg");

    // Ignore all logged messages
    struct T { static void logMessage(QtMsgType, const char *) { } };
    qInstallMsgHandler(&T::logMessage);

    QDir dir(app.arguments()[2]);
    std::cout << "Testing all files in directory " << dir.absolutePath().toAscii().data() << std::endl;

    foreach (const QFileInfo &fileInfo, dir.entryInfoList(QDir::NoFilter, QDir::Name))
      FileTester::testFile(fileInfo.absoluteFilePath());

    // Wait for all tasks to finish
    QThreadPool::globalInstance()->waitForDone();

    delete mediaApp;
  }
  else
  {
    if (QTest::qExec(new StreamTest(&app), app.arguments()) != 0)       return 1;
    if (QTest::qExec(new IOTest(&app), app.arguments()) != 0)           return 1;
    if (QTest::qExec(new DVDNavTest(&app), app.arguments()) != 0)       return 1;
    if (QTest::qExec(new FFMpegTest(&app), app.arguments()) != 0)       return 1;
#ifdef ENABLE_GLSL
    if (QTest::qExec(new OpenGLTest(&app), app.arguments()) != 0)       return 1;
#endif
#ifdef ENABLE_ALSA
    if (QTest::qExec(new AlsaTest(&app), app.arguments()) != 0)         return 1;
#endif
    if (QTest::qExec(new PerformanceTest(&app), app.arguments()) != 0)  return 1;
  }

  return 0;
}
