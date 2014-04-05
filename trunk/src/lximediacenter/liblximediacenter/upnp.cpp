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

#include <upnp/upnp.h>
#include "upnp.h"

namespace LXiMediaCenter {

struct UPnP::Data
{
  class Event : public QEvent
  {
  public:
    static const QEvent::Type myType;
    explicit Event(Functor &f, QSemaphore *sem = NULL) : QEvent(myType), f(f), sem(sem) { }
    virtual ~Event() { if (sem) sem->release(); else delete &f; }

    Functor &f;

  private:
    QSemaphore * const sem;
  };

  static UPnP * me;

  quint16 port;
  bool bindPublicInterfaces;

  QSet<QByteArray> availableAddresses;
  bool initialized, webServerEnabled;
  QMap<QString, HttpCallback *> httpCallbacks;

  QMutex responsesMutex;
  QMap<QByteArray, QPair<QIODevice *, QByteArray> > responses;
  static const int clearResponsesInterval = 15000;
  QTimer clearResponsesTimer;

  QSet<QObject *> pendingFiles;
  static const int updateInterfacesInterval = 10000;
  QTimer updateInterfacesTimer;
};

const char  UPnP::mimeAppOctet[]          = "application/octet-stream";
const char  UPnP::mimeAudioAac[]          = "audio/aac";
const char  UPnP::mimeAudioAc3[]          = "audio/x-ac3";
const char  UPnP::mimeAudioLpcm[]         = "audio/L16;rate=48000;channels=2";
const char  UPnP::mimeAudioMp3[]          = "audio/mp3";
const char  UPnP::mimeAudioMpeg[]         = "audio/mpeg";
const char  UPnP::mimeAudioMpegUrl[]      = "audio/x-mpegurl";
const char  UPnP::mimeAudioOgg[]          = "audio/ogg";
const char  UPnP::mimeAudioWave[]         = "audio/wave";
const char  UPnP::mimeAudioWma[]          = "audio/x-ms-wma";
const char  UPnP::mimeImageJpeg[]         = "image/jpeg";
const char  UPnP::mimeImagePng[]          = "image/png";
const char  UPnP::mimeImageSvg[]          = "image/svg+xml";
const char  UPnP::mimeImageTiff[]         = "image/tiff";
const char  UPnP::mimeVideo3g2[]          = "video/3gpp";
const char  UPnP::mimeVideoAsf[]          = "video/x-ms-asf";
const char  UPnP::mimeVideoAvi[]          = "video/avi";
const char  UPnP::mimeVideoFlv[]          = "video/x-flv";
const char  UPnP::mimeVideoMatroska[]     = "video/x-matroska";
const char  UPnP::mimeVideoMpeg[]         = "video/mpeg";
const char  UPnP::mimeVideoMpegM2TS[]     = "video/vnd.dlna.mpeg-tts";
const char  UPnP::mimeVideoMpegTS[]       = "video/x-mpegts";
const char  UPnP::mimeVideoMp4[]          = "video/mp4";
const char  UPnP::mimeVideoOgg[]          = "video/ogg";
const char  UPnP::mimeVideoQt[]           = "video/quicktime";
const char  UPnP::mimeVideoWmv[]          = "video/x-ms-wmv";
const char  UPnP::mimeTextCss[]           = "text/css;charset=\"utf-8\"";
const char  UPnP::mimeTextHtml[]          = "text/html;charset=\"utf-8\"";
const char  UPnP::mimeTextJs[]            = "text/javascript;charset=\"utf-8\"";
const char  UPnP::mimeTextPlain[]         = "text/plain;charset=\"utf-8\"";
const char  UPnP::mimeTextXml[]           = "text/xml;charset=\"utf-8\"";

UPnP   * UPnP::Data::me = NULL;
const QEvent::Type  UPnP::Data::Event::myType = QEvent::Type(QEvent::registerEventType());

UPnP::UPnP(QObject *parent)
  : QObject(parent),
    d(new Data())
{
  Q_ASSERT(d->me == NULL);

  d->me = this;
  d->port = 0;
  d->bindPublicInterfaces = false;
  d->initialized = false;
  d->webServerEnabled = false;

  connect(&d->clearResponsesTimer, SIGNAL(timeout()), SLOT(clearResponses()));
  d->clearResponsesTimer.setTimerType(Qt::VeryCoarseTimer);
  d->clearResponsesTimer.setSingleShot(true);

  connect(&d->updateInterfacesTimer, SIGNAL(timeout()), SLOT(updateInterfaces()));
  d->updateInterfacesTimer.setTimerType(Qt::VeryCoarseTimer);

  sApp->addLicense(
      " <h3>Portable SDK for UPnP Devices</h3>\n"
      " <p>Website: <a href=\"http://pupnp.sourceforge.net/\">pupnp.sourceforge.net</a></p>\n"
      " <p>Copyright &copy; 2000-2003 Intel Corporation. All rights reserved.</p>\n"
      " <p>Copyright &copy; 2011-2012 France Telecom. All rights reserved.</p>\n"
      " <p>Used under the terms of the BSD License.</p>\n");
}

UPnP::~UPnP()
{
  d->httpCallbacks.clear();

  UPnP::close();

  Q_ASSERT(d->me != NULL);
  d->me = NULL;

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

QString UPnP::hostname()
{
  char buffer[64] = { '\0' };

  // On Windows this will ensure Winsock is initialized properly.
  if (::UpnpGetAvailableIpAddresses() != NULL)
    gethostname(buffer, sizeof(buffer) - 1);

  return QString::fromUtf8(buffer);
}

void UPnP::registerHttpCallback(const QString &path, HttpCallback *callback)
{
  if (d->initialized && !d->webServerEnabled)
    enableWebserver();

  QString p = path;
  if (!p.endsWith('/'))
    p += '/';

  if (!d->httpCallbacks.contains(p))
  {
    d->httpCallbacks.insert(p, callback);
    if (d->initialized)
      ::UpnpAddVirtualDir(p.toUtf8());
  }
}

void UPnP::unregisterHttpCallback(HttpCallback *callback)
{
  for (QMap<QString, HttpCallback *>::Iterator i = d->httpCallbacks.begin();
       i != d->httpCallbacks.end();)
  {
    if (*i == callback)
    {
      if (d->initialized)
        ::UpnpRemoveVirtualDir(i.key().toUtf8());

      i = d->httpCallbacks.erase(i);
    }
    else
      i++;
  }
}

bool UPnP::initialize(quint16 port, bool bindPublicInterfaces)
{
  d->port = port;
  d->bindPublicInterfaces = bindPublicInterfaces;

  QVector<const char *> addresses;
  for (char **i = ::UpnpGetAvailableIpAddresses(); i && *i; i++)
  {
    d->availableAddresses.insert(*i);
    if (bindPublicInterfaces || isLocalAddress(*i))
      addresses.append(*i);
  }

  addresses.append(NULL);
  d->initialized = ::UpnpInit3(&(addresses[0]), port) == UPNP_E_SUCCESS;
  if (!d->initialized)
    d->initialized = ::UpnpInit3(&(addresses[0]), 0) == UPNP_E_SUCCESS;

  if (d->initialized)
  {
    for (char **i = ::UpnpGetServerIpAddresses(); i && *i; i++)
      qDebug() << "Bound " << *i << ":" << ::UpnpGetServerPort();

    if (!d->httpCallbacks.isEmpty())
      enableWebserver();

    for (QMap<QString, HttpCallback *>::ConstIterator i = d->httpCallbacks.begin();
         i != d->httpCallbacks.end();
         i++)
    {
      ::UpnpAddVirtualDir(i.key().toUtf8());
    }
  }

  d->updateInterfacesTimer.start(d->updateInterfacesInterval);

  return d->initialized;
}

void UPnP::close(void)
{
  d->updateInterfacesTimer.stop();

  if (d->initialized)
  {
    d->initialized = false;

    if (d->webServerEnabled)
    {
      d->webServerEnabled = false;
      ::UpnpRemoveAllVirtualDirs();
    }

    struct T : QThread
    {
      virtual void run() { ::UpnpFinish(); }
    } t;

    // Ugly, but needed as UpnpFinish waits for callbacks from the HTTP server.
    t.start();
    while (!t.isFinished()) { qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::WaitForMoreEvents, 16); }
    t.wait();

    clearResponses();
  }
}

bool UPnP::isLocalAddress(const char *address)
{
  return
      (qstrncmp(address, "10.", 3) == 0) ||
      (qstrncmp(address, "127.", 4) == 0) ||
      (qstrncmp(address, "169.254.", 8) == 0) ||
      (qstrncmp(address, "172.16.", 7) == 0) ||
      (qstrncmp(address, "172.17.", 7) == 0) ||
      (qstrncmp(address, "172.18.", 7) == 0) ||
      (qstrncmp(address, "172.19.", 7) == 0) ||
      (qstrncmp(address, "172.20.", 7) == 0) ||
      (qstrncmp(address, "172.21.", 7) == 0) ||
      (qstrncmp(address, "172.22.", 7) == 0) ||
      (qstrncmp(address, "172.23.", 7) == 0) ||
      (qstrncmp(address, "172.24.", 7) == 0) ||
      (qstrncmp(address, "172.25.", 7) == 0) ||
      (qstrncmp(address, "172.26.", 7) == 0) ||
      (qstrncmp(address, "172.27.", 7) == 0) ||
      (qstrncmp(address, "172.28.", 7) == 0) ||
      (qstrncmp(address, "172.29.", 7) == 0) ||
      (qstrncmp(address, "172.30.", 7) == 0) ||
      (qstrncmp(address, "172.31.", 7) == 0) ||
      (qstrncmp(address, "192.168.", 8) == 0);
}

bool UPnP::isMyAddress(const QByteArray &address) const
{
  const int colon = address.indexOf(':');

  return d->availableAddresses.find((colon > 0) ? address.left(colon) : address) != d->availableAddresses.end();
}

HttpStatus UPnP::handleHttpRequest(const QUrl &url, const HttpRequestInfo &requestInfo, QByteArray &contentType, QIODevice *&response)
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

void UPnP::customEvent(QEvent *e)
{
  if (e->type() == Data::Event::myType)
    static_cast<Data::Event *>(e)->f();
  else
    UPnP::customEvent(e);
}

void UPnP::clearResponses()
{
  QMutexLocker l(&d->responsesMutex);

  for (QMap<QByteArray, QPair<QIODevice *, QByteArray> >::Iterator i = d->responses.begin();
       i != d->responses.end();
       i = d->responses.erase(i))
  {
    i->first->deleteLater();
  }
}

void UPnP::closedFile(QObject *file)
{
  d->pendingFiles.remove(file);
  if (d->pendingFiles.isEmpty())
    d->updateInterfacesTimer.start(d->updateInterfacesInterval);
}

void UPnP::updateInterfaces()
{
  for (char **i = ::UpnpGetAvailableIpAddresses(); i && *i; i++)
  if (d->availableAddresses.find(*i) == d->availableAddresses.end())
  if (d->bindPublicInterfaces || isLocalAddress(*i))
  {
    close();
    initialize(d->port, d->bindPublicInterfaces);
    break;
  }
}

void UPnP::send(Functor &f) const
{
  QSemaphore sem;
  qApp->postEvent(Data::me, new Data::Event(f, &sem));
  sem.acquire();
}

void UPnP::enableWebserver()
{
  struct T
  {
    static int get_info(::Request_Info *request, const char *filename, ::File_Info *info)
    {
      QByteArray contentType;
      QIODevice *response = NULL;
      if (Data::me->getResponse(request->host, filename, request->userAgent, request->sourceAddress, contentType, response, false) == HttpStatus_Ok)
      {
        info->file_length = (response && !response->isSequential()) ? response->size() : -1;
        info->last_modified = QDateTime::currentDateTime().toTime_t();
        info->is_directory = FALSE;
        info->is_readable = TRUE;
        info->is_cacheable = FALSE;
        info->content_type = ::ixmlCloneDOMString(contentType);

        QFile * const file = qobject_cast<QFile *>(response);
        if (file)
        {
          QDateTime lastModified;
          if (file->fileName().startsWith(":/"))
            lastModified = QFileInfo(QCoreApplication::applicationFilePath()).lastModified();
          else
            lastModified = QFileInfo(file->fileName()).lastModified();

          if (lastModified.isValid())
            info->last_modified = lastModified.toTime_t();

          info->is_cacheable = TRUE;
        }

        return 0;
      }
      else
        return -1;
    }

    static ::UpnpWebFileHandle open(::Request_Info *request, const char *filename, ::UpnpOpenFileMode mode)
    {
      QByteArray contentType;
      QIODevice *response = NULL;
      if (Data::me->getResponse(request->host, filename, request->userAgent, request->sourceAddress, contentType, response, true) == HttpStatus_Ok)
      {
        if (response->open((mode == UPNP_READ) ? QIODevice::ReadOnly : QIODevice::WriteOnly))
        {
          return response;
        }
        else
          response->deleteLater();
      }

      return NULL;
    }

    static int read(::UpnpWebFileHandle fileHnd, char *buf, size_t len)
    {
      if (fileHnd)
        return qMax(int(reinterpret_cast<QIODevice *>(fileHnd)->read(buf, len)), 0);

      return UPNP_E_INVALID_HANDLE;
    }

    static int write(::UpnpWebFileHandle fileHnd, char *buf, size_t len)
    {
      if (fileHnd)
        return reinterpret_cast<QIODevice *>(fileHnd)->write(buf, len);

      return UPNP_E_INVALID_HANDLE;
    }

    static int seek(::UpnpWebFileHandle fileHnd, off_t offset, int origin)
    {
      if (fileHnd)
      {
        QIODevice * const ioDevice = reinterpret_cast<QIODevice *>(fileHnd);
        switch (origin)
        {
        case SEEK_SET:  return ioDevice->seek(offset) ? 0 : -1;
        case SEEK_CUR:  return ioDevice->seek(ioDevice->pos() + offset) ? 0 : -1;
        case SEEK_END:  return ioDevice->seek(ioDevice->size() + offset) ? 0 : -1;
        default:        return -1;
        }
      }

      return -1;
    }

    static int close(::UpnpWebFileHandle fileHnd)
    {
      if (fileHnd)
      {
        QIODevice * const ioDevice = reinterpret_cast<QIODevice *>(fileHnd);
        ioDevice->close();
        ioDevice->deleteLater();

        return UPNP_E_SUCCESS;
      }

      return UPNP_E_INVALID_HANDLE;
    }
  };

  ::UpnpEnableWebserver(TRUE);
  static const ::UpnpVirtualDirCallbacks callbacks = { &T::get_info, &T::open, &T::read, &T::write, &T::seek, &T::close };
  ::UpnpSetVirtualDirCallbacks(const_cast< ::UpnpVirtualDirCallbacks * >(&callbacks));

  d->webServerEnabled = true;
}

HttpStatus UPnP::getResponse(const QByteArray &host, const QByteArray &path, const QByteArray &userAgent, const QByteArray &sourceAddress, QByteArray &contentType, QIODevice *&response, bool erase)
{
  QMutexLocker l(&d->responsesMutex);

  QMap<QByteArray, QPair<QIODevice *, QByteArray> >::Iterator i = d->responses.find(path);
  if (i != d->responses.end())
  {
    response = i->first;
    contentType = i->second;
    if (erase)
      d->responses.erase(i);

    return HttpStatus_Ok;
  }
  else
  {
    l.unlock();

    struct F : Functor
    {
      F(UPnP *me, bool erase, const QUrl &url, const QByteArray &host, const QByteArray &userAgent, const QByteArray &sourceAddress)
        : me(me), erase(erase), url(url),
          response(NULL), result(HttpStatus_InternalServerError)
      {
        requestInfo.host = host;
        requestInfo.userAgent = userAgent;
        requestInfo.sourceAddress = sourceAddress;
      }

      void operator()()
      {
        if (me->d->initialized)
        {
          result = me->handleHttpRequest(url, requestInfo, contentType, response);
          if (response)
          {
            connect(response, SIGNAL(destroyed(QObject*)), me, SLOT(closedFile(QObject*)));
            if (me->d->pendingFiles.isEmpty())
              me->d->updateInterfacesTimer.stop();

            me->d->pendingFiles.insert(response);

            if (!erase && (result == HttpStatus_Ok))
              me->d->clearResponsesTimer.start(me->d->clearResponsesInterval);
          }
        }
        else
          result = HttpStatus_InternalServerError;
      }

      UPnP * const me;
      const bool erase;
      const QUrl url;
      HttpRequestInfo requestInfo;
      QByteArray contentType;
      QIODevice *response;
      HttpStatus result;
    } f(this, erase, QUrl("http://" + host + QString::fromUtf8(path)), host, userAgent, sourceAddress);

    send(f);

    if (!erase && (f.result == HttpStatus_Ok))
    {
      l.relock();

      d->responses[path] = qMakePair(f.response, f.contentType);
    }

    contentType = f.contentType;
    response = f.response;
    return f.result;
  }
}

/*! Returns the MIME type for the specified filename, based on the extension.
 */
const char * UPnP::toMimeType(const QString &fileName)
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
