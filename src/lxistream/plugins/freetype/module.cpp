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
      " <h3>FreeType</h3>\n"
      " <p>Version: " + QByteArray::number(FREETYPE_MAJOR) +
      "." + QByteArray::number(FREETYPE_MINOR) +
      "." + QByteArray::number(FREETYPE_PATCH) + "</p>\n"
      " <p>Website: <a href=\"http://freetype.org/\">freetype.org</a></p>\n"
      " <p>Used under the terms of the distributed under the terms of the\n"
      " FreeType project license.\n"
      " <h3>DejaVu Fonts</h3>\n"
      " <p>Website: <a href=\"http://dejavu-fonts.org/\">dejavu-fonts.org</a></p>\n";

  return text;
}

} } // End of namespaces