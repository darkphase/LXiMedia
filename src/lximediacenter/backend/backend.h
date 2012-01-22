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

protected: // From BackendServer::MasterServer
  virtual QByteArray            parseHtmlContent(const SHttpServer::RequestHeader &, const QByteArray &content, const QByteArray &head) const;

  virtual SHttpServer         * httpServer(void);
  virtual SSsdpServer         * ssdpServer(void);
  virtual SUPnPContentDirectory * contentDirectory(void);

  virtual SSandboxClient      * createSandbox(SSandboxClient::Priority);

private:
  static QUuid                  serverUuid(void);
  static QString                defaultDeviceName(void);

  SHttpServer::ResponseMessage  sendFile(const SHttpServer::RequestMessage &, const QString &fileName);
  QByteArray                    handleHtmlSettings(const SHttpServer::RequestMessage &);
  void                          saveHtmlSettings(const SHttpServer::RequestMessage &);

private:
  static const quint16          defaultPort = 4280;
#if !defined(QT_NO_DEBUG) || defined(Q_OS_MACX)
  static const QEvent::Type     exitEventType;
#endif

  SHttpServer                   masterHttpServer;
  SSsdpServer                   masterSsdpServer;
  SUPnPMediaServer              masterMediaServer;
  SUPnPConnectionManager        masterConnectionManager;
  SUPnPContentDirectory         masterContentDirectory;
  SUPnPMediaReceiverRegistrar   masterMediaReceiverRegistrar;

  const QString                 sandboxApplication;

  SStringParser                 cssParser;
  SStringParser                 htmlParser;
  QList<BackendServer *>        backendServers;

  SSandboxClient              * initSandbox;

private:
  static const char             htmlIndex[];
  static const char             htmlNavigatorRoot[];
  static const char             htmlNavigatorPath[];
  static const char             htmlNavigatorItem[];
  static const char             htmlNavigatorButton[];
  static const char             htmlFrontPagesHead[];
  static const char             htmlFrontPages[];
  static const char             htmlFrontPageItem[];
  static const char             htmlLogFile[];
  static const char             htmlLogFileHeadline[];
  static const char             htmlLogFileMessage[];

  static const char             htmlSettingsMain[];
  static const char             htmlSettingsDlnaRow[];
  static const char             htmlSettingsDlnaRowProfilesClosed[];
  static const char             htmlSettingsDlnaRowProfiles[];
  static const char             htmlSettingsDlnaRowProfilesCheck[];
  static const char             htmlSettingsOption[];

  static const char             htmlHelpHead[];
  static const char             htmlHelpContents[];
};

#endif
