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
  if (connnection)
  {
    ::xcb_disconnect(connnection);
    connnection = NULL;
  }
}

void ScreenInput::setFormat(const SVideoFormat &)
{
}

SVideoFormat ScreenInput::format(void)
{
  return SVideoFormat(SVideoFormat::Format_RGB32);
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

    xcb_get_image_reply_t * const img = ::xcb_get_image_reply(
          connnection,
          ::xcb_get_image(
            connnection,
            XCB_IMAGE_FORMAT_Z_PIXMAP,
            window,
            screenRect.x(), screenRect.y(), screenRect.width(), screenRect.height(),
            ~0),
          NULL);

    if (img)
    {
      const int offset[4] = { 0, 0, 0, 0 };
      const int lineSize[4] = { int(screenRect.width() * sizeof(uint32_t)), 0, 0, 0 };

      SVideoBuffer videoBuffer(videoFormat, SBuffer::MemoryPtr(new Memory(img)), offset, lineSize);
      videoBuffer.setTimeStamp(lastImage);
      emit produce(videoBuffer);

      return true;
    }
  }

  return false;
}

ScreenInput::Memory::Memory(::xcb_get_image_reply_t *img)
  : SBuffer::Memory(
      ::xcb_get_image_data_length(img),
      reinterpret_cast<char *>(::xcb_get_image_data(img)),
      ::xcb_get_image_data_length(img)),
    img(img)
{
}

ScreenInput::Memory::~Memory()
{
  ::free(img);
}

} } // End of namespaces
