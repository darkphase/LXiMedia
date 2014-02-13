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

#include "rootdevice.h"

namespace LXiMediaCenter {

const char  RootDevice::serviceTypeConnectionManager[]      = "urn:schemas-upnp-org:service:ConnectionManager:1";
const char  RootDevice::serviceTypeContentDirectory[]       = "urn:schemas-upnp-org:service:ContentDirectory:1";
const char  RootDevice::serviceTypeMediaReceiverRegistrar[] = "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1";

const char  RootDevice::mimeAppOctet[]          = "application/octet-stream";
const char  RootDevice::mimeAudioAac[]          = "audio/aac";
const char  RootDevice::mimeAudioAc3[]          = "audio/x-ac3";
const char  RootDevice::mimeAudioLpcm[]         = "audio/L16;rate=48000;channels=2";
const char  RootDevice::mimeAudioMp3[]          = "audio/mp3";
const char  RootDevice::mimeAudioMpeg[]         = "audio/mpeg";
const char  RootDevice::mimeAudioMpegUrl[]      = "audio/x-mpegurl";
const char  RootDevice::mimeAudioOgg[]          = "audio/ogg";
const char  RootDevice::mimeAudioWave[]         = "audio/wave";
const char  RootDevice::mimeAudioWma[]          = "audio/x-ms-wma";
const char  RootDevice::mimeImageJpeg[]         = "image/jpeg";
const char  RootDevice::mimeImagePng[]          = "image/png";
const char  RootDevice::mimeImageSvg[]          = "image/svg+xml";
const char  RootDevice::mimeImageTiff[]         = "image/tiff";
const char  RootDevice::mimeVideo3g2[]          = "video/3gpp";
const char  RootDevice::mimeVideoAsf[]          = "video/x-ms-asf";
const char  RootDevice::mimeVideoAvi[]          = "video/avi";
const char  RootDevice::mimeVideoFlv[]          = "video/x-flv";
const char  RootDevice::mimeVideoMatroska[]     = "video/x-matroska";
const char  RootDevice::mimeVideoMpeg[]         = "video/mpeg";
const char  RootDevice::mimeVideoMpegM2TS[]     = "video/vnd.dlna.mpeg-tts";
const char  RootDevice::mimeVideoMpegTS[]       = "video/x-mpegts";
const char  RootDevice::mimeVideoMp4[]          = "video/mp4";
const char  RootDevice::mimeVideoOgg[]          = "video/ogg";
const char  RootDevice::mimeVideoQt[]           = "video/quicktime";
const char  RootDevice::mimeVideoWmv[]          = "video/x-ms-wmv";
const char  RootDevice::mimeTextCss[]           = "text/css;charset=\"utf-8\"";
const char  RootDevice::mimeTextHtml[]          = "text/html;charset=\"utf-8\"";
const char  RootDevice::mimeTextJs[]            = "text/javascript;charset=\"utf-8\"";
const char  RootDevice::mimeTextPlain[]         = "text/plain;charset=\"utf-8\"";
const char  RootDevice::mimeTextXml[]           = "text/xml;charset=\"utf-8\"";

struct RootDevice::Data
{
  QUuid uuid;
  QByteArray deviceType;
  QString deviceName;
  QMap<QByteArray, Service *> services;
  QStringList icons;
  QMap<QString, HttpCallback *> httpCallbacks;
};

RootDevice::RootDevice(const QUuid &uuid, const QByteArray &deviceType, QObject *parent)
  : QObject(parent),
    d(new Data())
{
  d->uuid = uuid;
  d->deviceType = deviceType;
}

RootDevice::~RootDevice()
{
  d->httpCallbacks.clear();

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

void RootDevice::registerService(const QByteArray &serviceId, Service *service)
{
  d->services.insert(serviceId, service);
}

void RootDevice::unregisterService(const QByteArray &serviceId)
{
  d->services.remove(serviceId);
}

void RootDevice::registerHttpCallback(const QString &path, HttpCallback *callback)
{
  QString p = path;
  if (!p.endsWith('/'))
    p += '/';

  d->httpCallbacks.insert(p, callback);
}

void RootDevice::unregisterHttpCallback(HttpCallback *callback)
{
  for (QMap<QString, HttpCallback *>::Iterator i = d->httpCallbacks.begin();
       i != d->httpCallbacks.end();)
  {
    if (*i == callback)
      i = d->httpCallbacks.erase(i);
    else
      i++;
  }
}

void RootDevice::initialize(quint16, const QString &deviceName, bool)
{
  d->deviceName = deviceName;

  foreach (Service *service, d->services)
    service->initialize();
}

void RootDevice::close(void)
{
  foreach (Service *service, d->services)
    service->close();
}

//void RootDevice::bind(const QHostAddress &)
//{
//}

//void RootDevice::release(const QHostAddress &)
//{
//}

void RootDevice::addIcon(const QString &path)
{
  d->icons += path;
}

void RootDevice::emitEvent(const QByteArray &)
{
}

HttpStatus RootDevice::handleHttpRequest(const QUrl &url, const HttpCallback::RequestInfo &requestInfo, QByteArray &contentType, QIODevice *&response)
{
  QString path = url.path().left(url.path().lastIndexOf('/') + 1);
  while (!path.isEmpty())
  {
    QMap<QString, HttpCallback *>::Iterator i = d->httpCallbacks.find(path);
    if (i != d->httpCallbacks.end())
      return (*i)->httpRequest(url, requestInfo, contentType, response);

    path = path.left(path.lastIndexOf('/', -2) + 1);
  }

  return HttpStatus_NotFound;
}

void RootDevice::handleEvent(const QByteArray &serviceId, EventablePropertySet &propset)
{
  QMap<QByteArray, Service *>::Iterator i = d->services.find(serviceId);
  if (i != d->services.end())
    (*i)->writeEventableStateVariables(propset);
}

QByteArray RootDevice::udn() const
{
  return QString("uuid:" + d->uuid.toString()).replace("{", "").replace("}", "").toUtf8();
}

void RootDevice::writeDeviceDescription(DeviceDescription &desc)
{
  desc.setDeviceType(d->deviceType, "DMS-1.50");
  desc.setFriendlyName(d->deviceName);
  desc.setManufacturer(qApp->organizationName(), "http://" + qApp->organizationDomain() + "/");
  desc.setModel(qApp->applicationName(), qApp->applicationName(), "http://" + qApp->organizationDomain() + "/", qApp->applicationVersion());
  desc.setSerialNumber(d->uuid.toString().replace("{", "").replace("}", "").toUtf8());
  desc.setUDN(udn());

  foreach (const QString &path, d->icons)
  {
    const QImage icon(':' + path);
    if (!icon.isNull())
      desc.addIcon(path, toMimeType(path), icon.width(), icon.height(), icon.depth());
  }

  desc.setPresentationURL("/");
}

/*! Returns the MIME type for the specified filename, based on the extension.
 */
const char * RootDevice::toMimeType(const QString &fileName)
{
  const QString ext = QFileInfo(fileName).suffix().toLower();

  if      (ext == "js")     return mimeTextJs;
  else if (ext == "pdf")    return "application/pdf";
  else if (ext == "xhtml")  return "application/xhtml+xml";
  else if (ext == "dtd")    return "application/xml-dtd";
  else if (ext == "zip")    return "application/zip";
  else if (ext == "aac")    return mimeAudioAac;
  else if (ext == "ac3")    return mimeAudioAc3;
  else if (ext == "lpcm")   return mimeAudioLpcm;
  else if (ext == "m3u")    return mimeAudioMpegUrl;
  else if (ext == "mpa")    return mimeAudioMpeg;
  else if (ext == "mp2")    return mimeAudioMpeg;
  else if (ext == "mp3")    return mimeAudioMp3;
  else if (ext == "ac3")    return mimeAudioMpeg;
  else if (ext == "dts")    return mimeAudioMpeg;
  else if (ext == "oga")    return mimeAudioOgg;
  else if (ext == "wav")    return mimeAudioWave;
  else if (ext == "wma")    return mimeAudioWma;
  else if (ext == "jpeg")   return mimeImageJpeg;
  else if (ext == "jpg")    return mimeImageJpeg;
  else if (ext == "png")    return mimeImagePng;
  else if (ext == "svg")    return mimeImageSvg;
  else if (ext == "tiff")   return mimeImageTiff;
  else if (ext == "css")    return mimeTextCss;
  else if (ext == "html")   return mimeTextHtml;
  else if (ext == "htm")    return mimeTextHtml;
  else if (ext == "txt")    return mimeTextPlain;
  else if (ext == "log")    return mimeTextPlain;
  else if (ext == "xml")    return mimeTextXml;
  else if (ext == "3g2")    return mimeVideo3g2;
  else if (ext == "asf")    return mimeVideoAsf;
  else if (ext == "avi")    return mimeVideoAvi;
  else if (ext == "m2ts")   return mimeVideoMpegTS;
  else if (ext == "mkv")    return mimeVideoMatroska;
  else if (ext == "mpeg")   return mimeVideoMpeg;
  else if (ext == "mpg")    return mimeVideoMpeg;
  else if (ext == "mp4")    return mimeVideoMp4;
  else if (ext == "ts")     return mimeVideoMpeg;
  else if (ext == "ogg")    return mimeVideoOgg;
  else if (ext == "ogv")    return mimeVideoOgg;
  else if (ext == "ogx")    return mimeVideoOgg;
  else if (ext == "spx")    return mimeVideoOgg;
  else if (ext == "qt")     return mimeVideoQt;
  else if (ext == "flv")    return mimeVideoFlv;
  else if (ext == "wmv")    return mimeVideoWmv;

  // For licenses
  else if (fileName.startsWith("COPYING")) return "text/plain";

  else                      return "application/octet-stream";
}

} // End of namespace
