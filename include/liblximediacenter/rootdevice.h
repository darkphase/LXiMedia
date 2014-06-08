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

#ifndef LXIMEDIACENTER_ROOTDEVICE_H
#define LXIMEDIACENTER_ROOTDEVICE_H

#include <QtCore>
#include "export.h"
#include "upnp.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC RootDevice : public QObject,
                                         private UPnP::HttpCallback
{
Q_OBJECT
public:
  struct LXIMEDIACENTER_PUBLIC DeviceDescription
  {
    virtual void                setDeviceType(const QByteArray &, const QByteArray &dlnaDoc) = 0;
    virtual void                setFriendlyName(const QString &) = 0;
    virtual void                setManufacturer(const QString &manufacturer, const QString &url) = 0;
    virtual void                setModel(const QString &description, const QString &name, const QString &url, const QString &number) = 0;
    virtual void                setSerialNumber(const QByteArray &) = 0;
    virtual void                setUDN(const QByteArray &) = 0;
    virtual void                addIcon(const QString &url, const char *mimetype, int width, int height, int depth) = 0;
    virtual void                setPresentationURL(const QByteArray &) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC ServiceDescription
  {
    virtual void                addAction(const char *name, const char * const *argname, const char * const *argdir, const char * const *argvar, int argcount) = 0;
    template <int _c> void      addAction(const char *name, const char * const(&argname)[_c], const char * const(&argdir)[_c], const char * const(&argvar)[_c]) { addAction(name, argname, argdir, argvar, _c); }
    virtual void                addStateVariable(const char *name, const char *type, bool sendEvents, const char * const *values, int valcount) = 0;
    template <int _c> void      addStateVariable(const char *name, const char *type, bool sendEvents, const char * const(&values)[_c]) { addStateVariable(name, type, sendEvents, values, _c); }
    void                        addStateVariable(const char *name, const char *type, bool sendEvents) { addStateVariable(name, type, sendEvents, NULL, 0); }
  };

  struct LXIMEDIACENTER_PUBLIC EventablePropertySet
  {
    virtual void                addProperty(const QString &name, const QString &value) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC Service
  {
    virtual const char        * serviceType(void) = 0;

    virtual void                initialize(void) = 0;
    virtual void                close(void) = 0;

    virtual void                writeServiceDescription(ServiceDescription &) const = 0;
    virtual void                writeEventableStateVariables(EventablePropertySet &) const = 0;
  };

  static const char             serviceTypeConnectionManager[];
  static const char             serviceTypeContentDirectory[];
  static const char             serviceTypeMediaReceiverRegistrar[];

public:
                                RootDevice(UPnP *, const QUuid &, const QByteArray &deviceType);
  virtual                       ~RootDevice();

  UPnP                        * upnp();

  void                          setDeviceName(const QString &deviceName);
  void                          addIcon(const QString &path);

  void                          registerService(const QByteArray &serviceId, Service *);
  void                          unregisterService(const QByteArray &serviceId);

  virtual bool                  initialize();
  virtual void                  close(void);

  void                          emitEvent(const QByteArray &serviceId);

  QByteArray                    udn() const;

protected:
  void                          handleEvent(const QByteArray &serviceId, EventablePropertySet &);

private: // From UPnP::HttpCallback
  virtual HttpStatus            httpRequest(const QUrl &request, const UPnP::HttpRequestInfo &, QByteArray &contentType, QIODevice *&response);

private slots:
  void                          sendAdvertisements();

private:
  void                          writeDeviceDescription(DeviceDescription &);

  bool                          enableRootDevice(void);

private:
  struct Data;
  Data                 * const d;
};

} // End of namespace

#endif
