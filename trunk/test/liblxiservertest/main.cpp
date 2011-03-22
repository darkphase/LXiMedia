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

#include <QTest>
#include "httpservertest.h"
#include "sandboxtest.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  if ((argc >= 4) && (strcmp(argv[1], "--sandbox") == 0))
    return SandboxTest::startSandbox(argv[2], argv[3]);

  if (QTest::qExec(new HttpServerTest(&app), app.arguments()) != 0)     return 1;
  if (QTest::qExec(new SandboxTest(&app), app.arguments()) != 0)        return 1;

  return 0;
}
