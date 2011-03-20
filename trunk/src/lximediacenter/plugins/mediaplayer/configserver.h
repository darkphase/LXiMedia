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

class MediaPlayerBackend;

class ConfigServer : public BackendServer,
                     protected HttpServer::Callback
{
Q_OBJECT
public:
                                ConfigServer(Plugin *, MasterServer *server);
  virtual                       ~ConfigServer();

  static const QSet<QString>  & hiddenDirs(void) __attribute__((pure));

protected: // From HttpServer::Callback
  virtual HttpServer::SocketOp  handleHttpRequest(const HttpServer::RequestHeader &, QIODevice *);

private:
  HttpServer::SocketOp          handleHtmlRequest(const HttpServer::RequestHeader &, QIODevice *, const QString &);
  void                          generateDirs(HtmlParser &, const QFileInfoList &, int, const QSet<QString> &, const QStringList &);

  static const QFileInfoList  & drives(bool rescan = false);

private:
  static const char             dirSplit;
  Plugin                * const plugin;
  mutable QReadWriteLock        lock;

private:
  static const char     * const htmlMain;
  static const char     * const htmlDirTreeIndex;
  static const char     * const htmlDirTreeDir;
  static const char     * const htmlDirTreeIndent;
  static const char     * const htmlDirTreeExpand;
  static const char     * const htmlDirTreeCheck;
  static const char     * const htmlDirTreeCheckLink;
};

} // End of namespace

#endif
