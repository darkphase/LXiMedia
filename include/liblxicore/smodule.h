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

#ifndef LXICORE_SMODULE_H
#define LXICORE_SMODULE_H

#include <QtCore>
#include "export.h"

namespace LXiCore {

/*! The SModule interface is used to register modules to the factories.
 */
class LXICORE_PUBLIC SModule
{
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
};

} // End of namespace

#define LXiCore_SModule_iid "net.sourceforge.lximedia.LXiCore.SModule"
Q_DECLARE_INTERFACE(LXiCore::SModule, LXiCore_SModule_iid)

#endif
