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

#ifndef SCREENGRABBER_H
#define SCREENGRABBER_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <LXiServer>
#include <LXiMediaCenter>
#include <LXiStreamDevice>

class ScreenGrabber : public QObject,
                      public SSandboxServer::Callback
{
Q_OBJECT
public:
                                ScreenGrabber();
  virtual                       ~ScreenGrabber();

  void                          show();

public: // From SSandboxServer::Callback
  virtual SSandboxServer::ResponseMessage httpRequest(const SSandboxServer::RequestMessage &, QIODevice *);

private slots:
  void                          cleanStreams(void);

private:
  static QUuid                  serverUuid(void);

private:
  static const quint16          defaultPort = 4281;
  const QIcon                   screenIcon;
  const QIcon                   eyesIcon;

  QMenu                         menu;
  QSystemTrayIcon               trayIcon;

  SHttpServer                   httpServer;
  SSsdpServer                   ssdpServer;
  QList<SGraph *>               streams;
  QTimer                        cleanStreamsTimer;
};

class DesktopStream : public MediaStream
{
Q_OBJECT
public:
  explicit                      DesktopStream(const QString &device);
  virtual                       ~DesktopStream();

  bool                          setup(const SHttpServer::RequestMessage &, QIODevice *);

public:
  SAudioVideoInputNode          input;
};

#endif
