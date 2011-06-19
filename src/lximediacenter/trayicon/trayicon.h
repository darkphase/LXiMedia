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

#ifndef __TRAYICON_H
#define __TRAYICON_H

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <LXiServer>

class TrayIcon
#if defined(Q_OS_MACX)
    : public QWidget
#else
    : public QObject
#endif
{
Q_OBJECT
private:
  struct Server
  {
    inline Server(void)
      : updateStatusReply(NULL), visible(true), firstUpdate(true),
        notifiedErrorLog(false)
    {
    }

    QUrl                        url;
    QString                     hostname;

    QNetworkReply             * updateStatusReply;

    bool                        visible;
    bool                        firstUpdate;
    bool                        notifiedErrorLog;
    QSet<QString>               detectedClients;
  };

public:
                                TrayIcon();
  virtual                       ~TrayIcon();

  void                          show(void);

private slots:
  void                          startSSDP(void);

  void                          showAbout(void);
  void                          updateMenu(void);
  void                          rebuildMenu(void);
  void                          updateStatus(void);
  void                          updateStatus(QMap<QString, Server>::Iterator &);

  void                          requestFinished(QNetworkReply *);

  void                          messageClicked(void);
  void                          loadBrowser(QAction *);
  void                          loadBrowser(const QUrl &);

#if defined(Q_OS_MACX)
  void                          startStopLocalAgent(void);
  void                          registerAgent(bool);
#endif

private:
  static bool                   isLocalAddress(const QString &);

private:
  static const int              iconSize;

#if defined(Q_OS_MACX)
  static const char             agentName[];

  QLabel                        serviceLabel;
  QPushButton                   serviceButton;
  QToolButton                   menuButton;
  QHBoxLayout           * const serviceLayout;
  QCheckBox                     startAutomatically;
  QLabel                        helpLabel;
  QVBoxLayout           * const layout;
#else
  QSystemTrayIcon               trayIcon;
#endif

  const QIcon                   icon;
  QMenu                         menu;
  SSsdpClient                   ssdpClient;
  QMap<QString, Server>         servers;
  bool                          localhostRunning;
  QUrl                          messageUrl;

  QTimer                        updateStatusTimer;

  QNetworkAccessManager         networkAccessManager;
  QMessageBox                 * aboutBox;
};

#endif
