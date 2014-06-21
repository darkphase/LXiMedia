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

#ifndef TELEVISIONSANDBOX_H
#define TELEVISIONSANDBOX_H

#include <LXiMediaCenter>
#include <LXiStreamDevice>

namespace LXiMediaCenter {
namespace TelevisionBackend {

class TelevisionSandbox : public BackendSandbox
{
Q_OBJECT
public:
  explicit                      TelevisionSandbox(const QString &, QObject *parent = NULL);

  virtual void                  initialize(SSandboxServer *);
  virtual void                  close(void);

public: // From SSandboxServer::Callback
  virtual SSandboxServer::ResponseMessage httpRequest(const SSandboxServer::RequestMessage &, QIODevice *);
  virtual void                  handleHttpOptions(SHttpServer::ResponseHeader &);

private slots:
  void                          cleanStreams(void);

public:
  static const char     * const path;

private:
  static const QEvent::Type     probeResponseEventType;

  SSandboxServer              * server;
  QList<MediaStream *>          streams;
  QTimer                        cleanStreamsTimer;
};

class SandboxInputStream : public MediaStream
{
Q_OBJECT
public:
  explicit                      SandboxInputStream(const QString &device);
  virtual                       ~SandboxInputStream();

  bool                          setup(const SHttpServer::RequestMessage &, QIODevice *);

public:
  SAudioVideoInputNode          input;
};


} } // End of namespaces

#endif
