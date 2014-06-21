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

#include "vbiinput.h"

// AleVT functions
extern "C"
{
  // Enable/Disable debugging
  int debug = 0;

  // Cache
  void *cache_open(void);

  // VBI
  void *vbi_open(const char *vbi_dev_name, void *ca, int fine_tune, int big_buf);
  int  vbi_add_handler(void *vbi, void *handler, void *data);
  void *vbi_query_page(void *vbi, int pgno, int subno);
  void vbi_close(void *vbi);
  int vbi_fdset_select(void *vbi, int mstimeout);
}

namespace LXiStream {
namespace V4lBackend {

VBIInput::VBIInput(const QString &device, QObject *parent)
         :SNode(Behavior_Blocking, 0, SCodecList(), parent),
          device(device),
          running(false),
          quality(0.0),
          bitErrors(0),
          bitErrorCounter(0),
          bitErrorTimer(0),
          vbi(NULL),
          producedBuffers(NULL)
{
}

VBIInput::~VBIInput()
{
}

quint32 VBIInput::bitErrorRate(void) const
{
  return bitErrors;
}

qreal VBIInput::signalQuality(void) const
{
  return quality;
}

bool VBIInput::prepare(const SCodecList &)
{
  quality = 1.0;
  bitErrorTimer = startTimer(1000);

  vbi = vbi_open(device.toAscii(), NULL, 1, -1);
  if (vbi)
    vbi_add_handler(vbi, (void *)&vbiEvent, this);

  return true;
}

bool VBIInput::unprepare(void)
{
  killTimer(bitErrorTimer);

  if (vbi)
    vbi_close(vbi);

  return true;
}

SNode::Result VBIInput::processBuffer(const SBuffer &, SBufferList &output)
{
  producedBuffers = &output;

  // Process received buffers
  if (vbi)
  {
    QTime timer;
    timer.start();
    while ((vbi_fdset_select(vbi, SGraph::nonBlockingTaskTimeMs) > 0) &&
           (qAbs(timer.elapsed()) < int(SGraph::nonBlockingTaskTimeMs)) &&
           (producedBuffers->count() < 16))
      continue;
  }

  producedBuffers = NULL;
  return vbi ? Result_Active : Result_Idle;
}

void VBIInput::vbiEvent(VBIInput *me, Event *event)
{
  if (event)
  {
    Q_ASSERT(me->producedBuffers);

    if (event->type == 5) // Page
    {
      SDataBuffer buffer(sizeof(SDataBuffer::TeletextPage));
      Q_ASSERT(!buffer.isNull());

      Page *page = reinterpret_cast<Page *>(event->p1);

      memcpy(buffer.bits(), page, sizeof(SDataBuffer::TeletextPage));
      buffer.setCodec(SDataCodec::Format_TeletextPage);
      buffer.setNumBytes(sizeof(SDataBuffer::TeletextPage));
      buffer.setTimeStamp(me->timer.timeStamp());

      me->producedBuffers->append(buffer);

      page->lines = 0;

      // Compute the signal quality
      const qreal quality = 1.0 - qMin(qreal(page->errors) * 0.005, 1.0);
      me->quality = (me->quality * 0.9) + (quality * 0.1);
      me->bitErrorCounter += page->errors;

      if (page->errors > 0)
        page->errors = 0;
    }
    else if (event->type == 7) // XPacket
    {
      SDataBuffer buffer(sizeof(SDataBuffer::TeletextXPacket));
      Q_ASSERT(!buffer.isNull());

      memcpy(buffer.bits(), event->p1, sizeof(SDataBuffer::TeletextXPacket));
      buffer.setCodec(SDataCodec::Format_TeletextXPacket);
      buffer.setNumBytes(sizeof(SDataBuffer::TeletextXPacket));
      buffer.setTimeStamp(me->timer.timeStamp());

      me->producedBuffers->append(buffer);
    }
  }
}

void VBIInput::timerEvent(QTimerEvent *)
{
  SDebug::Trace t("VBIInput::timerEvent");

  bitErrors = bitErrorCounter;
  bitErrorCounter -= bitErrors;
}


} } // End of namespaces
