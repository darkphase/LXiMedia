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

namespace LXiStreamGui {

using namespace LXiStream;

SVideoViewSink * SVideoView::createSink(SGraph *graph)
{
  return new SVideoViewSink(graph, this);
}

SVideoViewSink::SVideoViewSink(SGraph *graph, SVideoView *parent)
  : SInterfaces::SinkNode(graph),
    parent(parent)
{
}

SVideoViewSink::~SVideoViewSink()
{
}

bool SVideoViewSink::start(STimer *timer)
{
  return parent->start(timer);
}

void SVideoViewSink::stop(void)
{
  parent->stop();
}

void SVideoViewSink::input(const SVideoBuffer &videoBuffer)
{
  parent->input(videoBuffer);
}

} // End of namespace
