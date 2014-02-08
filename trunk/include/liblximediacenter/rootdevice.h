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

namespace LXiMediaCenter {

enum HttpStatus
{
  HttpStatus_Ok                   = 200,
  HttpStatus_NotFound             = 404,
  HttpStatus_InternalServerError  = 500
};

class LXIMEDIACENTER_PUBLIC RootDevice : public QObject
{
Q_OBJECT
public:
  struct LXIMEDIACENTER_PUBLIC HttpCallback
  {
    virtual HttpStatus          httpRequest(const QUrl &request, QByteArray &contentType, QIODevice *&response) = 0;
  };

  struct LXIMEDIACENTER_PUBLIC DeviceDescription
  {
    virtual void                setDeviceType(const QByteArray &, const QByteArray &dlnaDoc) = 0;
    virtual void                setFriendlyName(const QString &) = 0;
    virtual void                setManufacturer(const QString &manufacturer, const QString &url) = 0;
    virtual void                setModel(const QString &description, const QString &name, const QString &url, const QString &number) = 0;
    virtual void                setSerialNumber(const QByteArray &) = 0;
    virtual void                setUDN(const QByteArray &) = 0;
    virtual void                addIcon(const QString &url, const char *mimetype, int width, int height, int depth) = 0;
    virtual void                setPresentationURL(const QString &) = 0;
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
  explicit                      RootDevice(const QUuid &uuid, const QByteArray &deviceType, QObject *parent);
  virtual                       ~RootDevice();

  virtual void                  registerService(const QByteArray &serviceId, Service *);
  virtual void                  unregisterService(const QByteArray &serviceId);

  virtual void                  registerHttpCallback(const QString &path, HttpCallback *);
  virtual void                  unregisterHttpCallback(HttpCallback *);

  virtual void                  initialize(quint16 port, const QString &deviceName);
  virtual void                  close(void);

//  virtual void                  bind(const QHostAddress &);
//  virtual void                  release(const QHostAddress &);

  virtual void                  addIcon(const QString &path);
  virtual void                  emitEvent(const QByteArray &serviceId);

  HttpStatus                    handleHttpRequest(const QUrl &path, QByteArray &contentType, QIODevice *&response);
  void                          handleEvent(const QByteArray &serviceId, EventablePropertySet &);

  QByteArray                    udn() const;

protected:
  void                          writeDeviceDescription(DeviceDescription &);

public:
  static const char           * toMimeType(const QString &fileName);
  static const char             mimeAppOctet[];
  static const char             mimeAudioAac[];
  static const char             mimeAudioAc3[];
  static const char             mimeAudioLpcm[];
  static const char             mimeAudioMp3[];
  static const char             mimeAudioMpeg[];
  static const char             mimeAudioMpegUrl[];
  static const char             mimeAudioOgg[];
  static const char             mimeAudioWave[];
  static const char             mimeAudioWma[];
  static const char             mimeImageJpeg[];
  static const char             mimeImagePng[];
  static const char             mimeImageSvg[];
  static const char             mimeImageTiff[];
  static const char             mimeVideo3g2[];
  static const char             mimeVideoAsf[];
  static const char             mimeVideoAvi[];
  static const char             mimeVideoFlv[];
  static const char             mimeVideoMatroska[];
  static const char             mimeVideoMpeg[];
  static const char             mimeVideoMpegM2TS[];
  static const char             mimeVideoMpegTS[];
  static const char             mimeVideoMp4[];
  static const char             mimeVideoOgg[];
  static const char             mimeVideoQt[];
  static const char             mimeVideoWmv[];
  static const char             mimeTextCss[];
  static const char             mimeTextHtml[];
  static const char             mimeTextJs[];
  static const char             mimeTextPlain[];
  static const char             mimeTextXml[];

private:
  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
