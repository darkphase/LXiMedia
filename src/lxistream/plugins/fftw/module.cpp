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

#include <fftw3.h>

namespace LXiStream {
namespace FFTWBackend {


void Module::registerClasses(void)
{
  fftwMutex();
}

void Module::unload(void)
{
}

QByteArray Module::about(void)
{
  const QByteArray text;/* =
      " <h2>FFTW</h2>\n"
      " Version: " + QByteArray(fftwf_version) + "<br />\n"
      " Website: <a href=\"http://www.fftw.org/\">www.fftw.org</a><br />\n"
      " <br />\n"
      " <b>Copyright (c) 2003, 2007-8 Matteo Frigo</b><br />\n"
      " <b>Copyright (c) 2003, 2007-8 Massachusetts Institute of Technology</b><br />\n"
      " <br />\n"
      " Used under the terms of the GNU General Public License version 2\n"
      " as published by the Free Software Foundation.<br />\n"
      " <br />\n";*/

  return text;
}


QMutex & Module::fftwMutex(void)
{
  static QMutex m(QMutex::Recursive);

  return m;
}


} } // End of namespaces

#include <QtPlugin>
Q_EXPORT_PLUGIN2("fftw", LXiStream::FFTWBackend::Module);
