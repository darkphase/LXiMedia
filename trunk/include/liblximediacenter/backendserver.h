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
#include "dlnaserver.h"
#include "httpserver.h"

namespace LXiMediaCenter {

class Plugin;

class BackendServer : public QObject
{
Q_OBJECT
public:
  class MasterServer : public QObject
  {
  public:
    virtual QByteArray          parseHtmlContent(const QUrl &url, const QByteArray &content, const QByteArray &head) const = 0;

    virtual HttpServer        * httpServer(void) = 0;
    virtual SsdpServer        * ssdpServer(void) = 0;
    virtual DlnaServer        * dlnaServer(void) = 0;
    virtual QThreadPool       * ioThreadPool(void) = 0;
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

protected:
  class HttpDir : public HttpServerDir
  {
  friend class BackendServer;
  public:
                                HttpDir(HttpServer *server, BackendServer *parent);

  protected:
    inline bool                 superHandleConnection(const QHttpRequestHeader &r, QAbstractSocket *s) { return HttpServerDir::handleConnection(r, s); }
    virtual bool                handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

  private:
    BackendServer       * const parent;
  };

public:
                                BackendServer(const char *name, Plugin *, MasterServer *);
  virtual                       ~BackendServer();

  virtual QByteArray            frontPageWidget(void) const;
  virtual SearchResultList      search(const QStringList &) const;

  MasterServer                * masterServer(void) const;
  const QString               & name(void) const;
  const QString               & httpPath(void) const;
  const QString               & dlnaPath(void) const;

  bool                          sendReply(QAbstractSocket *, const QByteArray &, const char * = dataMime, bool allowCache = false, const QString &redir = QString::null) const __attribute__((nonnull(1, 2)));
  bool                          sendReply(QAbstractSocket *, const QString &, const char * = textMime, bool allowCache = false, const QString &redir = QString::null) const __attribute__((nonnull(1, 2)));
  bool                          sendHtmlContent(QAbstractSocket *, const QUrl &, const QHttpResponseHeader &, const QByteArray &content, const QByteArray &head = QByteArray()) const __attribute__((nonnull));

public slots:
  void                          startDlnaUpdate(void);

protected:
  void                          enableDlna(void);
  virtual void                  updateDlnaTask(void);

  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

public:
  static const int              maxRequestTime;
  static const qreal            minSearchRelevance;
  static const char     * const searchDateFormat;
  static const char     * const searchTimeFormat;
  static const char     * const searchDateTimeFormat;

protected:
  HttpDir                       httpDir;
  DlnaServerAlphaDir            dlnaDir;

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