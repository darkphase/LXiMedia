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
#include "formatprober.h"
#include "imagedecoder.h"
#include "imageencoder.h"

namespace LXiStream {
namespace GuiBackend {

bool Module::registerClasses(void)
{
  FormatProber::registerClass<FormatProber>(0);

  ImageDecoder::registerClass<ImageDecoder>("BMP");
  ImageDecoder::registerClass<ImageDecoder>("JPG");
  ImageDecoder::registerClass<ImageDecoder>("JPEG");
  ImageDecoder::registerClass<ImageDecoder>("PNG");
  ImageDecoder::registerClass<ImageDecoder>("PBM");
  ImageDecoder::registerClass<ImageDecoder>("PGM");
  ImageDecoder::registerClass<ImageDecoder>("PPM");
  ImageDecoder::registerClass<ImageDecoder>("TIFF");

  ImageEncoder::registerClass<ImageEncoder>("BMP");
  ImageEncoder::registerClass<ImageEncoder>("JPG");
  ImageEncoder::registerClass<ImageEncoder>("JPEG");
  ImageEncoder::registerClass<ImageEncoder>("PNG");
  ImageEncoder::registerClass<ImageEncoder>("PPM");
  ImageEncoder::registerClass<ImageEncoder>("TIFF");

  return true;
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  return "LXiStreamGui plugin by A.J. Admiraal";
}

QByteArray Module::licenses(void)
{
  const QByteArray text =
      "<h3>libexif C EXIF library</h3>\n"
      "Website: <a href=\"http://libexif.sourceforge.net/\">libexif.sourceforge.net</a><br />\n"
      "<br />\n"
      "Used under the terms of the GNU Lesser General Public License version 2.1\n"
      "as published by the Free Software Foundation.\n";

  return text;
}

} } // End of namespaces

#include <QtPlugin>
Q_EXPORT_PLUGIN2(lxistream_gui, LXiStream::GuiBackend::Module);
