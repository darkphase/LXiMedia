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

  static ResponseMessage        blockedRequest(const RequestMessage &, int timeout = 30000);

public: // From HttpClientEngine
  virtual void                  openRequest(const RequestMessage &header, QObject *receiver, const char *slot);

protected:
  virtual void                  socketDestroyed(void);
      
private slots:
  _lxi_internal void            openRequest(void);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
