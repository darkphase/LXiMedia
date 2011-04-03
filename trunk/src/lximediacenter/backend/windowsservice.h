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

#ifndef WINDOWSSERVICE_H
#define WINDOWSSERVICE_H

#include <windows.h>
#include <tchar.h>
#include <QtCore>


class WindowsService
{
public:
                                WindowsService(const QString &);
  virtual                       ~WindowsService();

  virtual bool                  install(bool = true);
  virtual bool                  uninstall(void);

  virtual void                  waitForStopSignal(void);
  virtual bool                  waitForStopSignal(unsigned);
  virtual void                  startServiceDispatcher(void);

protected:
  virtual void                  interrogate(void);
  virtual void                  shutdown(void);
  virtual bool                  quit(void);
  virtual void                  pause(void);
  virtual void                  resume(void);
  virtual void                  user(quint32);
  virtual void                  unrecognised(quint32);

  virtual void                  init(int, const char *[]);
  virtual int                   run(void) = 0;
  virtual void                  close(void);

private:
  static void WINAPI            serviceControlHandler(DWORD);
  static void WINAPI            serviceMain(DWORD, TCHAR*[]);

  static WindowsService       * instance;
  static TCHAR                  serviceName[512];
  static SERVICE_STATUS         serviceStatus;
  static SERVICE_STATUS_HANDLE  serviceStatusHandle;
  static HANDLE                 stopServiceEvent;
};


#endif
