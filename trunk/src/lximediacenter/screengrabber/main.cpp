/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#include "screengrabber.h"

int main(int argc, char *argv[])
{
  QApplication qapp(argc, argv);
  qapp.setOrganizationName("LeX-Interactive");
  qapp.setOrganizationDomain("lximedia.sf.net");
  qapp.setApplicationName("LXiMediaCenter");
  qapp.setApplicationVersion(
#include "_version.h"
      );

  SApplication mediaApp(true);

  SStringParser::setStaticField("_PRODUCT", qApp->applicationName());

  ScreenGrabber screenGrabber;
  screenGrabber.show();

  return qapp.exec();
}
