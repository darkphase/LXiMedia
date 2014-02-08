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

#ifndef LXIMEDIACENTER_MEDIARECEIVERREGISTRAR_H
#define LXIMEDIACENTER_MEDIARECEIVERREGISTRAR_H

#include <QtCore>
#include "export.h"
#include "rootdevice.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC MediaReceiverRegistrar : public QObject,
                                                     public RootDevice::Service
{
Q_OBJECT
public:
  struct LXIMEDIACENTER_PUBLIC ActionIsAuthorized
  {
    virtual QByteArray          getDeviceID() const = 0;
    virtual void                setResponse(int) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC ActionIsValidated
  {
    virtual QByteArray          getDeviceID() const = 0;
    virtual void                setResponse(int) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC ActionRegisterDevice
  {
    virtual QByteArray          getRegistrationReqMsg() const = 0;
    virtual void                setResponse(const QByteArray &) = 0;
  };

public:
  explicit                      MediaReceiverRegistrar(RootDevice *parent);
  virtual                       ~MediaReceiverRegistrar();

  void                          handleAction(const QByteArray &, ActionIsAuthorized &);
  void                          handleAction(const QByteArray &, ActionIsValidated &);
  void                          handleAction(const QByteArray &, ActionRegisterDevice &);

protected: // From RootDevice::Service
  virtual const char          * serviceType(void);

  virtual void                  initialize(void);
  virtual void                  close(void);

  virtual void                  writeServiceDescription(RootDevice::ServiceDescription &) const;
  virtual void                  writeEventableStateVariables(RootDevice::EventablePropertySet &) const;

protected:
  virtual void                  customEvent(QEvent *);

protected:
  static const char             serviceId[];

private:
  RootDevice            * const parent;
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
