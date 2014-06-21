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

#ifndef MEDIAPLAYERSERVER_H
#define MEDIAPLAYERSERVER_H

#include <QtCore>
#include <LXiStream>
#include <LXiMediaCenter>

class Setup : public MediaServer
{
Q_OBJECT
public:
  explicit                      Setup(bool allowShutdown, QObject *parent = NULL);

protected: // From BackendServer
  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  virtual QString               serverName(void) const;
  virtual QString               serverIconPath(void) const;

  virtual QByteArray            frontPageContent(void);
  virtual QByteArray            settingsContent(void);

protected: // From MediaServer
  virtual MediaStream         * streamVideo(const QUrl &request);
  virtual HttpStatus            sendPhoto(const QUrl &request, QByteArray &contentType, QIODevice *&response);

  virtual QList<Item>           listItems(const QString &virtualPath, int start, int &count);
  virtual Item                  getItem(const QString &path);
  virtual ListType              listType(const QString &path);

private:
  const bool                    allowShutdown;
  MasterServer                * masterServer;
};

#endif
