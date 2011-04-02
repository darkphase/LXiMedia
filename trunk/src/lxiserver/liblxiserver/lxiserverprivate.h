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
#include "ssandboxclient.h"

using namespace LXiServer;

class HttpClientRequest : public QObject
{
Q_OBJECT
public:
  explicit                      HttpClientRequest(SHttpClientEngine *, const SHttpEngine::RequestMessage &);
  virtual                       ~HttpClientRequest();

public slots:
  void                          start(QIODevice *);

signals:
  void                          response(const SHttpEngine::ResponseMessage &);

private slots:
  void                          readyRead();
  void                          close();

private:
  SHttpClientEngine     * const parent;
  const SHttpEngine::RequestMessage message;
  QIODevice                   * socket;
  QByteArray                    data;
  QTimer                        closeTimer;
  bool                          responded;
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
  void                          ready(void);
  void                          stop(void);
  void                          finished(void);
  void                          consoleLine(const QString &);

private slots:
  void                          finished(int, QProcess::ExitStatus);
  void                          readyRead();

private:
  SSandboxClient        * const parent;
  QProcess              * const process;
};

class SandboxSocketRequest : public QObject
{
Q_OBJECT
public:
  explicit                      SandboxSocketRequest(SSandboxClient *);
  explicit                      SandboxSocketRequest(QIODevice *);
  virtual                       ~SandboxSocketRequest();

signals:
  void                          connected(QIODevice *);

private slots:
  void                          connected(void);
  void                          failed(void);
  void                          bytesWritten(void);

private:
  static const int              maxTTL = 15000;
  SSandboxClient        * const parent;
  QLocalSocket                * socket;
  QTimer                        deleteTimer;
};

class SandboxMessageRequest : public QObject
{
Q_OBJECT
public:
                                SandboxMessageRequest(const SHttpEngine::RequestMessage &);
  virtual                       ~SandboxMessageRequest();

public slots:
  void                          connected(QIODevice *);

signals:
  void                          headerSent(QIODevice *);

private:
  const QByteArray              message;
};

#endif
