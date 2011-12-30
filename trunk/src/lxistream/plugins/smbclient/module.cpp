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
#include "smbfilesystem.h"

namespace LXiStream {
namespace SMBClientBackend {

bool Module::registerClasses(void)
{
  SMBFilesystem::init();
  SMBFilesystem::registerClass<SMBFilesystem>(SMBFilesystem::scheme);

  return true;
}

void Module::unload(void)
{
  SMBFilesystem::close();
}

QByteArray Module::about(void)
{
  return "SMBClient plugin by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  const QByteArray text =
      " <h3>libsmbclient</h3>\n"
      " Version: " + SMBFilesystem::version() + "<br />\n"
      " <br />\n"
      " Used under the terms of the GNU General Public License version 2\n"
      " as published by the Free Software Foundation.\n";

  return text;
}

} } // End of namespaces
