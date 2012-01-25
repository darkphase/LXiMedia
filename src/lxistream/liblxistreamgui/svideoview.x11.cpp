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

#include "svideoview.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#include "simage.h"

namespace LXiStreamGui {

using namespace LXiStream;


struct SVideoView::Private
{
  inline Private(void) : mutex(QMutex::Recursive), formatConvert(NULL) { }

  QMutex                        mutex;
  bool                          slowUpdate;
  int                           skipCount;
  int                           updateTimerId;
  STimer                      * timer;
  SVideoBufferList              videoBuffers;

  QList<SVideoView *>           clones;
  SVideoView                  * source;

  SVideoFormatConvertNode       formatConvert;

  bool                          xvEnabled;
  Display                     * display;
  int                           xvPort;
  GC                            gc;
  XvImage                     * xvShmImage;
  XShmSegmentInfo               shmInfo;
  int                           shmCompletionType;
  SVideoFormat                  lastFormat;
  XvImageFormatValues           xvFormat;

  XvImageFormatValues           toXvFormat(SVideoFormat::Format) const;
  bool                          allocateImage(int, int, bool testOnly = false);
  void                          freeImage(void);
};

SVideoView::SVideoView(QWidget *parent)
  : QWidget(parent),
    p(new Private())
{
  p->slowUpdate = false;
  p->skipCount = 0;
  p->updateTimerId = -1;
  p->timer = NULL;

  p->source = NULL;

  p->formatConvert.setDestFormat(SVideoFormat::Format_RGB32);

  p->xvEnabled = false;
  p->display = XOpenDisplay(NULL);
  p->xvPort = -1;
  p->xvShmImage = NULL;
  p->shmCompletionType = 0;

  unsigned int version, release, requestBase, eventBase, errorBase;
  if (XvQueryExtension(p->display, &version, &release, &requestBase, &eventBase, &errorBase) == Success)
  {
    Window rootWin = RootWindow(p->display, 0);

    unsigned int numAdaptors;
    XvAdaptorInfo *adaptorInfo;
    if (XvQueryAdaptors(p->display, rootWin, &numAdaptors, &adaptorInfo) == Success)
    if (numAdaptors > 0)
    {
      p->xvPort = adaptorInfo[0].base_id;
      XvFreeAdaptorInfo(adaptorInfo);

      p->gc = XCreateGC(p->display, rootWin, 0, 0);

      // Test if shm works
      p->xvFormat = p->toXvFormat(SVideoFormat::Format_Invalid);
      if (p->allocateImage(64, 64, true))
        p->xvEnabled = true;
    }
  }

  if (p->xvEnabled)
  {
    // Detect supported codecs
    QList<SVideoFormat::Format> formats;
    foreach (SVideoFormat::Format format,
             QList<SVideoFormat::Format>()
                  << SVideoFormat::Format_YUV420P << SVideoFormat::Format_YUV422P
                  << SVideoFormat::Format_YUV410P << SVideoFormat::Format_YUV411P
                  << SVideoFormat::Format_YUV444P << SVideoFormat::Format_YUYV422
                  << SVideoFormat::Format_UYVY422
                  << SVideoFormat::Format_RGB32 << SVideoFormat::Format_BGR32
                  << SVideoFormat::Format_RGB24 << SVideoFormat::Format_BGR24
                  << SVideoFormat::Format_RGB565 << SVideoFormat::Format_BGR565
                  << SVideoFormat::Format_RGB555 << SVideoFormat::Format_BGR555)
    {
      if (p->toXvFormat(format).id != -1)
        formats << format;
    }
  }

  setAttribute(Qt::WA_NoSystemBackground);
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_PaintOnScreen);
}

SVideoView::~SVideoView()
{
  p->mutex.lock();

  if (p->source)
    p->source->removeClone(this);
  else
    p->freeImage();

  XCloseDisplay(p->display);

  delete p;
  *const_cast<Private **>(&p) = NULL;
}

bool SVideoView::slow(void) const
{
  return p->slowUpdate;
}

void SVideoView::setSlow(bool s)
{
  p->slowUpdate = s;
}

void SVideoView::addClone(SVideoView *clone)
{
  QMutexLocker l(&p->mutex);

  p->clones += clone;
  clone->setSource(this);
}

void SVideoView::removeClone(SVideoView *clone)
{
  QMutexLocker l(&p->mutex);

  p->clones.removeAll(clone);
  clone->setSource(NULL);
}

bool SVideoView::start(STimer *timer)
{
  p->timer = timer;
  if (p->updateTimerId == -1)
    p->updateTimerId = startTimer(10);

  return true;
}

void SVideoView::stop(void)
{
  QMutexLocker l(&p->mutex);

  p->timer = NULL;
  p->videoBuffers.clear();

  if (p->updateTimerId != -1)
  {
    killTimer(p->updateTimerId);
    p->updateTimerId = -1;
  }

  update();
}

void SVideoView::input(const SVideoBuffer &videoBuffer)
{
  QMutexLocker l(&p->mutex);

  if (p->timer && !videoBuffer.isNull())
  {
    if (((videoBuffer.format().format() >= SVideoFormat::Format_RGB555) &&
         (videoBuffer.format().format() <= SVideoFormat::Format_BGR32)) ||
        ((videoBuffer.format().format() >= SVideoFormat::Format_YUYV422) &&
         (videoBuffer.format().format() <= SVideoFormat::Format_YUV444P)))
    {
      p->videoBuffers.append(videoBuffer);
    }
    else
    {
      const SVideoBuffer buffer = p->formatConvert.convert(videoBuffer);
      if (!buffer.isNull())
        p->videoBuffers.append(buffer);
    }
  }
}

void SVideoView::paintEvent(QPaintEvent *)
{
  QPainter painter;
  painter.begin(this);
  painter.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha

  QMutexLocker l(&p->mutex);

  SImage image;
  if (p->source && !p->source->p->videoBuffers.isEmpty())
    image = SImage(p->source->p->videoBuffers.first());
  else if (!p->videoBuffers.isEmpty())
    image = SImage(p->videoBuffers.first());

  if (!image.isNull())
  {
    QSizeF sz(image.size().size());
    sz.scale(size(), Qt::KeepAspectRatio);
    image = image.scaled(sz.toSize(), Qt::IgnoreAspectRatio, (image.height() <= 576) ? Qt::SmoothTransformation : Qt::FastTransformation);

    const QPoint pos((width() / 2) - (image.width() / 2), (height() / 2) - (image.height() / 2));
    painter.drawImage(pos, image);

    foreach (SVideoView *clone, p->clones)
      clone->repaint();
  }
  else
    painter.fillRect(rect(), Qt::black);

  painter.end();
}

void SVideoView::timerEvent(QTimerEvent *e)
{
  if (e->timerId() == p->updateTimerId)
  {
    QMutexLocker l(&p->mutex);

    const STime now = p->timer ? (p->timer->timeStamp()) : STime();
    bool updateRequired = false;

    while (p->videoBuffers.count() > 1)
    {
      const SVideoBuffer head = p->videoBuffers.first();
      if (!now.isValid() || (head.timeStamp() < now))
      {
        p->videoBuffers.takeFirst();
        updateRequired = true;
      }
      else
      {
        if (p->timer)
          p->timer->correctOffset(head.timeStamp(), STime::fromSec(10));

        break;
      }
    }

    if (updateRequired)
      blitVideo();
  }
  else
    QWidget::timerEvent(e);
}

void SVideoView::blitVideo(void)
{
  QMutexLocker l(&p->mutex);

  if (p->xvEnabled && !p->videoBuffers.isEmpty())
  {
    const SVideoBuffer buffer = p->videoBuffers.first();
    const SVideoFormat format = buffer.format();

    if ((p->xvShmImage == NULL) || (p->lastFormat != format) ||
        (p->xvShmImage->width != format.size().width()) ||
        (p->xvShmImage->height != format.size().height()))
    {
      p->freeImage();
      p->lastFormat = format;
      p->xvFormat = p->toXvFormat(format);
      if (p->xvFormat.id != -1)
        p->allocateImage(format.size().width(), format.size().height());
    }

    if (p->xvShmImage)
    {
      if ((format == SVideoFormat::Format_RGB555) ||
          (format == SVideoFormat::Format_BGR555) ||
          (format == SVideoFormat::Format_RGB565) ||
          (format == SVideoFormat::Format_BGR565) ||
          (format == SVideoFormat::Format_RGB24) ||
          (format == SVideoFormat::Format_BGR24) ||
          (format == SVideoFormat::Format_BGR32) ||
          (format == SVideoFormat::Format_RGB32) ||
          (format == SVideoFormat::Format_YUYV422) ||
          (format == SVideoFormat::Format_UYVY422))
      {
        unsigned pos = p->xvShmImage->offsets[0];
        for (int i=0; i<format.size().height(); i++)
        {
          memcpy(p->xvShmImage->data + pos,
                 buffer.scanLine(i, 0),
                 buffer.lineSize(0));

          pos += p->xvShmImage->pitches[0];
        }
      }
      else if ((format == SVideoFormat::Format_YUV410P) ||
               (format == SVideoFormat::Format_YUV411P) ||
               (format == SVideoFormat::Format_YUV420P) ||
               (format == SVideoFormat::Format_YUV422P) ||
               (format == SVideoFormat::Format_YUV444P))
      {
        // Y
        unsigned pos = p->xvShmImage->offsets[0];
        for (int i=0; i<format.size().height(); i++)
        {
          memcpy(p->xvShmImage->data + pos,
                 buffer.scanLine(i, 0),
                 qMin(buffer.lineSize(0), p->xvShmImage->width));

          pos += p->xvShmImage->pitches[0];
        }

        int wr = 1, hr = 1;
        if (format.planarYUVRatio(wr, hr))
        {
          // U
          pos = p->xvShmImage->offsets[1];
          for (unsigned i=0, n=format.size().height()/hr; i<n; i++)
          {
            memcpy(p->xvShmImage->data + pos,
                   buffer.scanLine(i, 1),
                   qMin(buffer.lineSize(1), p->xvShmImage->width / wr));

            pos += p->xvShmImage->pitches[1];
          }

          // V
          pos = p->xvShmImage->offsets[2];
          for (unsigned i=0, n=format.size().height()/hr; i<n; i++)
          {
            memcpy(p->xvShmImage->data + pos,
                   buffer.scanLine(i, 2),
                   qMin(buffer.lineSize(2), p->xvShmImage->width / wr));

            pos += p->xvShmImage->pitches[2];
          }
        }
      }
    }

    if (p->source)
      p->xvShmImage = p->source->p->xvShmImage;

    if (p->xvShmImage)
    {
      QSizeF sz(format.size().absoluteSize());
      sz.scale(size(), Qt::KeepAspectRatio);
      const QPoint pos((width() / 2) - (sz.width() / 2), (height() / 2) - (sz.height() / 2));

      if (!p->source)
      {
        XvShmPutImage(p->display, p->xvPort, winId(), p->gc, p->xvShmImage,
                      0, 0, p->xvShmImage->width, p->xvShmImage->height,
                      pos.x(), pos.y(), sz.width(), sz.height(),
                      p->source ? False : True);

        struct T
        {
          static Bool predicate(Display *, XEvent *e, XPointer arg)
          {
            const Private * const p = reinterpret_cast<const Private *>(arg);

            return (e->type == p->shmCompletionType) ? True : False;
          }
        };

        // Wait for an XShmCompletionEvent
        XEvent e;
        XIfEvent(p->display, &e, &T::predicate, reinterpret_cast<XPointer>(p));
      }
      else
      {
        XvPutImage(p->display, p->xvPort, winId(), p->gc, p->xvShmImage,
                   0, 0, p->xvShmImage->width, p->xvShmImage->height,
                   pos.x(), pos.y(), sz.width(), sz.height());
      }

      foreach (SVideoView *clone, p->clones)
        clone->blitVideo();

      return;
    }
  }

  // Fallback; simply paint image.
  repaint();
}

void SVideoView::setSource(SVideoView *source)
{
  QMutexLocker l(&p->mutex);

  p->source = source;
}


XvImageFormatValues SVideoView::Private::toXvFormat(SVideoFormat::Format format) const
{
  int numFormats;
  XvImageFormatValues * const xvFormats = XvListImageFormats(display, xvPort, &numFormats);
  if (xvFormats && (numFormats > 0))
  {
    if (format == SVideoFormat::Format_Invalid) // Any format
    {
      XvImageFormatValues result = xvFormats[0];
      XFree(xvFormats);
      return result;
    }

////////////////////////////////////////////////////////////////////////////////
#define SELECT \
    { \
      XvImageFormatValues result = xvFormats[i]; \
      XFree(xvFormats); \
      return result; \
    }
#define CHECKPACKEDRGB(f, b, rm, gm, bm) \
  case f: \
    for (int i=0; i<numFormats; i++) \
    if ((xvFormats[i].type == XvRGB) && (xvFormats[i].format == XvPacked) && (xvFormats[i].bits_per_pixel == b)) \
    if ((xvFormats[i].red_mask == rm) && (xvFormats[i].green_mask == gm) && (xvFormats[i].blue_mask == bm)) \
      SELECT \
    break;
#define CHECKPACKEDYUV(f, o) \
  case f: \
    for (int i=0; i<numFormats; i++) \
    if ((xvFormats[i].type == XvYUV) && (xvFormats[i].format == XvPacked)) \
    if ((xvFormats[i].component_order[0] == o[0]) && (xvFormats[i].component_order[1] == o[1]) && (xvFormats[i].component_order[2] == o[2])) \
      SELECT \
    break;
#define CHECKPLANARYUV(f, o) \
  case f: \
  { \
    for (int i=0; i<numFormats; i++) \
    if ((xvFormats[i].type == XvYUV) && (xvFormats[i].format == XvPlanar)) \
    if ((xvFormats[i].component_order[0] == o[0]) && (xvFormats[i].component_order[1] == o[1]) && (xvFormats[i].component_order[2] == o[2]) && (xvFormats[i].component_order[3] == o[3])) \
    { \
      int h, v; \
      if (SVideoFormat::planarYUVRatio(f, h, v)) \
      if ((xvFormats[i].y_sample_bits = 8) && (xvFormats[i].u_sample_bits = 8) && (xvFormats[i].v_sample_bits = 8)) \
      if ((xvFormats[i].horz_y_period = 1) && (xvFormats[i].vert_y_period = 1)) \
      if ((xvFormats[i].horz_u_period = h) && (xvFormats[i].vert_u_period = v)) \
      if ((xvFormats[i].horz_v_period = h) && (xvFormats[i].vert_v_period = v)) \
        SELECT \
      break; \
    } \
  }
////////////////////////////////////////////////////////////////////////////////

    switch (format)
    {
      CHECKPACKEDRGB(SVideoFormat::Format_RGB555, 16, 0x7C00, 0x03E0, 0x001F)
      CHECKPACKEDRGB(SVideoFormat::Format_BGR555, 16, 0x001F, 0x03E0, 0x7C00)
      CHECKPACKEDRGB(SVideoFormat::Format_RGB565, 16, 0xF800, 0x07E0, 0x001F)
      CHECKPACKEDRGB(SVideoFormat::Format_BGR565, 16, 0x001F, 0x07E0, 0xF800)
      CHECKPACKEDRGB(SVideoFormat::Format_RGB24, 24, 0x00FF0000, 0x0000FF00, 0x000000FF)
      CHECKPACKEDRGB(SVideoFormat::Format_BGR24, 24, 0x000000FF, 0x0000FF00, 0x00FF0000)
      CHECKPACKEDRGB(SVideoFormat::Format_RGB32, 32, 0x00FF0000, 0x0000FF00, 0x000000FF)
      CHECKPACKEDRGB(SVideoFormat::Format_BGR32, 32, 0x000000FF, 0x0000FF00, 0x00FF0000)

      CHECKPACKEDYUV(SVideoFormat::Format_YUYV422, "YUYV")
      CHECKPACKEDYUV(SVideoFormat::Format_UYVY422, "UYVY")
      CHECKPLANARYUV(SVideoFormat::Format_YUV410P, "YUV")
      CHECKPLANARYUV(SVideoFormat::Format_YUV411P, "YUV")
      CHECKPLANARYUV(SVideoFormat::Format_YUV420P, "YUV")
      CHECKPLANARYUV(SVideoFormat::Format_YUV422P, "YUV")
      CHECKPLANARYUV(SVideoFormat::Format_YUV444P, "YUV")

      // Not supported
      case SVideoFormat::Format_GRAY8:
      case SVideoFormat::Format_GRAY16BE:
      case SVideoFormat::Format_GRAY16LE:
      case SVideoFormat::Format_BGGR8:
      case SVideoFormat::Format_GBRG8:
      case SVideoFormat::Format_GRBG8:
      case SVideoFormat::Format_RGGB8:
      case SVideoFormat::Format_BGGR10:
      case SVideoFormat::Format_GBRG10:
      case SVideoFormat::Format_GRBG10:
      case SVideoFormat::Format_RGGB10:
      case SVideoFormat::Format_BGGR16:
      case SVideoFormat::Format_GBRG16:
      case SVideoFormat::Format_GRBG16:
      case SVideoFormat::Format_RGGB16:
      case SVideoFormat::Format_Invalid:
        break;
    }

    XFree(xvFormats);
#undef SELECT
#undef CHECKPACKEDRGB
#undef CHECKPACKEDYUV
#undef CHECKPLANARYUV
  }

  XvImageFormatValues dummy;
  dummy.id = -1;
  return dummy;
}

bool SVideoView::Private::allocateImage(int w, int h, bool testOnly)
{
  bool result = false;

  xvShmImage = XvShmCreateImage(display, xvPort, xvFormat.id, NULL, w, h, &shmInfo);
  if (xvShmImage)
  {
    shmInfo.readOnly = False;
    shmInfo.shmid = shmget(IPC_PRIVATE, xvShmImage->data_size, IPC_CREAT | 0777);
    if (shmInfo.shmid >= 0)
    {
      xvShmImage->data = shmInfo.shmaddr = (char *)shmat(shmInfo.shmid, NULL, 0);
      if (shmInfo.shmaddr != ((void *)-1))
      {
        if (!testOnly)
        {
          if (XShmAttach(display, &shmInfo) != 0)
          {
            XSync(display, False);
            shmCompletionType = XShmGetEventBase(display) + ShmCompletion;
            return true;
          }
        }
        else
          result = true;

        shmdt(shmInfo.shmaddr);
      }

      shmctl(shmInfo.shmid, IPC_RMID, NULL);
    }

    XFree(xvShmImage);
    xvShmImage = NULL;
  }

  return result;
}

void SVideoView::Private::freeImage(void)
{
  if (xvShmImage)
  {
    if (shmInfo.shmid >= 0)
    {
      if (shmInfo.shmaddr != ((void *)-1))
      {
        XShmDetach(display, &shmInfo);
        shmdt(shmInfo.shmaddr);
      }

      shmctl(shmInfo.shmid, IPC_RMID, NULL);
    }

    XFree(xvShmImage);
    xvShmImage = NULL;
  }
}


} // End of namespace
