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

#include "test.h"
#include "lxistream/plugins/linuxdvb/module.h"


/*! Loads the LinuxDvbBackend plugin.

    \note This is required before any of the other tests depending on the
          LinuxDvbBackend can run.
 */
void LXiStreamTest::LinuxDvbBackend_Load(void)
{
  if (runSilent()) QSKIP("Test skipped beucause -silent was specified.", SkipSingle);

  QVERIFY(SSystem::loadModule(new LinuxDvbBackend::Module()));
}
