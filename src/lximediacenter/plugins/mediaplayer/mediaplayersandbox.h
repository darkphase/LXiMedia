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
  class ResponseEvent : public QEvent
  {
  public:
    inline ResponseEvent(
        const SSandboxServer::RequestMessage &request,
        const SHttpServer::ResponseMessage &response,
        QIODevice *socket)
      : QEvent(responseEventType),
        request(request), response(response), socket(socket)
    {
    }

  public:
    const SSandboxServer::RequestHeader request;
    SHttpServer::ResponseMessage response;
    QIODevice           * const socket;
  };

public:
  explicit                      MediaPlayerSandbox(const QString &, QObject *parent = NULL);

  virtual void                  initialize(SSandboxServer *);
  virtual void                  close(void);

public: // From SSandboxServer::Callback
  virtual SSandboxServer::ResponseMessage httpRequest(const SSandboxServer::RequestMessage &, QIODevice *);

protected: // From QObject
  virtual void                  customEvent(QEvent *);

private slots:
  void                          cleanStreams(void);

private:
  typedef void (MediaPlayerSandbox::* TaskFunc)(const SSandboxServer::RequestMessage &, QIODevice *);
  void                          startTask(TaskFunc, const SSandboxServer::RequestMessage &request, QIODevice *);

  void                          listFiles(const SSandboxServer::RequestMessage &request, QIODevice *);
  void                          probeFormat(const SSandboxServer::RequestMessage &request, QIODevice *);
  static SMediaInfo             probeFileFormat(const QUrl &filePath);
  void                          probeContent(const SSandboxServer::RequestMessage &request, QIODevice *);
  static SMediaInfo             probeFileContent(const QUrl &filePath);
  void                          readThumbnail(const SSandboxServer::RequestMessage &request, QIODevice *);
  void                          readImage(const SSandboxServer::RequestMessage &request, QIODevice *);

  SSandboxServer::ResponseMessage play(const SSandboxServer::RequestMessage &request, QIODevice *);
  static SMediaInfo::ProbeInfo::FileType probeFileType(const QUrl &filePath);

  static QByteArray             serializeNodes(const SMediaInfoList &);

public:
  static const char     * const path;

private:
  static const QEvent::Type     responseEventType;
  SSandboxServer              * server;
  QList<MediaStream *>          streams;
  QTimer                        cleanStreamsTimer;
  
  QMutex                        itemCacheMutex;
  QMap<QUrl, QPair<QStringList, QTime> > itemCache;
};

class SandboxFileStream : public MediaTranscodeStream
{
Q_OBJECT
public:
  explicit                      SandboxFileStream(const QUrl &fileName);
  virtual                       ~SandboxFileStream();

  bool                          setup(const SHttpServer::RequestMessage &, QIODevice *);

public:
  SFileInputNode                file;
};

class SandboxPlaylistStream : public MediaTranscodeStream
{
Q_OBJECT
public:
  explicit                      SandboxPlaylistStream(const QList<QUrl> &files, SMediaInfo::ProbeInfo::FileType);

  bool                          setup(const SHttpServer::RequestMessage &, QIODevice *);

public slots:
  void                          opened(const QUrl &);
  void                          closed(const QUrl &);

private:
  QUrl                          currentFile;
  SPlaylistNode                 playlistNode;
};

class SandboxSlideShowStream : public MediaStream
{
Q_OBJECT
public:
  explicit                      SandboxSlideShowStream(const QList<QUrl> &files);
  virtual                       ~SandboxSlideShowStream();

  bool                          setup(const SHttpServer::RequestMessage &, QIODevice *);

public:
  SlideShowNode                 slideShow;
};


} } // End of namespaces

#endif
