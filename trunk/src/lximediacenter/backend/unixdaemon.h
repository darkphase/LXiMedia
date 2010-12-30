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

#ifndef UNIXDEAMON_H
#define UNIXDEAMON_H

#include <unistd.h>
#include <QtCore>


class UnixDaemon
{
public:
                                UnixDaemon(const QString &name, const QString &pidFile, bool autoRecover);
  virtual                       ~UnixDaemon();

  virtual pid_t                 start(void);
  virtual bool                  stop(void);

protected:
  virtual int                   run(void) = 0;
  virtual bool                  terminate(void);

private:
  bool                          startAutoRecover(void);
  int                           startChild(void);
  static void                   termHandler(int);

private:
  static UnixDaemon           * instance;
  const QString                 name;
  const QString                 pidFile;
  const bool                    autoRecover;
  pid_t                         sessionID;
  pid_t                         childID;
};


#endif
