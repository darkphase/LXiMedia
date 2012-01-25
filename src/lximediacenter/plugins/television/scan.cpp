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

#include "scan.h"
#include <cmath>
#include "televisionserver.h"

namespace LXiMediaCenter {

Scan::Scan(STerminal *terminal, const QString &devName, quint64 frequency, QObject *parent)
     :QObject(parent),
      devName(devName),
      graph(SGraph::MediaTask_None),
      terminal(terminal),
      tuner(terminal->tuner()),
      captureNode(NULL),
      scanNode(this),
      frequency(frequency),
      lastFrequency(0),
      startFrequency(0),
      stopFrequency(0),
      stepFrequency(0),
      bigStepFrequency(0),
      scanTimer(-1),
      signalTimer(-1),
      mode(Coarse),
      dvb(false),
      graphStarted(false),
      fineBeginFrequency(0),
      nameSettleTime(0)
{
  if (qobject_cast<SAnalogTuner *>(tuner))
  {
    captureNode = graph.openStream(terminal, terminal->inputStream(0));
    graph.registerNode(&scanNode);
    graph.connectNodes(captureNode, &scanNode);

    if (graph.prepare())
    {
      graph.start();

      if (tuner->frequencyInfo(startFrequency, stopFrequency, stepFrequency))
      {
        if (this->frequency == 0)
        {
          if (tuner->setFrequency(startFrequency))
          {
            bigStepFrequency = stepFrequency * 4;
            this->frequency = startFrequency;
            scanTimer = startTimer(500);
            signalTimer = startTimer(250);
            dvb = false;
            graphStarted = true;

            return;
          }
        }
        else
        {
          if (tuner->setFrequency(this->frequency))
          {
            signalTimer = startTimer(250);
            dvb = false;
            graphStarted = true;

            return;
          }
        }
      }

      graph.unprepare();
    }
  }
  else if (qobject_cast<SDigitalTuner *>(tuner))
  {
    if (tuner->frequencyInfo(startFrequency, stopFrequency, stepFrequency))
    {
      if (this->frequency == 0)
      {
        if (tuner->setFrequency(startFrequency))
        {
          bigStepFrequency = stepFrequency * 4;
          this->frequency = startFrequency;
          scanTimer = startTimer(1000);
          signalTimer = startTimer(250);
          dvb = true;

          return;
        }
      }
      else
      {
        if (tuner->setFrequency(this->frequency))
        {
          signalTimer = startTimer(250);
          dvb = true;

          return;
        }
      }
    }
  }
}

Scan::~Scan()
{
  stop();
}

void Scan::timerEvent(QTimerEvent *e)
{
  if (e->timerId() == scanTimer)
  {
    SDebug::Trace t("Scan::timerEvent(scanTimer)");
    SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

    if (mode == Coarse)
    {
      if (tuner->signalStatus().hasSignal)
      {
        frequency -= bigStepFrequency - stepFrequency;
        fineBeginFrequency = frequency;
        mode = FineIn;
      }
    }
    else if (mode == FineIn)
    {
      if (tuner->signalStatus().hasLock)
      {
        fineBeginFrequency = frequency;
        mode = FineOut;
      }
    }
    else if (mode == FineOut)
    {
      if (!tuner->signalStatus().hasLock)
      {
        frequency = ((((frequency - fineBeginFrequency) / 2) + fineBeginFrequency) / stepFrequency) * stepFrequency;
        nameSettleTime = 20;
        channelName = "";
        programmeName = "";
        mode = Name;
      }
    }

    if (mode == Coarse)
    {
      frequency += bigStepFrequency;
    }
    else if ((mode == FineIn) || (mode == FineOut))
    {
      if (frequency - fineBeginFrequency > (bigStepFrequency * 8))
      {
        frequency = ((frequency + (bigStepFrequency * 2)) / bigStepFrequency) * bigStepFrequency;
        mode = Coarse;
      }
      else
        frequency += stepFrequency;
    }
    else if ((mode == Name) && (--nameSettleTime == 0))
    {
      parseChannels();
      mode = Coarse;
      frequency = ((frequency + (bigStepFrequency * 2)) / bigStepFrequency) * bigStepFrequency;
    }

    if (frequency != lastFrequency)
    {
      if ((frequency > stopFrequency) || !tuner->setFrequency(frequency))
        stop();
      else
        lastFrequency = frequency;
    }
  }
  else if (e->timerId() == signalTimer)
  {
    SDebug::Trace t("Scan::timerEvent(signalTimer)");
    SDebug::WriteLocker l(&lock, __FILE__, __LINE__);

    const STuner::Status status = tuner->signalStatus();
    statusHistory.enqueue(status);
    if (statusHistory.count() > 176)
      statusHistory.dequeue();

    if (status.hasLock && (scanTimer == -1))
    {
      channelsFound.clear();
      parseChannels();
    }
  }
}

void Scan::parseChannels(void)
{
  if (dvb == false)
  {
    if (channelName.length() == 0)
    {
      if (programmeName.length() > 0)
        channelName = programmeName;
      else
        channelName = QString::number(frequency / Q_UINT64_C(1000)) + " kHz";
    }

    channelsFound += Channel(Type_Television, frequency, 0, channelName, programmeName, snapshot.isNull() ? QImage() : snapshot.toImage(true));
  }
  else
  {
    foreach (const STerminal::Stream &stream, terminal->inputStreams())
    {
      Type type;
      if (!stream.videoPacketIDs.isEmpty())
        type = Type_Television;
      else if (!stream.audioPacketIDs.isEmpty())
        type = Type_Radio;
      else
        continue;

      QString programme = typeName(type);
      if (!stream.dataPacketIDs.isEmpty())
        programme += ", TT";

      programme += ", " + stream.provider;

      channelsFound += Channel(type, frequency, stream.serviceID, stream.name, programme, QImage());
      providerName = stream.provider;
    }
  }
}

void Scan::stop(void)
{
  if (scanTimer != -1)
  {
    killTimer(scanTimer);
    scanTimer = -1;
  }

  if (signalTimer != -1)
  {
    killTimer(signalTimer);
    scanTimer = -1;
  }

  if (graphStarted)
  {
    graph.stop();
    graph.unprepare();
    graphStarted = false;
  }
}

QImage Scan::lastSnapshot(void) const
{
  SDebug::ReadLocker l(&lock, __FILE__, __LINE__);

  if (!snapshot.isNull())
    return snapshot.toImage();
  else
    return QImage();
}

QImage Scan::signalPlot(void) const
{
  static const int w = 352, h = 144;
  QImage image(w, h, QImage::Format_RGB32);
  QPainter p(&image);

  p.fillRect(image.rect(), Qt::white);

  SDebug::ReadLocker l(&lock, __FILE__, __LINE__);

  if (!statusHistory.isEmpty())
  {
    int pos = 1;
    foreach (const STuner::Status &status, statusHistory)
    {
      p.setPen(QPen(Qt::darkGreen, 2.0));
      p.drawLine(pos, h - 1, pos, h - int(status.signalStrength * qreal(h)));
      p.setPen(QPen(Qt::blue, 2.0));
      p.drawPoint(pos, h - int(status.signalNoiseRatio * qreal(h)));
      p.setPen(QPen(Qt::red, 2.0));
      p.drawPoint(pos, h - int(log(status.bitErrorRate) / log(65536) * qreal(h)));

      pos += 2;
    }

    for (int i=0; i<4; i++)
    {
      const bool on = reinterpret_cast<const bool *>(&(statusHistory.last().hasSignal))[i];

      p.setPen(QPen(Qt::black, 2.0));
      p.setBrush(on ? Qt::green : Qt::gray);
      p.drawEllipse(10 + (i * 16), 10, 10, 10);
    }
  }

  return image;
}


SNode::Result Scan::ScanNode::processBuffer(const SBuffer &input, SBufferList &)
{
  SDebug::WriteLocker l(&(parent->lock), __FILE__, __LINE__);

  if (input.typeId() == SVideoBuffer::baseTypeId)
  {
    if (!input.isNull() && !input.codec().isCompressed())
      parent->snapshot = input;
  }
  else if (input.typeId() == SDataBuffer::baseTypeId)
  {
    const SDataBuffer dataBuffer = input;

    if (dataBuffer.codec() == SDataCodec::Format_TeletextPage)
    {
      const SDataBuffer::TeletextPage * const page =
          reinterpret_cast<const SDataBuffer::TeletextPage *>(dataBuffer.bits());

      parent->channelName = page->channelName();
    }
    else if (dataBuffer.codec() == SDataCodec::Format_TeletextXPacket)
    {
      const SDataBuffer::TeletextXPacket * const xPacket =
          reinterpret_cast<const SDataBuffer::TeletextXPacket *>(dataBuffer.bits());

      parent->programmeName = QString::fromAscii(xPacket->programmeName, sizeof(xPacket->programmeName)).trimmed();
    }
  }

  return Result_Active;
}

} // End of namespace
