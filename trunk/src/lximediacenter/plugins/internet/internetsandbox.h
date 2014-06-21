/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#ifndef INTERNETSANDBOX_H
#define INTERNETSANDBOX_H

#include <LXiMediaCenter>
#include "streaminputnode.h"

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
  virtual SSandboxServer::ResponseMessage httpRequest(const SSandboxServer::RequestMessage &, QIODevice *);

private slots:
  void                          cleanStreams(void);

public:
  static const char     * const path;

private:
  SSandboxServer              * server;
  QList<MediaStream *>          streams;
  QTimer                        cleanStreamsTimer;
};

class SandboxNetworkStream : public MediaStream
{
Q_OBJECT
public:
  explicit                      SandboxNetworkStream(const QUrl &url);
  virtual                       ~SandboxNetworkStream();

  bool                          setup(const SHttpServer::RequestMessage &, QIODevice *);

public:
  const QUrl                    url;
  StreamInputNode               streamInput;
  SAudioDecoderNode             audioDecoder;
  SVideoDecoderNode             videoDecoder;
  SDataDecoderNode              dataDecoder;
  SVideoGeneratorNode           videoGenerator;
};

} } // End of namespaces

#endif
