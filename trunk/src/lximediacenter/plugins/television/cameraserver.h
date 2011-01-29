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

#ifndef CAMERASERVER_H
#define CAMERASERVER_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStream>

namespace LXiMediaCenter {

class TelevisionBackend;

class CameraServer : public VideoServer
{
Q_OBJECT
protected:
  class CameraStream : public Stream
  {
  public:
                                CameraStream(CameraServer *, const QHostAddress &peer, const QString &url, const QString &camera);

  public:
    SAudioVideoInputNode        input;
  };

public:
                                CameraServer(TelevisionBackend *, MasterServer *, const QStringList &);
  virtual                       ~CameraServer();

  inline bool                   hasCameras(void) const                           { return !cameras.isEmpty(); }

  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

protected:
  virtual bool                  streamVideo(const QHttpRequestHeader &, QAbstractSocket *);
  virtual bool                  buildPlaylist(const QHttpRequestHeader &, QAbstractSocket *);

private:
  bool                          handleHtmlRequest(const QUrl &, const QString &, QAbstractSocket *);

private:
  TelevisionBackend     * const plugin;
  const QStringList             cameras;
};

} // End of namespace

#endif