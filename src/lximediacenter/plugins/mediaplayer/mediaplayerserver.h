/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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
#include "slideshownode.h"

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

class MediaPlayerServerDir;

class MediaPlayerServer : public MediaServer
{
Q_OBJECT
friend class MediaPlayerServerDir;
private:
  struct RootPath
  {
    enum Type { Auto, Music };

    inline                      RootPath() : url(), type(Auto) { }
    inline                      RootPath(const QUrl &url, Type type) : url(url), type(type) { }

    QUrl                        url;
    Type                        type;
  };

  struct DirType
  {
    inline                      DirType(SMediaInfo::ProbeInfo::FileType f, RootPath::Type p = RootPath::Auto) : fileType(f), pathType(p) { }

    SMediaInfo::ProbeInfo::FileType fileType;
    RootPath::Type              pathType;
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

protected: // From HttpCallback
  virtual HttpStatus            httpRequest(const QUrl &request, const UPnP::HttpRequestInfo &requestInfo, QByteArray &contentType, QIODevice *&response);

protected: // From MediaServer
  virtual MediaStream         * streamVideo(const QUrl &request);
  virtual HttpStatus            sendPhoto(const QUrl &request, QByteArray &contentType, QIODevice *&response);

  virtual QList<Item>           listItems(const QString &virtualPath, int start, int &count);
  virtual Item                  getItem(const QString &path);
  virtual ListType              listType(const QString &path);

private:
  void                          setRootPaths(const QList<RootPath> &paths);
  QString                       virtualPath(const QUrl &realPath) const;
  RootPath                      realPath(const QString &virtualPath) const;
  static QString                virtualFile(const QString &virtualPath);
  static QString                dirLabel(const QString &);

  Item                          makeItem(DirType dirType, const SMediaInfo &, int titleId = -1);
  Item                          makePlayAllItem(const QString &virtualPath);
  DirType                       dirType(const QString &virtualPath);

private:
  void                          generateDirs(SStringParser &, const QList<QUrl> &, int, const QStringList &, const QList<RootPath> &);
  void                          scanDrives(void);

private:
  static const int              numRepresentativeItems = 4;
  static const char             dirSplit;
  static const QEvent::Type     responseEventType;
  static const int              maxSongDurationMin;
  MasterServer                * masterServer;
  MediaDatabase               * mediaDatabase;

  QMap<QString, RootPath>       rootPaths;
  QMap<QUrl, QString>           driveList;

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
  static const char             htmlSettingsDirTreeContentType[];
};

class FileStream : public MediaTranscodeStream
{
Q_OBJECT
public:
  explicit                      FileStream(const QUrl &fileName);
  virtual                       ~FileStream();

  bool                          setup(const QUrl &);

public:
  SFileInputNode                file;
};

class PlaylistStream : public MediaTranscodeStream
{
Q_OBJECT
public:
  explicit                      PlaylistStream(const QList<QUrl> &files, SMediaInfo::ProbeInfo::FileType);
  virtual                       ~PlaylistStream();

  bool                          setup(const QUrl &);

public slots:
  void                          opened(const QUrl &);
  void                          closed(const QUrl &);

private:
  QUrl                          currentFile;
  SPlaylistNode                 playlistNode;
};

class SlideShowStream : public MediaStream
{
Q_OBJECT
public:
  explicit                      SlideShowStream(const QList<QUrl> &files);
  virtual                       ~SlideShowStream();

  bool                          setup(const QUrl &);

public:
  SlideShowNode                 slideShow;
};

} } // End of namespaces

#endif
