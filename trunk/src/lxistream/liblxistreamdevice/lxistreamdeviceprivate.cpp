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

#include "lxistreamdeviceprivate.h"

class LXiStreamDeviceInit : public SApplication::Initializer
{
public:
  virtual void                  startup(void);
  virtual void                  shutdown(void);

private:
  static LXiStreamDeviceInit    self;
};

LXiStreamDeviceInit LXiStreamDeviceInit::self;

void LXiStreamDeviceInit::startup(void)
{
  sApp->addModuleFilter("lxistreamdevice");
}

void LXiStreamDeviceInit::shutdown(void)
{
}
