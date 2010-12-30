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

#ifndef FFTWBACKEND_MODULE_H
#define FFTWBACKEND_MODULE_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace FFTWBackend {


class Module : public QObject,
               public SInterfaces::Module
{
Q_OBJECT
Q_INTERFACES(LXiStream::SInterfaces::Module)
public:
  virtual void                  registerClasses(void);
  virtual void                  unload(void);
  virtual QByteArray            about(void);

public:
  static QMutex               & fftwMutex(void);
};


} } // End of namespaces

#endif
