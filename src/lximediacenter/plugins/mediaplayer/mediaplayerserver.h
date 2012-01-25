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

  virtual QList<Item>           listItems(const QString &virtualPath, int start, int &count);
  virtual Item                  getItem(const QString &path);
  virtual ListType              listType(const QString &path);

private:
  void                          setRootPaths(const QList<QUrl> &paths);
  QString                       virtualPath(const QUrl &realPath) const;
  QUrl                          realPath(const QString &virtualPath) const;
  static QString                virtualFile(const QString &virtualPath);
  static QString                dirLabel(const QString &);

  Item                          makeItem(const FileNode &, int titleId = -1);
  Item                          makePlayAllItem(const QString &virtualPath);
  FileNode::ProbeInfo::FileType dirType(const QString &virtualPath);

private slots:
  void                          consoleLine(const QString &);
  void                          nodeRead(const FileNode &);
  void                          aborted(void);

private:
  void                          generateDirs(SStringParser &, const QList<QUrl> &, int, const QStringList &, const QList<QUrl> &);
  void                          scanDrives(void);

private:
  static const char             dirSplit;
  static const QEvent::Type     responseEventType;
  static const int              maxSongDurationMin;
  MasterServer                * masterServer;
  MediaDatabase               * mediaDatabase;

  QMap<QString, QUrl>           rootPaths;
  QMap<QUrl, QString>           driveList;

  QMap<QUrl, QPair<SHttpServer::RequestMessage, QIODevice *> > nodeReadQueue;

private:
  static const char             htmlFrontPageContent[];
  static const char             htmlSettingsMain[];
  static const char             htmlSettingsAddNetworkDrive[];

  static const char             htmlSettingsDirTreeIndex[];
  static const char             htmlSettingsDirTreeDir[];
  static const char             htmlSettingsDirTreeIndent[];
  static const char             htmlSettingsDirTreeExpand[];
  static const char             htmlSettingsDirTreeCheck[];
  static const char             htmlSettingsDirTreeCheckLink[];
};

} } // End of namespaces

#endif
