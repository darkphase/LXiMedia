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

#ifndef LXISERVER_SHTTPCLIENT_H
#define LXISERVER_SHTTPCLIENT_H

#include <QtCore>
#include <LXiCore>
#include "shttpengine.h"
#include "export.h"

namespace LXiServer {

/*! This class provides a basic HTTP client.
 */
class LXISERVER_PUBLIC SHttpClient : public SHttpClientEngine
{
Q_OBJECT
public:
                                SHttpClient(QObject * = NULL);
  virtual                       ~SHttpClient();

public: // From HttpClientEngine
  virtual void                  openRequest(const RequestMessage &header, QObject *receiver, const char *slot, Qt::ConnectionType = Qt::AutoConnection);
  virtual int                   maxSocketCount(void) const;

public slots: // From HttpClientEngine
  virtual void                  closeSocket(QIODevice *);
  virtual void                  reuseSocket(QIODevice *);

protected:
  virtual QIODevice           * openSocket(const QString &host);

private slots:
  void                          openRequest(void);
  void                          clearSocketPool(void);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
