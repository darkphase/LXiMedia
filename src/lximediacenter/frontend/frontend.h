/***************************************************************************
 *   Copyright (C) 2011 by A.J. Admiraal                                   *
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

#ifndef FRONTEND_H
#define FRONTEND_H

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include <QtWebKit>
#include <QtXml>
#include <LXiServer>

class Frontend : public QWebView
{
Q_OBJECT
private:
  struct Server : SSsdpClient::Node
  {
    inline                      Server(const SSsdpClient::Node &node) : Node(node) { }
    inline                      Server(const Server &server) : Node(server) { }

    QString                     friendlyName;
    QString                     modelName;
    QUrl                        presentationURL;
    QUrl                        iconURL;
  };

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
  virtual void                  keyReleaseEvent(QKeyEvent *);

private slots:
  void                          loadFrontendPage(const QUrl &);
  void                          updateServers(void);
  void                          requestFinished(QNetworkReply *);
  void                          updateFrontendPage(void);
  void                          titleChanged(const QString &);

private:
  static bool                   isBackendInstalled(void);
  static bool                   isBackendRunning(void);
  static bool                   isLocalAddress(const QString &);

private:
  static const char             backendExecutable[];

  SSsdpClient                   ssdpClient;
  QNetworkAccessManager         networkAccessManager;

  QMap<QString, Server>         servers;
  QTimer                        frontendPageTimer;
  bool                          frontendPageShowing;

private:
  QByteArray                    makeFrontendPage(void) const;
  QByteArray                    makeIFrame(const QByteArray &) const;

private:
  static const char             htmlIndex[];
  static const char             htmlNavigator[];
  static const char             htmlServers[];
  static const char             htmlLocalServer[];
  static const char             htmlNoLocalServer[];
  static const char             htmlStartLocalServer[];
  static const char             htmlStopLocalServer[];
  static const char             htmlConfigureLocalServer[];
  static const char             htmlServer[];
  static const char             htmlNoServers[];
  static const char             htmlIFrame[];
};

#endif
