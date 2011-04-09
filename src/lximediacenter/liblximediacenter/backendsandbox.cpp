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

#include "backendsandbox.h"

namespace LXiMediaCenter {

S_FACTORIZABLE_INSTANCE(BackendSandbox)

QList<BackendSandbox *> BackendSandbox::create(QObject *parent)
{
  return factory().createObjects<BackendSandbox>(parent);
}

BackendSandbox::BackendSandbox(QObject *parent)
  : QObject(parent)
{
}

BackendSandbox::~BackendSandbox()
{
}

void BackendSandbox::initialize(SSandboxServer *)
{
}

void BackendSandbox::close(void)
{
}

} // End of namespace
