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

#ifndef BACKEND_H
#define BACKEND_H

#include <QtCore>
#include <LXiMediaCenter>

// This starts the sandboxes in the local process, so they can be debugged.
#ifndef QT_NO_DEBUG
//#define DEBUG_USE_LOCAL_SANDBOX
#endif

class Backend : public QObject,
                protected BackendServer::MasterServer,
                protected SHttpServer::Callback
{
Q_OBJECT
private:
  struct MenuItem
  {
    inline MenuItem(const QString &title, const QString &url, const QString &iconurl)
      :title(title), url(url), iconurl(iconurl)
    {
    }

    QString                     title;
    QString                     url;
    QString                     iconurl;
  };

  struct SearchCacheEntry
  {
    QMultiMap<qreal, BackendServer::SearchResult> results;
    int                         duration;
    QTime                       update;
  };

public:
  static QString                createLogDir(void);

                                Backend();
  virtual                       ~Backend();

  void                          start(void);
  void                          reset(void);

private slots:
  void                          start(const SHttpEngine::ResponseMessage &);
  void                          addModules(const SHttpEngine::ResponseMessage &);

protected:
  virtual void                  customEvent(QEvent *);

protected: // From SHttpServer::Callback
  virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &, QIODevice *);

private:
  SearchCacheEntry              search(const QString &) const;

  virtual QByteArray            parseHtmlContent(const QUrl &, const QByteArray &content, const QByteArray &head) const;

  virtual SHttpServer         * httpServer(void);
  virtual SSsdpServer         * ssdpServer(void);
  virtual SUPnPContentDirectory * contentDirectory(void);
  virtual ImdbClient          * imdbClient(void);

  virtual SSandboxClient      * createSandbox(SSandboxClient::Priority);
  virtual void                  recycleSandbox(SSandboxClient *);

  QByteArray                    parseHtmlLogErrors(void) const;

  SHttpServer::ResponseMessage  handleHtmlRequest(const SHttpServer::RequestMessage &, const MediaServer::File &);
  SHttpServer::ResponseMessage  handleHtmlRequest(const SHttpServer::RequestMessage &, const QString &);
  SHttpServer::ResponseMessage  handleHtmlSearch(const SHttpServer::RequestMessage &, const MediaServer::File &);
  SHttpServer::ResponseMessage  handleHtmlLogFileRequest(const SHttpServer::RequestMessage &, const MediaServer::File &);
  SHttpServer::ResponseMessage  showAbout(const SHttpServer::RequestMessage &);
  SHttpServer::ResponseMessage  handleHtmlConfig(const SHttpServer::RequestMessage &);

private:
  static const QEvent::Type     exitEventType;
  static const QEvent::Type     restartEventType;
  static const QEvent::Type     shutdownEventType;
  static const QUrl             submitErrorUrl;

  SHttpServer                   masterHttpServer;
  SSsdpServer                   masterSsdpServer;
  SUPnPMediaServer              masterMediaServer;
  SUPnPConnectionManager        masterConnectionManager;
  SUPnPContentDirectory         masterContentDirectory;
  SUPnPMediaReceiverRegistrar   masterMediaReceiverRegistrar;
  ImdbClient                  * masterImdbClient;

  const QString                 sandboxApplication;

  HtmlParser                    cssParser;
  HtmlParser                    htmlParser;
  QList<BackendServer *>        backendServers;
  QMap<QString, QList<MenuItem> > submenuItems;
  QByteArray                    menuHtml;

  SSandboxClient              * initSandbox;

  mutable QMap<QString, SearchCacheEntry> searchCache;

private:
  static const char     * const cssMain;
  static const char     * const cssLogin;
  static const char     * const cssMenu;
  static const char     * const csslist;
  static const char     * const csslog;

  static const char     * const htmlIndex;
  static const char     * const htmlLocationItem;
  static const char     * const htmlMenuGroup;
  static const char     * const htmlMenuItem;
  static const char     * const htmlMain;
  static const char     * const htmlMainGroupItem;
  static const char     * const htmlMainServerItem;
  static const char     * const htmlSearchResults;
  static const char     * const htmlSearchResultsItem;
  static const char     * const htmlLogFile;
  static const char     * const htmlLogFileHeadline;
  static const char     * const htmlLogFileMessage;

  static const char     * const htmlAbout;

  static const char     * const htmlConfigMain;
  static const char     * const htmlConfigDlnaRow;
  static const char     * const htmlConfigDlnaRowProfilesClosed;
  static const char     * const htmlConfigDlnaRowProfiles;
  static const char     * const htmlConfigDlnaRowProfilesCheck;
  static const char     * const htmlConfigOption;
  static const char     * const htmlConfigImdbDownload;
};

#endif
