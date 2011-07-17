/***************************************************************************
 *   Copyright (C) 2010 by A.J. Admiraal                                   *
 *   code@admiraal.dds.nl                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include "svideoview.h"
#include <QtEndian>

namespace LXiStreamGui {

using namespace LXiStream;


struct SVideoView::Private
{
  inline Private(void) : mutex(QMutex::Recursive) { }

  QMutex                        mutex;
  bool                          slowUpdate;
  int                           skipCount;
  int                           updateTimerId;
  STimer                      * timer;
  SVideoBufferList              videoBuffers;

  QList<SVideoView *>           clones;
  SVideoView                  * source;
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

  setAttribute(Qt::WA_OpaquePaintEvent);
}

SVideoView::~SVideoView()
{
  p->mutex.lock();

  if (p->source)
    p->source->removeClone(this);

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
    p->updateTimerId = startTimer(20);

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

  if (!videoBuffer.isNull())
    p->videoBuffers.append(videoBuffer);
}

void SVideoView::paintEvent(QPaintEvent *)
{
  QPainter painter;
  painter.begin(this);
  painter.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha

  QMutexLocker l(&p->mutex);

  /*SImageBuffer imageBuffer;
  if (p->source && !p->source->p->videoBuffers.isEmpty())
    imageBuffer = p->source->p->videoBuffers.first();
  else if (!p->videoBuffers.isEmpty())
    imageBuffer = p->videoBuffers.first();

  if (!imageBuffer.isNull())
  {
    QSizeF sz(imageBuffer.codec().size().absoluteSize());
    sz.scale(size(), Qt::KeepAspectRatio);
    QImage image = imageBuffer.toImage();
    if (!image.isNull())
    {
      image = image.scaled(sz.toSize(), Qt::IgnoreAspectRatio, (image.height() <= 576) ? Qt::SmoothTransformation : Qt::FastTransformation);
      const QPoint pos((width() / 2) - (image.width() / 2), (height() / 2) - (image.height() / 2));
      if ((pos.x() != 0) || (pos.y() != 0))
        painter.fillRect(rect(), Qt::black);

      painter.drawImage(pos, image);
    }

    foreach (SVideoView *clone, p->clones)
      clone->repaint();
  }
  else*/
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
        break;
    }

    if (updateRequired)
      blitVideo();
  }
  else
    QWidget::timerEvent(e);
}

void SVideoView::blitVideo(void)
{
  // Fallback; simply paint image.
  repaint();
}

void SVideoView::setSource(SVideoView *source)
{
  QMutexLocker l(&p->mutex);

  p->source = source;
}


} // End of namespace
