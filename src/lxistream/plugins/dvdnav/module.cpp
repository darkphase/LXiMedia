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
#include "discreader.h"
#include "formatprober.h"


namespace LXiStream {
namespace DVDNavBackend {


void Module::registerClasses(void)
{
  FormatProber::registerClass<FormatProber>(0);
  DiscReader::registerClass<DiscReader>(DiscReader::formatName);
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  const QByteArray text;/* =
      " <h2>FFMpeg (libavcodec, libavformat, libswscale)</h2>\n"
      " Versions: " LIBAVCODEC_IDENT ", " LIBAVFORMAT_IDENT ", " LIBSWSCALE_IDENT "<br />\n"
      " Website: <a href=\"http://www.ffmpeg.org/\">www.ffmpeg.org</a><br />\n"
      " <br />\n"
      " Used under the terms of the GNU Lesser General Public License version 2.1\n"
      " as published by the Free Software Foundation.<br />\n"
      " <br />\n";*/

  return text;
}


} } // End of namespaces

#ifdef PLUGIN_NAME
#include <QtPlugin>
Q_EXPORT_PLUGIN2(PLUGIN_NAME, LXiStream::DVDNavBackend::Module);
#endif
