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

#ifndef MEDIAPLAYERSERVER_H
#define MEDIAPLAYERSERVER_H

#include <QtCore>
#include <LXiStream>
#include <LXiMediaCenter>
#include "mediadatabase.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

class MediaPlayerServerDir;

class MediaPlayerServer : public MediaServer
{
Q_OBJECT
friend class MediaPlayerServerDir;
private:
  class ResponseEvent : public QEvent
  {
  public:
    inline ResponseEvent(
        const SSandboxServer::RequestMessage &request,
        const SHttpServer::ResponseMessage &response,
        QIODevice *socket)
      : QEvent(responseEventType),
        request(request), response(response), socket(socket)
    {
    }

  public:
    const SSandboxServer::RequestHeader request;
    SHttpServer::ResponseMessage response;
    QIODevice           * const socket;
  };

protected:
  class Stream : public MediaServer::Stream
  {
  public:
                                Stream(MediaPlayerServer *, SSandboxClient *, const QString &url);
    virtual                     ~Stream();

    bool                        setup(const QUrl &request, const QByteArray &content);

  public:
    SSandboxClient      * const sandbox;
  };

public:
                                MediaPlayerServer(const QString &, QObject *);

protected: // From BackendServer
  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  virtual QString               serverName(void) const;
  virtual QString               serverIconPath(void) const;

  virtual QByteArray            frontPageContent(void);
  virtual QByteArray            settingsContent(void);

protected: // From SHttpServer::Callback
  virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &, QIODevice *);

protected: // From MediaServer
  virtual Stream              * streamVideo(const SHttpServer::RequestMessage &);
  virtual SHttpServer::ResponseMessage sendPhoto(const SHttpServer::RequestMessage &);

  virtual int                   countItems(const QString &virtualPath);
  virtual QList<Item>           listItems(const QString &virtualPath, unsigned start = 0, unsigned count = 0);
  virtual Item                  getItem(const QString &path);

protected: // From QObject
  virtual void                  customEvent(QEvent *);

private:
  static bool                   isHidden(const QString &path);

  void                          setRootPaths(const QStringList &paths);
  QString                       virtualPath(const QString &realPath) const;
  QString                       realPath(const QString &virtualPath) const;

  int                           countAlbums(const QString &virtualPath);
  QList<Item>                   listAlbums(const QString &virtualPath, unsigned &start, unsigned &count);
  Item                          makeItem(const FileNode &);
  Item                          makePlayAllItem(const QString &virtualPath);

private slots:
  void                          consoleLine(const QString &);

private:
  void                          generateDirs(HtmlParser &, const QFileInfoList &, int, const QStringList &, const QStringList &);
  void                          scanDrives(void);

private:
  static const char             dirSplit;
  static const Qt::CaseSensitivity caseSensitivity;
  static const QEvent::Type     responseEventType;
  static const int              maxSongDurationMin;
  MasterServer                * masterServer;
  MediaDatabase               * mediaDatabase;

  QMap<QString, QString>        rootPaths;
  QMap<QString, QFileInfo>      driveInfoList;
  QMap<QString, QString>        driveLabelList;

private:
  static const char             htmlFrontPageContent[];
  static const char             htmlSettingsMain[];

  static const char             htmlSettingsDirTreeIndex[];
  static const char             htmlSettingsDirTreeDir[];
  static const char             htmlSettingsDirTreeIndent[];
  static const char             htmlSettingsDirTreeExpand[];
  static const char             htmlSettingsDirTreeCheck[];
  static const char             htmlSettingsDirTreeCheckLink[];
};

} } // End of namespaces

#endif
