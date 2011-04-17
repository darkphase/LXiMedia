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

#ifndef LXMEDIACENTER_HTTPSTREAMPROXY_H
#define LXMEDIACENTER_HTTPSTREAMPROXY_H

#include <QtCore>
#include <QtNetwork>
#include <LXiCore>
#include "export.h"

namespace LXiServer {

/*! This is a generic output node, writing to a QIODevice.
 */
class LXISERVER_PUBLIC SHttpStreamProxy : public QThread
{
Q_OBJECT
public:
  explicit                      SHttpStreamProxy(void);
  virtual                       ~SHttpStreamProxy();

  bool                          isConnected(void) const;

public slots:
  bool                          setSource(QAbstractSocket *);
  bool                          addSocket(QAbstractSocket *);

signals:
  void                          disconnected(void);

protected:
  virtual void                  run(void);

private:
  _lxi_internal void            disconnectAllSockets(void);

private slots:
  _lxi_internal void            processData(void);
  _lxi_internal void            flushData(void);

private:
  _lxi_internal static const int outBufferSize;

  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
