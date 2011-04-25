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

#ifndef __BACKEND_H
#define __BACKEND_H

#include <QtCore>
#include <LXiMediaCenter>


class Backend : public BackendServer::MasterServer,
                protected SHttpServer::Callback
{
Q_OBJECT
private:
  struct SearchCacheEntry
  {
    QMultiMap<qreal, BackendServer::SearchResult> results;
    int                         duration;
    QTime                       update;
  };

public:
                                Backend();
  virtual                       ~Backend();

  void                          start(void);

protected:
  virtual void                  customEvent(QEvent *);

protected: // From SHttpServer::Callback
  virtual SHttpServer::SocketOp  handleHttpRequest(const SHttpServer::RequestMessage &, QAbstractSocket *);

private:
  static QString                createLogDir(void);

  SearchCacheEntry              search(const QString &) const;

  virtual QByteArray            parseHtmlContent(const QUrl &, const QByteArray &content, const QByteArray &head) const;
  virtual SHttpServer         * httpServer(void);
  virtual SSsdpServer         * ssdpServer(void);
  virtual SUPnPContentDirectory * contentDirectory(void);
  virtual ImdbClient          * imdbClient(void);

  virtual void                  ensureSandboxReady(SSandboxClient::Mode);
  virtual SSandboxClient      * createSandbox(SSandboxClient::Mode);
  virtual void                  recycleSandbox(SSandboxClient *);

  SHttpServer::SocketOp         handleCssRequest(const SHttpServer::RequestMessage &, QAbstractSocket *, const QString &);
  SHttpServer::SocketOp         handleHtmlSearch(const SHttpServer::RequestMessage &, QAbstractSocket *, const QString &);
  SHttpServer::SocketOp         handleHtmlRequest(const SHttpServer::RequestMessage &, QAbstractSocket *, const QString &);
  SHttpServer::SocketOp         showAbout(const SHttpServer::RequestMessage &, QAbstractSocket *);
  SHttpServer::SocketOp         handleHtmlConfig(const SHttpServer::RequestMessage &, QAbstractSocket *);

  void                          setContentDirectoryQueryItems(void);

private:
  static const QEvent::Type     exitEventType;
  static const QEvent::Type     restartEventType;
  static const QEvent::Type     shutdownEventType;
  static const QUrl             submitErrorUrl;

  SApplication                  mediaApp;

  SHttpServer                   masterHttpServer;
  SSsdpServer                   masterSsdpServer;
  SUPnPMediaServer              masterMediaServer;
  SUPnPConnectionManager        masterConnectionManager;
  SUPnPContentDirectory         masterContentDirectory;
  ImdbClient                  * masterImdbClient;

  const QString                 sandboxApplication;
  QMap<SSandboxClient::Mode, QList<SSandboxClient *> > sandboxClients;

  HtmlParser                    cssParser;
  HtmlParser                    htmlParser;
  QList<BackendServer *>        backendServers;
  QMap<QString, QList<QPair<QString, QString> > > submenuItems;

  mutable QMap<QString, SearchCacheEntry> searchCache;

private:
  static const char     * const cssMain;
  static const char     * const cssLogin;
  static const char     * const cssMenu;
  static const char     * const csslist;
  static const char     * const csslog;

  static const char     * const htmlIndex;
  static const char     * const htmlMenuItem;
  static const char     * const htmlMenuItemSel;
  static const char     * const htmlSubMenuItem;
  static const char     * const htmlSubMenuItemSel;
  static const char     * const htmlMain;
  static const char     * const htmlWidgetRow;
  static const char     * const htmlWidgetButton;
  static const char     * const htmlSearchWidget;
  static const char     * const htmlSearchResults;
  static const char     * const htmlSearchResultsPage;
  static const char     * const htmlSearchResultsItem;
  static const char     * const htmlSearchResultsItemThumb;
  static const char     * const htmlToolboxWidget;
  static const char     * const htmlErrorLogWidget;
  static const char     * const htmlErrorLogWidgetFile;
  static const char     * const htmlLogFile;
  static const char     * const htmlLogFileHeadline;
  static const char     * const htmlLogFileMessage;

  static const char     * const htmlAbout;

  static const char     * const htmlConfigMain;
  static const char     * const htmlConfigDlnaDefaultRow;
  static const char     * const htmlConfigDlnaClientRow;
  static const char     * const htmlConfigOption;
  static const char     * const htmlConfigImdbDownload;

  static const char     * const headSearchResults;
};


#endif
