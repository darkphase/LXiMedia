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

#ifndef LXISERVER_SSANDBOXCLIENT_H
#define LXISERVER_SSANDBOXCLIENT_H

#include <QtCore>
#include "shttpengine.h"

namespace LXiServer {

class SSandboxClient : public SHttpClientEngine
{
Q_OBJECT
public:
  enum Mode
  {
    Mode_Normal               = 0,
    Mode_Nice
  };

  typedef void (* LogFunc)(const QString &);

public:
  static QString              & sandboxApplication(void);

public:
  explicit                      SSandboxClient(Mode, QObject * = NULL);
  virtual                       ~SSandboxClient();

  void                          setLogFunc(LogFunc);
  const QString               & serverName(void) const;

public: // From HttpClientEngine
  virtual void                  openRequest(const SHttpEngine::RequestHeader &header, QObject *receiver, const char *slot);
  virtual void                  closeRequest(QIODevice *, bool canReuse = false);

private slots:
  void                          openSockets(void);
  void                          stop(void);
  void                          finished(void);
  void                          consoleLine(const QString &);

private:
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
