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

#ifndef LXICORE_SDAEMON_H
#define LXICORE_SDAEMON_H

#include <QtCore>
#include "export.h"

namespace LXiCore {

/*! The SDaemon class abstracts the system daemon/service functionality. On
    Unix, this class will fork the the main application and leave the child
    process running while exiting the parent process, so that the application
    can be started using startstopdaemon. On Windows this class uses the
    CreateService() and associated API to provide a Windows system service.

    To use it, create a subclass of SDaemon and implement the run() and quit()
    methods and create an instance of the subclass from the main() function.

    The SDaemon will provide a command-line interface to manage it. For example,
    --run will directy run the application code (without forking) for use within
    a debugger. Furthermore, a --install and --uninstall will be available on
    Windows to install and uninstall the service.

    \code
      class MyDaemon : public SDaemon
      {
      public:
        MyDaemon(void)
          : SDaemon("My Super Daemon")
        {
        }

        virtual int run(int &argc, char *argv[])
        {
          QCoreApplication app(argc, argv);
          SApplictation mediaApp;

          // App code ...

          return qApp->exec();
        }

        virtual void quit(void)
        {
          qApp->quit();
        }
      };

      int main(int argc, char *argv[])
      {
          MyDaemon daemon;
          return daemon.main(argc, argv);
      }
    \endcode
 */
class LXICORE_PUBLIC SDaemon
{
public:
  static bool                   isInstalled(const QString &name);
  static bool                   isRunning(const QString &name);
  static bool                   start(const QString &name);
  static bool                   stop(const QString &name);

public:
  explicit                      SDaemon(const QString &name);
  virtual                       ~SDaemon();

  int                           main(int &argc, char *argv[]);

protected:
  virtual int                   run(int &argc, char *argv[]) = 0;
  virtual void                  quit(void) = 0;

private:
  struct Data;
};

} // End of namespace

#endif
