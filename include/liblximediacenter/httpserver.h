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

#ifndef LXMEDIACENTER_HTTPSERVER_H
#define LXMEDIACENTER_HTTPSERVER_H

#include <QtCore>
#include <LXiStream>
#include <liblximediacenter/fileserver.h>

namespace LXiMediaCenter {

class HttpServerFile;
class HttpServerDir;

class HttpServer : public QThread,
                   public FileServer
{
Q_OBJECT
friend class HttpServerDir;
private:
  class Interface;
  class SocketHandler;

public:
  explicit                      HttpServer(void);
  virtual                       ~HttpServer();

  void                          initialize(const QList<QHostAddress> &addresses, quint16 port = 0);
  void                          close(void);
  quint16                       serverPort(const QHostAddress &) const;

  bool                          addFile(const QString &path, const QString &realFile);

  static QString                toMimeType(const QString &fileName) __attribute__((pure));

protected:
  virtual void                  run(void);

private:
  struct Private;
  Private               * const p;
};

class HttpServerDir : public FileServerDir
{
Q_OBJECT
friend class HttpServer;
public:
  explicit                      HttpServerDir(HttpServer *);

  void                          addFile(const QString &name, const QString &realFile);

  inline HttpServer           * server(void)                                    { return static_cast<HttpServer *>(FileServerDir::server()); }
  inline const HttpServer     * server(void) const                              { return static_cast<const HttpServer *>(FileServerDir::server()); }

protected:
  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

private:
  QMap<QString, QString>        files;
};


} // End of namespace

#endif
