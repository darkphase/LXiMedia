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

#ifndef LXICORE_SMODULE_H
#define LXICORE_SMODULE_H

#include <QtCore>
#include "export.h"

namespace LXiCore {

/*! The SModule interface is used to register modules to the factories.
 */
class LXICORE_PUBLIC SModule : public QObject
{
Q_OBJECT
public:
  virtual                       ~SModule();

  /*! Shall register all classes with their respective factories.
   */
  virtual bool                  registerClasses(void) = 0;

  /*! Shall clean up all allocated resources, any registred classes classes do
      not have to be unregistered.
   */
  virtual void                  unload(void) = 0;

  /*! Returns one line of text describing the plugin (e.g. name, author).
   */
  virtual QByteArray            about(void) = 0;

  /*! Returns an HTML formatted H3 section describing the license(s) that apply
      to the use of this pluging (e.g. licenses of any libraries used).
   */
  virtual QByteArray            licenses(void) = 0;

public:
  /*! This boolean indicates if a module is allowed to open devices (e.g. audio
      inputs, DVB tuners, etc.). By default this is set to false to inclrease
      startup speed, set this to true before creating an instance of
      SApplication if access to devices is needed.
   */
  static bool                   loadDevices;
};

} // End of namespace

#endif
