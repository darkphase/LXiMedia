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
#include "svideobuffer.h"

namespace LXiStream {

struct SVideoResizeNode::Data
{
  QString                       algo;
  SInterfaces::VideoResizer   * resizer;
  SInterfaces::VideoResizer   * lanczosResizer;
};

SVideoResizeNode::SVideoResizeNode(SGraph *parent, const QString &algo)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->algo = algo;

  if (algo.isEmpty())
    d->resizer = SInterfaces::VideoResizer::create(this, "bilinear");
  else
    d->resizer = SInterfaces::VideoResizer::create(this, algo);

  if (d->resizer == NULL)
    d->resizer = SInterfaces::VideoResizer::create(this, QString::null);

  d->lanczosResizer = NULL;
}

SVideoResizeNode::~SVideoResizeNode()
{
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

  return Qt::KeepAspectRatio;
}

QStringList SVideoResizeNode::algorithms(void)
{
  return SInterfaces::VideoDeinterlacer::available();
}

bool SVideoResizeNode::start(void)
{
  return true;
}

void SVideoResizeNode::stop(void)
{
}

void SVideoResizeNode::input(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull() && d->resizer)
  {
    const SSize size = videoBuffer.format().size();
    SInterfaces::VideoResizer *resizer = d->resizer;
    if (d->lanczosResizer && (d->lanczosResizer->size().width() > size.width()) &&
        (size.width() < 1280) && (size.height() < 720))
    {
      resizer = d->lanczosResizer;
    }

    if (resizer->needsResize(videoBuffer.format()))
      emit output(resizer->processBuffer(videoBuffer));
    else
      emit output(videoBuffer);
  }
  else
    emit output(videoBuffer);
}

} // End of namespace
