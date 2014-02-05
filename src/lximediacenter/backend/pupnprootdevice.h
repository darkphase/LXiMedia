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

#ifndef PUPNPSERVER_H
#define PUPNPSERVER_H

#include <QtCore>
#include <LXiMediaCenter>

class PupnpRootDevice : public RootDevice,
                        private RootDevice::HttpCallback
{
Q_OBJECT
public:
  struct FileInfo
  {
    size_t                      size;
    QDateTime                   modified;
    QString                     mimeType;
  };

public:
  explicit                      PupnpRootDevice(const QUuid &, const QString &deviceType, QObject *parent);
  virtual                       ~PupnpRootDevice();

  virtual void                  registerService(const QByteArray &serviceId, Service *);
  virtual void                  unregisterService(const QByteArray &serviceId);

  virtual void                  registerHttpCallback(const QString &path, RootDevice::HttpCallback *);
  virtual void                  unregisterHttpCallback(RootDevice::HttpCallback *);

  virtual void                  initialize(quint16 port, const QString &deviceName);
  virtual void                  close(void);

  virtual void                  emitEvent(const QByteArray &serviceId);

protected:
  virtual void                  customEvent(QEvent *);

private: // From RootDevice::HttpCallback
  virtual HttpStatus            httpRequest(const QUrl &request, QByteArray &contentType, QIODevice *&response);

private:
  struct Functor { virtual ~Functor() { } virtual void operator()() = 0; };
  void                          send(Functor &) const;

  void                          enableWebserver(void);
  HttpStatus                    getResponse(const QByteArray &path, QByteArray &contentType, QIODevice *&response, bool erase);

  void                          enableRootDevice(void);

private:
  struct Data;
  Data                 * const d;
};

#endif
