/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#ifndef BACKEND_H
#define BACKEND_H

#include <QtCore>
#include <LXiMediaCenter>
#include "pupnprootdevice.h"

class Backend : public QObject,
                protected BackendServer::MasterServer,
                protected RootDevice::HttpCallback
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

protected:
  virtual void                  customEvent(QEvent *);

protected: // From RootDevice::HttpCallback
  virtual HttpStatus            httpRequest(const QUrl &request, QByteArray &contentType, QIODevice *&response);

protected: // From BackendServer::MasterServer
  virtual HttpStatus            parseHtmlContent(const QUrl &, const QByteArray &content, const QByteArray &head, QByteArray &contentType, QIODevice *&response) const;

  virtual RootDevice          * rootDevice(void);
  virtual ContentDirectory    * contentDirectory(void);

private:
  static QUuid                  serverUuid(void);
  static QString                defaultDeviceName(void);

  HttpStatus                    sendFile(const QUrl &, const QString &, QByteArray &, QIODevice *&);
  QByteArray                    handleHtmlSettings(const QUrl &);
  void                          saveHtmlSettings(const QUrl &);

private:
  static const quint16          defaultPort = 4280;
#if !defined(QT_NO_DEBUG) || defined(Q_OS_MACX)
  static const QEvent::Type     exitEventType;
#endif

  PupnpRootDevice               upnpRootDevice;
  ConnectionManager             upnpConnectionManager;
  ContentDirectory              upnpContentDirectory;
  MediaReceiverRegistrar        upnpMediaReceiverRegistrar;

  const QString                 sandboxApplication;

  SStringParser                 cssParser;
  SStringParser                 htmlParser;
  QList<BackendServer *>        backendServers;

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
