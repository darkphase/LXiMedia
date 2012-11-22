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

#ifndef CAMERASANDBOX_H
#define CAMERASANDBOX_H

#include <LXiMediaCenter>
#include <LXiStreamDevice>

namespace LXiMediaCenter {
namespace CameraBackend {

class CameraSandbox : public BackendSandbox
{
Q_OBJECT
public:
  explicit                      CameraSandbox(const QString &, QObject *parent = NULL);

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
  QList<SGraph *>               streams;
  QTimer                        cleanStreamsTimer;
};

class CameraStream : public MediaStream
{
Q_OBJECT
public:
  explicit                      CameraStream(const QString &device);
  virtual                       ~CameraStream();

  bool                          setup(const SHttpServer::RequestMessage &, QIODevice *);

protected:
  static SSize                  toWebcamSize(const SSize &);

public:
  SAudioVideoInputNode          input;
};

} } // End of namespaces

#endif
