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

namespace LXiStreamDevice {
namespace GdiCapture {

QMap<QString, QRect>                      ScreenInput::screens;
const int                                 ScreenInput::numImages = 5;
QMutex                                    ScreenInput::imagesMutex(QMutex::Recursive);
QAtomicInt                                ScreenInput::inputIdCounter = 0;
QMap< int, QStack<ScreenInput::Image *> > ScreenInput::images;

QList<SFactory::Scheme> ScreenInput::listDevices(void)
{
  QList<SFactory::Scheme> result;

  ScreenInput::screens.clear();

  QDesktopWidget * const desktop = qApp->desktop();
  for (int i=0; i<desktop->screenCount(); i++)
  {
    const QRect rect = desktop->screenGeometry(i);

    const QString name =
        "Desktop " + QString::number(i + 1) +
        " (" + QString::number(rect.width()) +
        " x " + QString::number(rect.height()) + ")";

    ScreenInput::screens.insert(name, rect);

    result += SFactory::Scheme(name);
  }

  return result;
}

ScreenInput::ScreenInput(const QString &screen, QObject *parent)
  : SInterfaces::VideoInput(parent),
    inputId(inputIdCounter.fetchAndAddAcquire(1)),
    screenDc(NULL),
    screenRect(),
    videoFormat()
{
  QMap<QString, QRect>::const_iterator i = screens.find(screen);
  if (i != screens.end())
  {
    screenRect = *i;

    screenDc = ::CreateDCA("DISPLAY", NULL, NULL, NULL);
    if (screenDc != NULL)
    {
      const int bpp = ::GetDeviceCaps(screenDc, BITSPIXEL);
      switch (bpp)
      {
      case 16:
        videoFormat = SVideoFormat(
              SVideoFormat::Format_RGB555,
              screenRect.size(),
              SInterval::fromFrequency(30));
        break;

      case 24:
        videoFormat = SVideoFormat(
              SVideoFormat::Format_RGB24,
              screenRect.size(),
              SInterval::fromFrequency(30));
        break;

      default:
      case 32:
        videoFormat = SVideoFormat(
              SVideoFormat::Format_RGB32,
              screenRect.size(),
              SInterval::fromFrequency(30));
        break;
      }
    }
  }
}

ScreenInput::~ScreenInput()
{
  ScreenInput::stop();

  if (screenDc != NULL)
    ::ReleaseDC(0, screenDc);
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

  if (!screenRect.isEmpty() && (screenDc != NULL))
  {
    QMap< int, QStack<Image *> >::Iterator imgs = images.find(inputId);
    if (imgs == images.end())
      imgs = images.insert(inputId, QStack<Image *>());

    for (int i=0; i<numImages; i++)
    {
      Image * const image = new Image();
      image->dc = ::CreateCompatibleDC(screenDc);
      if (image->dc != NULL)
      {
        memset(&image->bitmapInfo, 0, sizeof(image->bitmapInfo));
        image->bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        image->bitmapInfo.bmiHeader.biWidth = screenRect.width();
        image->bitmapInfo.bmiHeader.biHeight = -screenRect.height();
        image->bitmapInfo.bmiHeader.biPlanes = 1;
        image->bitmapInfo.bmiHeader.biBitCount = videoFormat.sampleSize() * 8;
        image->bitmapInfo.bmiHeader.biCompression = BI_RGB;

        image->bitmap = ::CreateDIBSection(image->dc, &image->bitmapInfo, DIB_RGB_COLORS, &image->buffer, NULL, 0);
        if (image->bitmap != NULL)
        {
          if (image->buffer != NULL)
          {
            ::SelectObject(image->dc, image->bitmap);

            imgs->push(image);

            continue;
          }

          ::DeleteObject(image->bitmap);
        }

        ::DeleteDC(image->dc);
      }

      delete image;
    }

    lastImage = timer.timeStamp();

    return true;
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
      deleteImage(imgs->pop());

    images.erase(imgs);
  }
}

bool ScreenInput::process(void)
{
  // Hack to get access to msleep()
  struct T : QThread { static inline void msleep(unsigned long msec) { QThread::msleep(msec); } };

  if (!screenRect.isEmpty() && (screenDc != NULL))
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

      if (::BitBlt(
            image->dc,
            0, 0,
            image->bitmapInfo.bmiHeader.biWidth, -image->bitmapInfo.bmiHeader.biHeight,
            screenDc,
            screenRect.x(), screenRect.y(),
            SRCCOPY | CAPTUREBLT))
      {
        const int offset[4] = { 0, 0, 0, 0 };
        const int lineSize[4] = { int(image->bitmapInfo.bmiHeader.biWidth) * videoFormat.sampleSize(), 0, 0, 0 };

        SVideoBuffer videoBuffer(videoFormat, SBuffer::MemoryPtr(new Memory(inputId, image)), offset, lineSize);
        videoBuffer.setTimeStamp(lastImage);
        emit produce(videoBuffer);
      }
      else
      {
        l.relock();

        imgs = images.find(inputId);
        if (imgs != images.end())
          imgs->push(image);
        else
          deleteImage(image);
      }
    }

    return true;
  }

  return false;
}

void ScreenInput::deleteImage(Image *image)
{
  ::DeleteObject(image->bitmap);
  ::DeleteDC(image->dc);
  delete image;
}

ScreenInput::Memory::Memory(int inputId, Image *image)
  : SBuffer::Memory(
      image->bitmapInfo.bmiHeader.biWidth * -image->bitmapInfo.bmiHeader.biHeight,
      reinterpret_cast<char *>(image->buffer),
      image->bitmapInfo.bmiHeader.biWidth * -image->bitmapInfo.bmiHeader.biHeight),
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
    deleteImage(image);
}

} } // End of namespaces
