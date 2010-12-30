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
private:
  struct Camera
  {
    inline Camera(quint32 id, STerminals::AudioVideoDevice *terminal = NULL)
        : id(id), terminal(terminal)
    {
    }

    const quint32               id;
    STerminals::AudioVideoDevice * terminal;
  };

public:
                                CameraServer(MasterServer *server, TelevisionBackend *);
  virtual                       ~CameraServer();

  inline bool                   hasCameras(void) const                           { return !cameras.isEmpty(); }

  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

protected:
  virtual bool                  streamVideo(const QHttpRequestHeader &, QAbstractSocket *, const StreamRequest &);

private:
  bool                          handleHtmlRequest(const QUrl &, const QString &, QAbstractSocket *);

private:
  TelevisionBackend     * const plugin;

  QMap<QString, Camera *>       cameras;
};

} // End of namespace

#endif
