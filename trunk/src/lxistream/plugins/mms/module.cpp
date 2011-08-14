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

#include "module.h"
#include "networkbufferreader.h"

namespace LXiStream {
namespace MMSBackend {

bool Module::registerClasses(void)
{
  NetworkBufferReader::registerClass<NetworkBufferReader>("mms");
  NetworkBufferReader::registerClass<NetworkBufferReader>("mmsh");
  NetworkBufferReader::registerClass<NetworkBufferReader>("http.asx");

  return true;
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return "libMMS plugin by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  const QByteArray text =
      "<h3>libmms</h3>\n"
      "Website: <a href=\"http://libmms.sourceforge.net/\">libmms.sourceforge.net</a><br />\n"
      "<br />\n"
      "Used under the terms of the GNU Lesser General Public License version 2.1\n"
      "as published by the Free Software Foundation.\n";

  return text;
}

} } // End of namespaces
