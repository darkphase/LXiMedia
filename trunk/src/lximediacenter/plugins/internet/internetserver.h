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

#ifndef INTRENETSERVER_H
#define INTRENETSERVER_H

#include <QtCore>
#include <LXiStream>
#include <LXiMediaCenter>
#include "sitedatabase.h"

namespace LXiMediaCenter {
namespace InternetBackend {

class MediaPlayerServerDir;

class InternetServer : public MediaServer
{
Q_OBJECT
friend class MediaPlayerServerDir;
protected:
  class Stream : public MediaServer::Stream
  {
  public:
                                Stream(InternetServer *, SSandboxClient *, const QString &url);
    virtual                     ~Stream();

    bool                        setup(const QUrl &request, const QByteArray &content);

  public:
    SSandboxClient      * const sandbox;
  };

public:
                                InternetServer(SiteDatabase::Category, QObject *);

  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  virtual QString               pluginName(void) const;

  virtual SearchResultList      search(const QStringList &) const;

protected: // From MediaServer
  virtual Stream              * streamVideo(const SHttpServer::RequestMessage &);

  virtual int                   countItems(const QString &path);
  virtual QList<Item>           listItems(const QString &path, unsigned start = 0, unsigned count = 0);

protected: // From SHttpServer::Callback
  virtual SHttpServer::SocketOp handleHttpRequest(const SHttpServer::RequestMessage &, QAbstractSocket *);

private:
  Item::Type                    defaultItemType(void) const;

protected:
  const SiteDatabase::Category  category;
  MasterServer                * masterServer;
  SiteDatabase                * siteDatabase;
};

} } // End of namespaces

#endif
