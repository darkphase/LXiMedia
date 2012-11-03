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

#ifndef SCREENGRABBERSERVER_H
#define SCREENGRABBERSERVER_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStream>
#include <LXiStreamDevice>

namespace LXiMediaCenter {
namespace ScreenGrabberBackend {

class ScreenGrabberServer : public MediaServer
{
Q_OBJECT
protected:
  class Stream : public MediaServer::Stream
  {
  public:
                                Stream(ScreenGrabberServer *, SSandboxClient *, const QString &url);
    virtual                     ~Stream();

    bool                        setup(const QUrl &request);

  public:
    SSandboxClient      * const sandbox;
  };

public:
                                ScreenGrabberServer(const QString &, QObject *);

  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  virtual QString               serverName(void) const;
  virtual QString               serverIconPath(void) const;

  virtual QByteArray            frontPageContent(void);

protected: // From MediaServer
  virtual Stream              * streamVideo(const SHttpServer::RequestMessage &);
  virtual SHttpServer::ResponseMessage sendPhoto(const SHttpServer::RequestMessage &);

  virtual QList<Item>           listItems(const QString &path, int start, int &count);
  virtual Item                  getItem(const QString &path);

private:
  SSandboxClient              * sandbox;

private:
  static const char             htmlFrontPageContent[];
};

} } // End of namespaces

#endif
