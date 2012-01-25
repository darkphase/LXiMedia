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

#include "nodes/ssubtitlerendernode.h"
#include "ssubtitlebuffer.h"
#include "svideobuffer.h"

#if defined(_MSC_VER)
#pragma warning (disable : 4200)
#endif

namespace LXiStream {

struct SSubtitleRenderNode::Data
{
  QMap<STime, SSubtitleBuffer>  subtitles;
  SSubtitleBuffer               subtitle;
  STime                         subtitleTime;
  SSize                         videoSize;
  SInterfaces::SubtitleRenderer * renderer;
};

SSubtitleRenderNode::SSubtitleRenderNode(SGraph *parent)
  : SInterfaces::Node(parent),
    d(new Data())
{
  d->renderer = SInterfaces::SubtitleRenderer::create(this, QString::null);
}

SSubtitleRenderNode::~SSubtitleRenderNode()
{
  delete d->renderer;
  delete d;
  *const_cast<Data **>(&d) = NULL;
}

float SSubtitleRenderNode::fontSize(void) const
{
  if (d->renderer)
    return d->renderer->fontSize();

  return 1.0f;
}

void SSubtitleRenderNode::setFontSize(float s)
{
  if (d->renderer)
    return d->renderer->setFontSize(s);
}

bool SSubtitleRenderNode::start(void)
{
  d->videoSize = SSize();

  return true;
}

void SSubtitleRenderNode::stop(void)
{
}

void SSubtitleRenderNode::input(const SSubtitleBuffer &subtitleBuffer)
{
  if (d->renderer)
  {
    bool prerenderNext = d->subtitles.isEmpty();

    if (subtitleBuffer.duration().toSec() <= 10)
    {
      d->subtitles.insert(subtitleBuffer.timeStamp(), subtitleBuffer);
    }
    else // Prevent showing subtitles too long.
    {
      SSubtitleBuffer corrected = subtitleBuffer;
      corrected.setDuration(STime::fromSec(10));
      d->subtitles.insert(subtitleBuffer.timeStamp(), corrected);
    }

    if (prerenderNext && !d->videoSize.isNull())
      d->renderer->prepareSubtitle(subtitleBuffer, d->videoSize);
  }
}

void SSubtitleRenderNode::input(const SVideoBuffer &videoBuffer)
{
  if (!videoBuffer.isNull() && d->renderer)
  {
    const STime timeStamp = videoBuffer.timeStamp();

    // Can the current subtitle be removed.
    if (!d->subtitle.isNull() && ((d->subtitleTime + d->subtitle.duration()) < timeStamp))
    {
      d->subtitle = SSubtitleBuffer();
      d->subtitleTime = STime();
    }

    // Find the new current subtitle.
    QMap<STime, SSubtitleBuffer>::Iterator i = d->subtitles.begin();
    bool prerenderNext = false;

    while (i != d->subtitles.end())
    {
      if (i.key() <= timeStamp)
      {
        d->subtitle = *i;
        d->subtitleTime = i.key();

        i = d->subtitles.erase(i);
        prerenderNext = true;
      }
      else
        break;
    }

    // Prerender the next subtitle.
    d->videoSize = videoBuffer.format().size();
    if (prerenderNext && (i != d->subtitles.end()))
      d->renderer->prepareSubtitle(*i, d->videoSize);

    // Render the current subtitle
    if (!d->subtitle.isNull())
      emit output(d->renderer->processBuffer(videoBuffer, d->subtitle));
    else
      emit output(videoBuffer);
  }
  else
    emit output(videoBuffer);
}

} // End of namespace
