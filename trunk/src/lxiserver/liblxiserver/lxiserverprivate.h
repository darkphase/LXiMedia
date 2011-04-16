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

#ifndef LXISERVERPRIVATE_H
#define LXISERVERPRIVATE_H

#include <QtCore>
#include <QtNetwork>
#include "shttpengine.h"
#include "shttpclient.h"
#include "ssandboxclient.h"

using namespace LXiServer;

class HttpClientRequest : public QObject
{
Q_OBJECT
public:
  explicit                      HttpClientRequest(SHttpClientEngine *);
  virtual                       ~HttpClientRequest();

public slots:
  void                          start(QAbstractSocket *);

signals:
  void                          response(const SHttpEngine::ResponseMessage &);

private slots:
  void                          readyRead();
  void                          close();

private:
  const SHttpEngine::RequestMessage message;
  QAbstractSocket             * socket;
  QByteArray                    data;
  QTimer                        closeTimer;
  bool                          responded;
};

class HttpServerRequest : public QObject
{
Q_OBJECT
public:
  explicit                      HttpServerRequest(SHttpServerEngine *);
  virtual                       ~HttpServerRequest();

public slots:
  void                          start(QAbstractSocket *);

signals:
  void                          handleHttpRequest(const SHttpEngine::RequestHeader &, QAbstractSocket *);

private slots:
  void                          readyRead();

private:
  const QPointer<SHttpServerEngine> parent;
  QAbstractSocket             * socket;
  QByteArray                    data;
  QTimer                        closeTimer;
};

class HttpSocketRequest : public QObject
{
Q_OBJECT
public:
  explicit                      HttpSocketRequest(QObject *, QAbstractSocket *, const QHostAddress &host, quint16 port, const QByteArray &message = QByteArray());
  explicit                      HttpSocketRequest(QObject *, QAbstractSocket *, const QString &host, quint16 port, const QByteArray &message = QByteArray());
  virtual                       ~HttpSocketRequest();

signals:
  void                          connected(QAbstractSocket *);

private slots:
  void                          connectToHost(const QHostInfo &);
  void                          connected(void);
  void                          bytesWritten(void);
  void                          failed(void);

private:
  static const int              maxTTL = 15000;
  const quint16                 port;
  QByteArray                    message;
  QAbstractSocket             * socket;
  QTimer                        failTimer;
};

class SandboxProcess : public QObject
{
Q_OBJECT
public:
  explicit                      SandboxProcess(SSandboxClient *, const QString &);
  virtual                       ~SandboxProcess();

public slots:
  void                          kill(void);

signals:
  void                          ready(const QHostAddress &, quint16);
  void                          stop(void);
  void                          finished(QProcess::ExitStatus);
  void                          consoleLine(const QString &);

private slots:
  void                          finished(int, QProcess::ExitStatus);
  void                          readyRead();

private:
  const QPointer<SSandboxClient> parent;
  QProcess              * const process;
};

#endif
