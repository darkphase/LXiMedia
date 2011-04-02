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

public:
                                SSandboxClient(const QString &application, Mode, QObject * = NULL);
  virtual                       ~SSandboxClient();

  const QString               & serverName(void) const;
  Mode                          mode(void) const;

public: // From HttpClientEngine
  virtual void                  openRequest(const RequestMessage &header, QObject *receiver, const char *slot);
  virtual void                  closeRequest(QIODevice *, bool canReuse = false);

signals:
  void                          consoleLine(const QString &);

private slots:
  void                          openSockets(void);
  void                          stop(void);
  void                          finished(void);

private:
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
