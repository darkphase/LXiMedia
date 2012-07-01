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

#include "cameraserver.h"

namespace LXiMediaCenter {
namespace CameraBackend {

const char CameraServer::htmlFrontPageContent[] =
    "   <div class=\"list_thumbnails\" id=\"cameraitems\">\n"
    "   </div>\n"
    "   <script type=\"text/javascript\">loadListContent(\"cameraitems\", \"{SERVER_PATH}\", 0, 0);</script>\n";

QByteArray CameraServer::frontPageContent(void)
{
  int count = 1;
  if (!listItems(QString::null, 0, count).isEmpty())
  {
    SStringParser htmlParser;
    htmlParser.setField("SERVER_PATH", QUrl(serverPath()));

    return htmlParser.parse(htmlFrontPageContent);
  }

  return QByteArray();
}

} } // End of namespaces
