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

#ifndef MEDIAPLAYERSANDBOX_H
#define MEDIAPLAYERSANDBOX_H

#include <LXiMediaCenter>
#include "slideshownode.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

class MediaPlayerSandbox : public BackendSandbox
{
Q_OBJECT
private:
  class ProbeResponseEvent : public QEvent
  {
  public:
    inline ProbeResponseEvent(const SSandboxServer::RequestMessage &request, QIODevice *socket, const QByteArray &data)
      : QEvent(probeResponseEventType), request(request), socket(socket), data(data)
    {
    }

  public:
    const SSandboxServer::RequestHeader request;
    QIODevice           * const socket;
    const QByteArray            data;
  };

public:
  explicit                      MediaPlayerSandbox(const QString &, QObject *parent = NULL);

  virtual void                  initialize(SSandboxServer *);
  virtual void                  close(void);

public: // From SSandboxServer::Callback
  virtual SSandboxServer::ResponseMessage httpRequest(const SSandboxServer::RequestMessage &, QIODevice *);

protected: // From QObject
  virtual void                  customEvent(QEvent *);

private:
  void                          probe(const SSandboxServer::RequestMessage &request, QIODevice *socket, const QString &file);

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

class SandboxFileStream : public MediaTranscodeStream
{
Q_OBJECT
public:
  explicit                      SandboxFileStream(const QString &fileName);
  virtual                       ~SandboxFileStream();

  bool                          setup(const SHttpServer::RequestMessage &, QIODevice *);

public:
  SFileInputNode                file;
};

class SandboxPlaylistStream : public MediaTranscodeStream
{
Q_OBJECT
public:
  explicit                      SandboxPlaylistStream(const SMediaInfoList &files);

  bool                          setup(const SHttpServer::RequestMessage &, QIODevice *);

public slots:
  void                          opened(const QString &, quint16);
  void                          closed(const QString &, quint16);

private:
  QString                       currentFile;
  SPlaylistNode                 playlistNode;
};

class SandboxSlideShowStream : public MediaStream
{
Q_OBJECT
public:
  explicit                      SandboxSlideShowStream(const SMediaInfoList &files);

  bool                          setup(const SHttpServer::RequestMessage &, QIODevice *);

public:
  SlideShowNode                 slideShow;
};


} } // End of namespaces

#endif
