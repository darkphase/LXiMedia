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

#ifndef CONFIGSERVER_H
#define CONFIGSERVER_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStream>

namespace LXiMediaCenter {
namespace InternetBackend {

class SiteDatabase;

class ConfigServer : public BackendServer,
                     protected SHttpServer::Callback
{
Q_OBJECT
public:
                                ConfigServer(const QString &, QObject * = NULL);
  virtual                       ~ConfigServer();

  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  virtual QString               serverName(void) const;
  virtual QString               serverIconPath(void) const;

protected: // From SHttpServer::Callback
  virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &, QIODevice *);

private:
  SHttpServer::ResponseMessage  handleHtmlRequest(const SHttpServer::RequestMessage &, const MediaServer::File &);
  void                          generateTree(HtmlParser &, const QFileInfoList &, int, const QSet<QString> &, const QStringList &);

private:
  static const char             dirSplit;
  MasterServer                * masterServer;
  SiteDatabase                * siteDatabase;

private:
  static const char             htmlMain[];

  static const char             htmlTreeIndex[];
  static const char             htmlTreeDir[];
  static const char             htmlTreeIndent[];
  static const char             htmlTreeExpand[];
  static const char             htmlTreeCheckLink[];
  static const char             htmlTreeCheckIcon[];
  static const char             htmlTreeScriptLink[];

  static const char             htmlEditMain[];
  static const char             htmlEditHead[];
};

} } // End of namespaces

#endif
