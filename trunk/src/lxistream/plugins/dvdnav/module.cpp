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

#include "module.h"
#include "bufferreader.h"
#include "formatprober.h"


namespace LXiStream {
namespace DVDNavBackend {

bool Module::registerClasses(void)
{
  FormatProber::registerClass<FormatProber>(0);
  BufferReader::registerClass<BufferReader>(BufferReader::formatName);

  return true;
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return "DVDNav plugin by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  const QByteArray text =
      " <h3>libdvdnav</h3>\n"
      " <p>Website: <a href=\"http://dvd.sourceforge.net/\">dvd.sourceforge.net</a></p>\n"
      " <p>Used under the terms of the GNU General Public License version 3\n"
      " as published by the Free Software Foundation.</p>\n";

  return text;
}

} } // End of namespaces
