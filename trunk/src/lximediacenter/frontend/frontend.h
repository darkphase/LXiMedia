/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
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

#ifndef FRONTEND_H
#define FRONTEND_H

#include <QtCore>
#include <QtGui>
#include <QtWebKit>
#include <QtWebKitWidgets>
#include <LXiMediaCenter>

class Frontend : public QWebView
{
Q_OBJECT
private:
  class WebPage : public QWebPage
  {
  public:
    explicit                    WebPage(Frontend *parent);

  protected:
    virtual bool                acceptNavigationRequest(QWebFrame *, const QNetworkRequest &, NavigationType);

  private:
    Frontend            * const parent;
  };

  class NetworkAccessManager : public QNetworkAccessManager
  {
  public:
    explicit                    NetworkAccessManager(Frontend *parent);

  protected:
    virtual QNetworkReply     * createRequest(Operation, const QNetworkRequest &, QIODevice *);
  };

public:
                                Frontend();
  virtual                       ~Frontend();

  void                          show(void);

protected:
  virtual void                  contextMenuEvent(QContextMenuEvent *);
  virtual void                  keyPressEvent(QKeyEvent *);

private slots:
  void                          deviceDiscovered(const QByteArray &, const QByteArray &location);
  void                          deviceClosed(const QByteArray &deviceId);
  void                          loadFrontendPage(const QUrl &);
  void                          updateFrontendPage(void);
  void                          titleChanged(const QString &);

private:
#if defined(Q_OS_MACX)
  static void                   registerAgent(void);
#endif

private:
  static const char             daemonName[];

  UPnP                          upnp;
  Client                        upnpClient;

  QMap<QByteArray, Client::DeviceDescription> devices;

  QTimer                        frontendPageTimer;
  bool                          frontendPageShowing;
  bool                          waitingForWelcome;

#if defined(Q_OS_MACX)
  QTime                         startingTimer;
#endif

private:
  QByteArray                    makeWaitingPage(void) const;
  QByteArray                    makeFrontendPage(void) const;
  QByteArray                    makeIFrame(const QByteArray &) const;

private:
  static const char             htmlIndex[];
  static const char             htmlWaiting[];
  static const char             htmlNavigator[];
  static const char             htmlServers[];
  static const char             htmlLocalServer[];
  static const char             htmlNoLocalServer[];
  static const char             htmlStartLocalServer[];
  static const char             htmlStopLocalServer[];
  static const char             htmlDisableLocalServer[];
  static const char             htmlConfigureLocalServer[];
  static const char             htmlServer[];
  static const char             htmlNoServers[];
  static const char             htmlIFrame[];
};

#endif
