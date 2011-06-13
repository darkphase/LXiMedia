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

  virtual QString               pluginName(void) const;
  virtual QString               serverName(void) const;
  virtual QString               serverIconPath(void) const;

protected: // From SHttpServer::Callback
  virtual SHttpServer::SocketOp handleHttpRequest(const SHttpServer::RequestMessage &, QAbstractSocket *);
  virtual void                  handleHttpOptions(SHttpServer::ResponseHeader &);

private:
  SHttpServer::SocketOp         handleHtmlRequest(const SHttpServer::RequestMessage &, QAbstractSocket *, const QString &);
  void                          generateTree(HtmlParser &, const QFileInfoList &, int, const QSet<QString> &, const QStringList &);

private:
  static const char             dirSplit;
  MasterServer                * masterServer;
  SiteDatabase                * siteDatabase;

private:
  static const char     * const htmlMain;
  static const char     * const htmlTreeIndex;
  static const char     * const htmlTreeDir;
  static const char     * const htmlTreeIndent;
  static const char     * const htmlTreeExpand;
  static const char     * const htmlTreeCheckLink;
  static const char     * const htmlTreeEditItemLink;
  static const char     * const htmlEditIndex;
};

} } // End of namespaces

#endif
