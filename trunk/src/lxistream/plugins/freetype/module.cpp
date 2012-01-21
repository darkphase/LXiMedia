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
#include "subtitlerenderer.h"

namespace LXiStream {
namespace FreeTypeBackend {

bool Module::registerClasses(void)
{
  SubtitleRenderer::registerClass<SubtitleRenderer>(1);

  return true;
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return "FreeType plugin by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  const QByteArray text =
      "<h3>FreeType</h3>\n"
      "Version: " + QByteArray::number(FREETYPE_MAJOR) +
      "." + QByteArray::number(FREETYPE_MINOR) +
      "." + QByteArray::number(FREETYPE_PATCH) + "<br />\n"
      "Website: <a href=\"http://freetype.org/\">freetype.org</a><br />\n"
      "<br />\n"
      "Used under the terms of the GNU General Public License version 2\n"
      "as published by the Free Software Foundation.\n"
      "<h3>DejaVu Fonts</h3>\n"
      "Website: <a href=\"http://dejavu-fonts.org/\">dejavu-fonts.org</a><br />\n";

  return text;
}

} } // End of namespaces
