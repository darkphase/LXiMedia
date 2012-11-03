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

#include "screeninput.h"
#include <xcb/xinerama.h>

namespace LXiStreamDevice {
namespace X11Capture {

QMap<QString, QRect> ScreenInput::screens;

QList<SFactory::Scheme> ScreenInput::listDevices(void)
{
  QList<SFactory::Scheme> result;

  ScreenInput::screens.clear();

  int snum = 0;
  ::xcb_connection_t * connnection = ::xcb_connect(NULL, &snum);
  if (!::xcb_connection_has_error(connnection))
  {
    ::xcb_xinerama_query_screens_reply_t * const screens = ::xcb_xinerama_query_screens_reply(
          connnection,
          ::xcb_xinerama_query_screens(connnection),
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

    ::xcb_disconnect(connnection);
  }

  return result;
}

ScreenInput::ScreenInput(const QString &screen, QObject *parent)
  : SInterfaces::VideoInput(parent),
    connnection(NULL),
    window(),
    screenRect(),
    videoFormat()
{
  QMap<QString, QRect>::const_iterator i = screens.find(screen);
  if (i != screens.end())
    screenRect = *i;
}

ScreenInput::~ScreenInput()
{
  imagesMutex.lock();

  if (connnection)
  {
    while (!images.isEmpty())
    {
      Image * const image = images.pop();

      ::xcb_shm_detach(connnection, image->shminfo.shmseg);
      ::shmdt(image->shminfo.shmaddr);
      ::shmctl(image->shminfo.shmid, IPC_RMID, 0);
      delete image;
    }

    ::xcb_disconnect(connnection);
    connnection = NULL;
  }

  while (!images.isEmpty())
  {
    Image * const image = images.pop();

    ::shmdt(image->shminfo.shmaddr);
    ::shmctl(image->shminfo.shmid, IPC_RMID, 0);
    delete image;
  }
}

void ScreenInput::setFormat(const SVideoFormat &)
{
}

SVideoFormat ScreenInput::format(void)
{
  return SVideoFormat(SVideoFormat::Format_RGB32, screenRect.size(), SInterval::fromFrequency(25));
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
    connnection = ::xcb_connect(NULL, &snum);
    if (!::xcb_connection_has_error(connnection))
    {
      const ::xcb_setup_t * const setup = ::xcb_get_setup(connnection);
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
          image->img.stride = image->img.width * (image->img.bpp / 8);
          image->img.size = image->img.stride * image->img.height;
          image->img.base = NULL;
          image->img.data = NULL;

          image->shminfo.shmid = ::shmget(IPC_PRIVATE, image->img.stride * image->img.height, IPC_CREAT | 0666);
          if (image->shminfo.shmid != uint32_t(-1))
          {
            image->shminfo.shmaddr = reinterpret_cast<uint8_t *>(::shmat(image->shminfo.shmid, 0, 0));
            image->img.data = image->shminfo.shmaddr;

            image->shminfo.shmseg = ::xcb_generate_id(connnection);
            ::xcb_shm_attach(connnection, image->shminfo.shmseg, image->shminfo.shmid, 0);

            QMutexLocker l(&imagesMutex);

            images.push(image);

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
  if (connnection)
  {
    QMutexLocker l(&imagesMutex);

    while (!images.isEmpty())
    {
      Image * const image = images.pop();

      ::xcb_shm_detach(connnection, image->shminfo.shmseg);
      ::shmdt(image->shminfo.shmaddr);
      ::shmctl(image->shminfo.shmid, IPC_RMID, 0);
      delete image;
    }

    ::xcb_disconnect(connnection);
    connnection = NULL;
  }
}

bool ScreenInput::process(void)
{
  // Hack to get access to msleep()
  struct T : QThread { static inline void msleep(unsigned long msec) { QThread::msleep(msec); } };

  if (connnection)
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

    if (!images.isEmpty())
    {
      Image * const image = images.pop();

      l.unlock();

      const ::xcb_shm_get_image_cookie_t cookie = ::xcb_shm_get_image(
            connnection, window,
            screenRect.x(), screenRect.y(), image->img.width, image->img.height,
            0xffffffff, image->img.format,
            image->shminfo.shmseg, image->img.data - image->shminfo.shmaddr);

      ::xcb_shm_get_image_reply_t * const ireply = ::xcb_shm_get_image_reply(
            connnection, cookie, NULL);

      if (ireply)
      {
        const int offset[4] = { 0, 0, 0, 0 };
        const int lineSize[4] = { int(image->img.stride), 0, 0, 0 };

        SVideoBuffer videoBuffer(videoFormat, SBuffer::MemoryPtr(new Memory(this, image)), offset, lineSize);
        videoBuffer.setTimeStamp(lastImage);
        emit produce(videoBuffer);

        ::free(ireply);
      }
      else
      {
        l.relock();

        images.push(image);
      }
    }

    return true;
  }

  return false;
}

ScreenInput::Memory::Memory(ScreenInput *parent, Image *image)
  : SBuffer::Memory(
      image->img.stride * image->img.height,
      reinterpret_cast<char *>(image->img.data),
      image->img.stride * image->img.height),
    parent(parent),
    image(image)
{
}

ScreenInput::Memory::~Memory()
{
  QMutexLocker l(&parent->imagesMutex);

  parent->images.push(image);
}

} } // End of namespaces
