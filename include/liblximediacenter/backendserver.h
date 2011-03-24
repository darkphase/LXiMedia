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

namespace LXiMediaCenter {

class ImdbClient;
class Plugin;

class BackendServer : public QObject
{
Q_OBJECT
public:
  class MasterServer : public QObject
  {
  public:
    virtual QByteArray          parseHtmlContent(const QUrl &url, const QByteArray &content, const QByteArray &head) const = 0;

    virtual SHttpServer       * httpServer(void) = 0;
    virtual SSsdpServer       * ssdpServer(void) = 0;
    virtual SUPnPContentDirectory * contentDirectory(void) = 0;
    virtual ImdbClient        * imdbClient(void) = 0;
  };

  struct SearchResult
  {
    inline                      SearchResult(void) : relevance(0.0)             { }

    qreal                       relevance;
    QString                     headline;
    QByteArray                  location;
    QString                     text;
    QByteArray                  thumbLocation;
  };

  typedef QList<SearchResult>   SearchResultList;

public:
                                BackendServer(const char *name, Plugin *, MasterServer *);
  virtual                       ~BackendServer();

  virtual QByteArray            frontPageWidget(void) const;
  virtual SearchResultList      search(const QStringList &) const;

  MasterServer                * masterServer(void) const;
  const QString               & name(void) const;
  const QString               & httpPath(void) const;
  const QString               & contentDirPath(void) const;

  SHttpServer::SocketOp         sendResponse(const SHttpServer::RequestHeader &, QIODevice *, const QByteArray &, const char * = dataMime, bool allowCache = false, const QString &redir = QString::null) const;
  SHttpServer::SocketOp         sendResponse(const SHttpServer::RequestHeader &, QIODevice *, const QString &, const char * = textMime, bool allowCache = false, const QString &redir = QString::null) const;
  SHttpServer::SocketOp         sendHtmlContent(QIODevice *, const QUrl &, const SHttpServer::ResponseHeader &, const QByteArray &content, const QByteArray &head = QByteArray()) const;

public:
  static const int              maxRequestTime;
  static const qreal            minSearchRelevance;
  static const char     * const searchDateFormat;
  static const char     * const searchTimeFormat;
  static const char     * const searchDateTimeFormat;

private:
  static const char     * const dataMime;
  static const char     * const textMime;

  struct Private;
  Private               * const p;

protected:
  static const char     * const htmlFrontPageWidget;
};


} // End of namespace

#endif
