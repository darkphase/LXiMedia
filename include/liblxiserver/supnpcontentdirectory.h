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

#ifndef LXISERVER_SUPNPCONTENTDIRECTORY_H
#define LXISERVER_SUPNPCONTENTDIRECTORY_H

#include <QtCore>
#include <QtNetwork>
#include <QtXml>
#include "shttpserver.h"
#include "supnpbase.h"

namespace LXiServer {

class SUPnPContentDirectory : public SUPnPBase
{
Q_OBJECT
public:
  struct Item
  {
    enum Type
    {
      Type_None                 = 0,
      Type_Playlist,

      Type_Audio                = 10,
      Type_Music,
      Type_AudioBroadcast,
      Type_AudioBook,

      Type_Video                = 20,
      Type_Movie,
      Type_VideoBroadcast,
      Type_MusicVideo,

      Type_Image                = 30,
      Type_Photo
    };

    struct Stream
    {
      inline Stream(quint32 id = 0, const QString &lang = QString::null)
        : id(id), lang(lang)
      {
      }

      quint32                   id;
      QString                   lang;
    };

    struct Chapter
    {
      inline Chapter(const QString &title = QString::null, unsigned position = 0)
        : title(title), position(position)
      {
      }

      QString                   title;
      unsigned                  position;
    };

    inline Item(void)
      : isDir(false), played(false), direct(false), type(Type_None), duration(0)
    {
    }

    inline bool                 isNull(void) const                              { return url.isEmpty(); }

    bool                        isDir;
    bool                        played;
    bool                        direct;
    quint8                      type;
    QUrl                        url;
    QUrl                        iconUrl;

    QString                     title;
    QString                     artist;
    QString                     album;
    int                         track;

    QList<Stream>               audioStreams;
    QList<Stream>               videoStreams;
    QList<Stream>               subtitleStreams;

    unsigned                    duration; //!< In seconds
    QList<Chapter>              chapters;
  };

  struct Callback
  {
    virtual int                 countContentDirItems(const QString &path) = 0;
    virtual QList<Item>         listContentDirItems(const QString &path, unsigned start = 0, unsigned count = 0) = 0;
  };

public:
  explicit                      SUPnPContentDirectory(const QString &basePath, QObject * = NULL);
  virtual                       ~SUPnPContentDirectory();

  void                          initialize(SHttpServer *, SUPnPMediaServer *);
  void                          close(void);

  void                          setProtocols(ProtocolType, const ProtocolList &);
  void                          setQueryItems(const QString &peer, const QMap<QString, QString> &);
  QMap<QString, QString>        activeClients(void) const;

  void                          registerCallback(const QString &path, Callback *);
  void                          unregisterCallback(Callback *);

public slots:
  void                          modified(void);

protected: // From SUPnPBase
  virtual void                  buildDescription(QDomDocument &, QDomElement &);
  virtual void                  handleSoapMessage(const QDomElement &, QDomDocument &, QDomElement &, const SHttpServer::RequestHeader &, const QHostAddress &);

private:
  void                          handleBrowse(const QDomElement &, QDomDocument &, QDomElement &, const SHttpServer::RequestHeader &, const QHostAddress &);
  QDomElement                   didlDirectory(QDomDocument &, Item::Type, const QString &path, const QString &title = QString::null);
  QDomElement                   didlFile(QDomDocument &doc, const QString &peer, const QString &host, const Item &, const QString &path, const QString &title = QString::null);
  void                          emitEvent(bool dirty);

  static QStringList            streamItems(const Item &);
  static QStringList            playSeekItems(const Item &);
  static QStringList            seekItems(const Item &);
  static QStringList            chapterItems(const Item &);
  static QStringList            splitItemProps(const QString &);
  static Item                   makePlayItem(const Item &, const QStringList &);

  static QString                baseDir(const QString &);
  static QString                parentDir(const QString &);
  QString                       toObjectID(const QString &path);
  QString                       fromObjectID(const QString &id);

public:
  static const char     * const contentDirectoryNS;

private:
  static const unsigned         seekSec;

  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
