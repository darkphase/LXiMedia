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

#ifndef LXSTREAM_POWERMANAGER_H
#define LXSTREAM_POWERMANAGER_H

#include <QtCore>

namespace LXiStream {

class SGraph;

namespace Private {


class PowerManager
{
public:
  static void                   addActiveObject(QObject *);
  static void                   removeActiveObject(QObject *);
  static bool                   hasActiveObjects(void);

  static QStringList            getCPUs(void);
  static QStringList            getGovernors(void);
  static QString                getGovernor(void);
  static QString                getGovernor(const QString &cpu);
  static bool                   setGovernor(const QString &governor);

private:
  static QAtomicInt           & activeObjects(void);
  static QString              & standardGovernor(void);
};


} } // End of namespaces

#endif
