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

#include "nodes/svideoresizenode.h"
#include "sgraph.h"
#include "svideobuffer.h"

namespace LXiStream {

struct SVideoResizeNode::Data
{
  SDependency                 * dependency;
  QString                       algo;
  SInterfaces::VideoResizer   * resizer;
  SInterfaces::VideoResizer   * lanczosResizer;
};

SVideoResizeNode::SVideoResizeNode(SGraph *parent, const QString &algo)
  : QObject(parent),
    SInterfaces::Node(parent),
    d(new Data())
{
  d->dependency = parent ? new SDependency() : NULL;
  d->algo = algo;
  d->lanczosResizer = NULL;

  if (algo.isEmpty())
    d->resizer = SInterfaces::VideoResizer::create(this, "bilinear");
  else
    d->resizer = SInterfaces::VideoResizer::create(this, algo);

  if (d->resizer == NULL)
    d->resizer = SInterfaces::VideoResizer::create(this, QString::null);
}

SVideoResizeNode::~SVideoResizeNode()
{
  delete d->dependency;
  delete d->resizer;
  delete d->lanczosResizer;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

/*! Enables Lanczos filtering for upsampling if no algorithm was specified
    during construction. Does nothing if an algorithm was specified during
    construction.
 */
void SVideoResizeNode::setHighQuality(bool enabled)
{
  if (d->algo.isEmpty())
  {
    delete d->lanczosResizer;

    if (enabled)
      d->lanczosResizer = SInterfaces::VideoResizer::create(this, "lanczos");
    else
      d->lanczosResizer = NULL;
  }
}

bool SVideoResizeNode::highQuality(void) const
{
  return d->lanczosResizer;
}

void SVideoResizeNode::setSize(const SSize &size)
{
  if (d->resizer)
    d->resizer->setSize(size);

  if (d->lanczosResizer)
    d->lanczosResizer->setSize(size);
}

SSize SVideoResizeNode::size(void) const
{
  if (d->resizer)
    return d->resizer->size();
  else
    return SSize();
}

void SVideoResizeNode::setAspectRatioMode(Qt::AspectRatioMode a)
{
  if (d->resizer)
    d->resizer->setAspectRatioMode(a);

  if (d->lanczosResizer)
    d->lanczosResizer->setAspectRatioMode(a);
}

Qt::AspectRatioMode SVideoResizeNode::aspectRatioMode(void) const
{
  if (d->resizer)
    return d->resizer->aspectRatioMode();
  else
    return Qt::KeepAspectRatio;
}

QStringList SVideoResizeNode::algorithms(void)
{
  return SInterfaces::VideoDeinterlacer::available();
}

void SVideoResizeNode::input(const SVideoBuffer &videoBuffer)
{
  if (d->resizer)
  {
    SInterfaces::VideoResizer *resizer = d->resizer;
    if (d->lanczosResizer)
    if ((d->lanczosResizer->size().height() > videoBuffer.format().size().height()) &&
        (videoBuffer.format().size().height() < 720))
    {
      resizer = d->lanczosResizer;
    }

    if (graph)
      graph->queue(this, &SVideoResizeNode::processTask, videoBuffer, resizer, d->dependency);
    else
      processTask(videoBuffer, resizer);
  }
  else
    emit output(videoBuffer);
}

void SVideoResizeNode::processTask(const SVideoBuffer &videoBuffer, SInterfaces::VideoResizer *resizer)
{
  emit output(resizer->processBuffer(videoBuffer));
}

} // End of namespace
