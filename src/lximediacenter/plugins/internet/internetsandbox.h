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

#ifndef INTERNETSANDBOX_H
#define INTERNETSANDBOX_H

#include <LXiMediaCenter>

namespace LXiMediaCenter {
namespace InternetBackend {

class InternetSandbox : public BackendSandbox
{
Q_OBJECT
public:
  explicit                      InternetSandbox(const QString &, QObject *parent = NULL);

  virtual void                  initialize(SSandboxServer *);
  virtual void                  close(void);

public: // From SSandboxServer::Callback
  virtual SSandboxServer::SocketOp handleHttpRequest(const SSandboxServer::RequestMessage &, QIODevice *);
  virtual void                  handleHttpOptions(SHttpServer::ResponseHeader &);

private slots:
  void                          cleanStreams(void);

public:
  static const char     * const path;

private:
  SSandboxServer              * server;
  QList<MediaStream *>          streams;
  QTimer                        cleanStreamsTimer;
};

class SandboxNetworkStream : public MediaTranscodeStream
{
Q_OBJECT
public:
  explicit                      SandboxNetworkStream(const QUrl &url);
  virtual                       ~SandboxNetworkStream();

  bool                          setup(const SHttpServer::RequestMessage &, QIODevice *);

public:
  SNetworkInputNode             source;
};

} } // End of namespaces

#endif
