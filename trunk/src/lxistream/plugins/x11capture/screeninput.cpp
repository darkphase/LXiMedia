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

#include "screeninput.h"
#include <xcb/xinerama.h>

namespace LXiStreamDevice {
namespace X11Capture {

QMap<QString, QRect>                      ScreenInput::screens;
const int                                 ScreenInput::numImages = 5;
QMutex                                    ScreenInput::imagesMutex(QMutex::Recursive);
QAtomicInt                                ScreenInput::inputIdCounter = 0;
QMap< int, QStack<ScreenInput::Image *> > ScreenInput::images;

QList<SFactory::Scheme> ScreenInput::listDevices(void)
{
  QList<SFactory::Scheme> result;

  ScreenInput::screens.clear();

  int snum = 0;
  ::xcb_connection_t * connection = ::xcb_connect(NULL, &snum);
  if (!::xcb_connection_has_error(connection))
  {
    ::xcb_xinerama_query_screens_reply_t * const screens = ::xcb_xinerama_query_screens_reply(
          connection,
          ::xcb_xinerama_query_screens(connection),
          NULL);

    int count = 1;
    for (::xcb_xinerama_screen_info_iterator_t i = ::xcb_xinerama_query_screens_screen_info_iterator(screens);
         i.rem > 0;
         ::xcb_xinerama_screen_info_next(&i))
    {
      const QString name =
          "Desktop " + QString::number(count++) +
          " (" + QString::number(i.data->width) +
          " x " + QString::number(i.data->height) + ")";

      ScreenInput::screens.insert(
            name,
            QRect(i.data->x_org, i.data->y_org, i.data->width, i.data->height));

      result += SFactory::Scheme(name);
    }

    ::free(screens);

    ::xcb_disconnect(connection);
  }

  return result;
}

ScreenInput::ScreenInput(const QString &screen, QObject *parent)
  : SInterfaces::VideoInput(parent),
    inputId(inputIdCounter.fetchAndAddAcquire(1)),
    connection(NULL),
    window(),
    screenRect(),
    videoFormat()
{
  QMap<QString, QRect>::const_iterator i = screens.find(screen);
  if (i != screens.end())
  {
    screenRect = *i;

    videoFormat = SVideoFormat(
          SVideoFormat::Format_RGB32,
          screenRect.size(),
          SInterval::fromFrequency(30));
  }
}

ScreenInput::~ScreenInput()
{
  ScreenInput::stop();
}

void ScreenInput::setFormat(const SVideoFormat &format)
{
  videoFormat.setFrameRate(format.frameRate());
}

SVideoFormat ScreenInput::format(void)
{
  return videoFormat;
}

void ScreenInput::setMaxBuffers(int)
{
}

int ScreenInput::maxBuffers(void) const
{
  return 1;
}

bool ScreenInput::start(void)
{
  stop();

  if (!screenRect.isEmpty())
  {
    int snum = 0;
    connection = ::xcb_connect(NULL, &snum);
    if (!::xcb_connection_has_error(connection))
    {
      const ::xcb_setup_t * const setup = ::xcb_get_setup(connection);
      const ::xcb_screen_t *scr = NULL;
      for (::xcb_screen_iterator_t i = ::xcb_setup_roots_iterator(setup);
           i.rem > 0;
           ::xcb_screen_next(&i))
      {
          if (snum == 0)
          {
              scr = i.data;
              break;
          }

          snum--;
      }

      if (scr != NULL)
      {
        QMutexLocker l(&imagesMutex);

        QMap< int, QStack<Image *> >::Iterator imgs = images.find(inputId);
        if (imgs == images.end())
          imgs = images.insert(inputId, QStack<Image *>());

        window = scr->root;

        videoFormat = SVideoFormat(
              SVideoFormat::Format_RGB32,
              SSize(screenRect.size()),
              SInterval::fromFrequency(25),
              SVideoFormat::FieldMode_Progressive);

        for (int i=0; i<numImages; i++)
        {
          Image * const image = new Image();
          image->img.width = screenRect.width();
          image->img.height = screenRect.height();
          image->img.format = XCB_IMAGE_FORMAT_Z_PIXMAP;
          image->img.depth = 24;
          image->img.bpp = 32;
          image->img.unit = 32;
          image->img.plane_mask = 0xFFFFFFFF;
          image->img.byte_order = XCB_IMAGE_ORDER_LSB_FIRST;
          image->img.bit_order = XCB_IMAGE_ORDER_LSB_FIRST;
          image->img.stride = SBuffer::align(image->img.width * (image->img.bpp / 8));
          image->img.size = image->img.stride * image->img.height;
          image->img.base = NULL;
          image->img.data = NULL;

          image->shminfo.shmid = ::shmget(IPC_PRIVATE, image->img.stride * image->img.height, IPC_CREAT | 0666);
          if (image->shminfo.shmid != uint32_t(-1))
          {
            image->shminfo.shmaddr = reinterpret_cast<uint8_t *>(::shmat(image->shminfo.shmid, 0, 0));
            image->img.data = image->shminfo.shmaddr;

            image->shminfo.shmseg = ::xcb_generate_id(connection);
            ::xcb_shm_attach(connection, image->shminfo.shmseg, image->shminfo.shmid, 0);

            imgs->push(image);

            continue;
          }
        }

        lastImage = timer.timeStamp();

        return true;
      }
    }
  }

  return false;
}

void ScreenInput::stop(void)
{
  QMutexLocker l(&imagesMutex);

  QMap< int, QStack<Image *> >::Iterator imgs = images.find(inputId);
  if (imgs != images.end())
  {
    while (!imgs->isEmpty())
      deleteImage(connection, imgs->pop());

    images.erase(imgs);
  }

  if (connection)
  {
    ::xcb_disconnect(connection);
    connection = NULL;
  }
}

bool ScreenInput::process(void)
{
  // Hack to get access to msleep()
  struct T : QThread { static inline void msleep(unsigned long msec) { QThread::msleep(msec); } };

  if (connection)
  {
    const STime now = timer.timeStamp();
    const STime next = lastImage + videoFormat.frameRate();
    const qint64 wait = (next - now).toMSec();

    if (qAbs(wait) >= 1000)
      lastImage = now;
    else if (wait > 0)
      T::msleep(wait);

    lastImage += videoFormat.frameRate();

    QMutexLocker l(&imagesMutex);

    QMap< int, QStack<Image *> >::Iterator imgs = images.find(inputId);
    if ((imgs != images.end()) && !imgs->isEmpty())
    {
      Image * const image = imgs->pop();

      l.unlock();

      const ::xcb_shm_get_image_cookie_t cookie = ::xcb_shm_get_image(
            connection, window,
            screenRect.x(), screenRect.y(), image->img.width, image->img.height,
            0xffffffff, image->img.format,
            image->shminfo.shmseg, image->img.data - image->shminfo.shmaddr);

      ::xcb_shm_get_image_reply_t * const ireply = ::xcb_shm_get_image_reply(
            connection, cookie, NULL);

      if (ireply)
      {
        const int offset[4] = { 0, 0, 0, 0 };
        const int lineSize[4] = { int(image->img.stride), 0, 0, 0 };

        SVideoBuffer videoBuffer(videoFormat, SBuffer::MemoryPtr(new Memory(inputId, image)), offset, lineSize);
        videoBuffer.setTimeStamp(lastImage);
        emit produce(videoBuffer);

        ::free(ireply);
      }
      else
      {
        l.relock();

        imgs = images.find(inputId);
        if (imgs != images.end())
          imgs->push(image);
        else
          deleteImage(connection, image);
      }
    }

    return true;
  }

  return false;
}

void ScreenInput::deleteImage(::xcb_connection_t *connection, Image *image)
{
  if (connection)
    ::xcb_shm_detach(connection, image->shminfo.shmseg);

  ::shmdt(image->shminfo.shmaddr);
  ::shmctl(image->shminfo.shmid, IPC_RMID, 0);
  delete image;
}

ScreenInput::Memory::Memory(int inputId, Image *image)
  : SBuffer::Memory(
      image->img.stride * image->img.height,
      reinterpret_cast<char *>(image->img.data),
      image->img.stride * image->img.height),
    inputId(inputId),
    image(image)
{
}

ScreenInput::Memory::~Memory()
{
  QMutexLocker l(&imagesMutex);

  QMap< int, QStack<Image *> >::Iterator imgs = images.find(inputId);
  if (imgs != images.end())
    imgs->push(image);
  else
    deleteImage(NULL, image);
}

} } // End of namespaces
