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

#ifndef LXIMEDIACENTER_CLIENT_H
#define LXIMEDIACENTER_CLIENT_H

#include <QtCore>
#include "export.h"
#include "upnp.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC Client : public QObject,
                                     public UPnP::Child
{
Q_OBJECT
public:
  struct DeviceDescription
  {
    QByteArray                  deviceType;
    QString                     friendlyName;
    QString                     manufacturer;
    QString                     modelName;
    QByteArray                  udn;
    QUrl                        iconURL;
    QUrl                        presentationURL;
  };

public:
  explicit                      Client(UPnP *);
  virtual                       ~Client();

  UPnP                        * upnp();

  virtual bool                  initialize(void);
  virtual void                  close(void);

  void                          startSearch(const QByteArray &target, int mx = 3);

  bool                          getDeviceDescription(const QByteArray &location, DeviceDescription &description);

signals:
  void                          deviceDiscovered(const QByteArray &deviceId, const QByteArray &location);
  void                          deviceClosed(const QByteArray &deviceId);

private:
  bool                          enableClient(void);

private:
  struct Data;
  Data                 * const d;
};

} // End of namespace

#endif
