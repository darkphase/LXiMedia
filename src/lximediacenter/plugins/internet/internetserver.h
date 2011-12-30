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

#ifndef INTRENETSERVER_H
#define INTRENETSERVER_H

#include <QtCore>
#include <LXiStream>
#include <LXiMediaCenter>
#include "sitedatabase.h"

namespace LXiMediaCenter {
namespace InternetBackend {

class ScriptEngine;

class InternetServer : public MediaServer
{
Q_OBJECT
friend class MediaPlayerServerDir;
protected:
  class Stream : public MediaServer::Stream
  {
  public:
                                Stream(InternetServer *, SSandboxClient *, const QString &url);
    virtual                     ~Stream();

    bool                        setup(const QUrl &request, const QByteArray &content);

  public:
    SSandboxClient      * const sandbox;
  };

public:
                                InternetServer(const QString &category, QObject *);

  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  virtual QString               serverName(void) const;
  virtual QString               serverIconPath(void) const;

  virtual QByteArray            frontPageContent(void);
  virtual QByteArray            settingsContent(void);

protected: // From MediaServer
  virtual Stream              * streamVideo(const SHttpServer::RequestMessage &);
  virtual SHttpServer::ResponseMessage sendPhoto(const SHttpServer::RequestMessage &);

  virtual QList<Item>           listItems(const QString &path, int start, int &count);
  virtual Item                  getItem(const QString &path);

protected: // From SHttpServer::Callback
  virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &, QIODevice *);

private:
  SHttpServer::ResponseMessage  editRequest(const SHttpServer::RequestMessage &, const QString &host, const QString &script, const QString & = QString::null);
  QString                       sitePath(const QString &path) const;
  ScriptEngine                * getScriptEngine(const QString &host);
  void                          deleteScriptEngine(const QString &host);

private:
  static const char             dirSplit;
  MasterServer                * masterServer;
  SiteDatabase                * siteDatabase;
  QMap<QString, ScriptEngine *> scriptEngines;

private:
  static const char             htmlFrontPageContent[];
  static const char             htmlSettingsMain[];

  static const char             htmlSiteTreeIndex[];
  static const char             htmlSiteTreeDir[];
  static const char             htmlSiteTreeIndent[];
  static const char             htmlSiteTreeExpand[];
  static const char             htmlSiteTreeCheckLink[];
  static const char             htmlSiteTreeCheckIcon[];
  static const char             htmlSiteTreeScriptLink[];

  static const char             htmlSiteEditIndex[];
  static const char             htmlSiteEditButton[];
  static const char             htmlSiteEditCloseIndex[];
};

} } // End of namespaces

#endif
