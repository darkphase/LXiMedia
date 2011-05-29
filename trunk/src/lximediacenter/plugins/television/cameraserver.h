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
#include <LXiStreamDevice>

namespace LXiMediaCenter {
namespace TelevisionBackend {

class CameraServer : public MediaServer
{
Q_OBJECT
public:
                                CameraServer(const QString &, QObject *);

  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  virtual QString               pluginName(void) const;
  virtual QString               serverName(void) const;
  virtual QString               serverIconPath(void) const;

  virtual SearchResultList      search(const QStringList &) const;

protected: // From MediaServer
  virtual Stream              * streamVideo(const SHttpServer::RequestMessage &);

  virtual int                   countItems(const QString &path);
  virtual QList<Item>           listItems(const QString &path, unsigned start = 0, unsigned count = 0);

protected: // From SHttpServer::Callback
  virtual SHttpServer::SocketOp handleHttpRequest(const SHttpServer::RequestMessage &, QAbstractSocket *);

private:
  MasterServer                * masterServer;
  const QStringList             cameras;
};

} } // End of namespaces

#endif
