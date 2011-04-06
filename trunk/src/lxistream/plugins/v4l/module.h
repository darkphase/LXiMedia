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

#ifndef V4LBACKEND_MODULE_H
#define V4LBACKEND_MODULE_H

#include <LXiStream>

#ifndef QT_PLUGIN
#define QT_PLUGIN
#endif

namespace LXiStream {
namespace V4lBackend {

class Module : public SModule
{
Q_OBJECT
public:
  virtual bool                  registerClasses(void);
  virtual void                  unload(void);
  virtual QByteArray            about(void);
};

} } // End of namespaces

#endif
