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
#include "formatprober.h"
#include "imagedecoder.h"
#include "imageencoder.h"

namespace LXiStream {
namespace GuiBackend {

bool Module::registerClasses(void)
{
  // Ensure static initializers have been initialized.
  SImage::rawImageSuffixes();

  FormatProber::registerClass<FormatProber>(1);

  ImageDecoder::registerClass<ImageDecoder>("bmp");
  ImageDecoder::registerClass<ImageDecoder>("jpg");
  ImageDecoder::registerClass<ImageDecoder>("jpeg");
  ImageDecoder::registerClass<ImageDecoder>("png");
  ImageDecoder::registerClass<ImageDecoder>("pbm");
  ImageDecoder::registerClass<ImageDecoder>("pgm");
  ImageDecoder::registerClass<ImageDecoder>("ppm");
  ImageDecoder::registerClass<ImageDecoder>("tiff");

  ImageEncoder::registerClass<ImageEncoder>("bmp");
  ImageEncoder::registerClass<ImageEncoder>("jpg");
  ImageEncoder::registerClass<ImageEncoder>("jpeg");
  ImageEncoder::registerClass<ImageEncoder>("png");
  ImageEncoder::registerClass<ImageEncoder>("ppm");
  ImageEncoder::registerClass<ImageEncoder>("tiff");

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
      " <h3>libexif C EXIF library</h3>\n"
      " <p>Website: <a href=\"http://libexif.sourceforge.net/\">libexif.sourceforge.net</a></p>\n"
      " <p>Used under the terms of the GNU Lesser General Public License version 2.1\n"
      " as published by the Free Software Foundation.</p>\n";

  return text;
}

} } // End of namespaces
