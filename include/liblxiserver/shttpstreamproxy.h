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

#ifndef LXMEDIACENTER_HTTPSTREAMPROXY_H
#define LXMEDIACENTER_HTTPSTREAMPROXY_H

#include <QtCore>
#include <QtNetwork>
#include <LXiCore>
#include "export.h"

namespace LXiServer {

class SHttpEngine;

/*! This is a HTTP stream proxy that can be used to stream to one or more
    clients simultaneously.
 */
class LXISERVER_PUBLIC SHttpStreamProxy : public QThread
{
Q_OBJECT
public:
  explicit                      SHttpStreamProxy(void);
  virtual                       ~SHttpStreamProxy();

  bool                          isConnected(void) const;

public slots:
  bool                          setSource(QIODevice *, SHttpEngine *);
  bool                          addSocket(QIODevice *, SHttpEngine *);

protected:
  virtual void                  run(void);
  virtual void                  customEvent(QEvent *);

signals:
  void                          disconnected(void);

private:
  void                          disconnectAllSockets(void);

private slots:
  void                          processData(void);
  void                          flushData(void);

private:
  static const QEvent::Type     setSourceEventType;
  static const QEvent::Type     addSocketEventType;
  static const QEvent::Type     disconnectEventType;

  static const int              inBufferSize;
  static const int              outBufferSize;

  class SetSourceEvent;
  class AddSocketEvent;

  struct Data;
  Data                  * const d;
};


} // End of namespace

#endif
