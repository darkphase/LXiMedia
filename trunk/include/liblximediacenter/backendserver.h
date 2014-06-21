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

#ifndef LXMEDIACENTER_BACKENDSERVER_H
#define LXMEDIACENTER_BACKENDSERVER_H

#include <QtCore>
#include <LXiCore>
#include "export.h"
#include "rootdevice.h"
#include "contentdirectory.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC BackendServer : public QObject
{
Q_OBJECT
S_FACTORIZABLE(BackendServer)
public:
  class LXIMEDIACENTER_PUBLIC MasterServer
  {
  public:
    virtual HttpStatus          parseHtmlContent(const QUrl &, const QByteArray &content, const QByteArray &head, QByteArray &contentType, QIODevice *&response) const = 0;

    virtual RootDevice        * rootDevice(void) = 0;
    virtual ContentDirectory  * contentDirectory(void) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC SearchResult
  {
                                SearchResult(void);
                                ~SearchResult();

    qreal                       relevance;
    QString                     headline;
    QString                     location;
    QString                     thumbLocation;
  };

  typedef QList<SearchResult>   SearchResultList;

public:
  /*! Creates all registred BackendServers.
      \param parent   The parent object, or NULL if none.
   */
  static QList<BackendServer *> create(QObject *parent);

public:
  explicit                      BackendServer(QObject * = NULL);
  virtual                       ~BackendServer();

  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  virtual QString               serverName(void) const = 0;
  virtual QString               serverIconPath(void) const = 0;
  QString                       serverPath(void) const;

  virtual QByteArray            frontPageContent(void);
  virtual QByteArray            settingsContent(void);

public:
  HttpStatus                    makeResponse(const QByteArray &, QByteArray &contentType, QIODevice *&response) const;
  HttpStatus                    makeResponse(const QString &, QByteArray &contentType, QIODevice *&response) const;
  HttpStatus                    makeHtmlContent(const QUrl &, const QByteArray &content, QByteArray &contentType, QIODevice *&response, const QByteArray &head = QByteArray()) const;

protected:
  MasterServer                * masterServer(void);

public:
  static const int              maxRequestTime;
  static const qreal            minSearchRelevance;
  static const char             searchDateFormat[];
  static const char             searchTimeFormat[];
  static const char             searchDateTimeFormat[];

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
