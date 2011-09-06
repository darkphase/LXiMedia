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

#ifndef LXMEDIACENTER_BACKENDSERVER_H
#define LXMEDIACENTER_BACKENDSERVER_H

#include <QtCore>
#include <LXiServer>
#include "export.h"

namespace LXiMediaCenter {

class ImdbClient;

class LXIMEDIACENTER_PUBLIC BackendServer : public QObject
{
Q_OBJECT
S_FACTORIZABLE(BackendServer)
public:
  class LXIMEDIACENTER_PUBLIC MasterServer
  {
  public:
    virtual QByteArray          parseHtmlContent(const QUrl &url, const QByteArray &content, const QByteArray &head) const = 0;

    virtual SHttpServer       * httpServer(void) = 0;
    virtual SSsdpServer       * ssdpServer(void) = 0;
    virtual SUPnPContentDirectory * contentDirectory(void) = 0;
    virtual ImdbClient        * imdbClient(void) = 0;

    virtual SSandboxClient    * createSandbox(SSandboxClient::Priority) = 0;
    virtual void                recycleSandbox(SSandboxClient *) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC SearchResult
  {
                                SearchResult(void);
                                ~SearchResult();

    qreal                       relevance;
    QString                     headline;
    QString                     location;
    QString                     thumbLocation;
  };

  typedef QList<SearchResult>   SearchResultList;

public:
  /*! Creates all registred BackendServers.
      \param parent   The parent object, or NULL if none.
   */
  static QList<BackendServer *> create(QObject *parent);

public:
  explicit                      BackendServer(QObject * = NULL);
  virtual                       ~BackendServer();

  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  virtual QString               pluginName(void) const = 0;
  virtual QString               serverName(void) const = 0;
  virtual QString               serverIconPath(void) const = 0;
  virtual QString               serverPath(void) const;

  virtual QByteArray            frontPageWidget(void) const;
  virtual SearchResultList      search(const QStringList &rawQuery) const;

public:
  SHttpServer::ResponseMessage  makeResponse(const SHttpServer::RequestHeader &, const QByteArray &, const char * = SHttpEngine::mimeAppOctet, bool allowCache = false) const;
  SHttpServer::ResponseMessage  makeResponse(const SHttpServer::RequestHeader &, const QString &, const char * = SHttpEngine::mimeTextPlain, bool allowCache = false) const;
  SHttpServer::ResponseMessage  makeHtmlContent(const SHttpServer::RequestHeader &, const QUrl &, const QByteArray &content, const QByteArray &head = QByteArray()) const;

  QString                       basePath(const QString &) const;
  static QString                dirName(const QString &);

public:
  static const int              maxRequestTime;
  static const qreal            minSearchRelevance;
  static const char             searchDateFormat[];
  static const char             searchTimeFormat[];
  static const char             searchDateTimeFormat[];

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
