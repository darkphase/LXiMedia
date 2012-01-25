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

#include "svumeter.h"
#include <cmath>

namespace LXiStreamGui {

using namespace LXiStream;


const qreal SVuMeter::maxRms = 1.0 / sqrt(2.0);

SVuMeter::SVuMeter(QWidget *parent)
         :QFrame(parent),
          mutex(QMutex::Recursive),
          slowUpdate(false),
          updateTimer(-1),
          needsUpdate(false),
          myRect(rect().adjusted(frameWidth(), frameWidth(), -frameWidth(), -frameWidth()))
{
  setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  setMinimumHeight((frameWidth() * 2) + 2 + (5 * 3) + 2); // 6 bars with each 2 pixels and a separator pixel
}

SVuMeter::~SVuMeter()
{
  Q_ASSERT(rms.isEmpty());
}

SAudioFormat SVuMeter::inputFormat(void) const
{
  QMutexLocker l(&mutex);

  return format;
}

void SVuMeter::input(const SAudioBuffer &audioBuffer)
{
  if (!audioBuffer.isNull())
  if (audioBuffer.format() == SAudioFormat::Format_PCM_S16)
  {
    const unsigned channels = audioBuffer.format().numChannels();
    const unsigned numSamples = audioBuffer.numSamples();

    if (numSamples > 0)
    {
      const qint16 * const samples = reinterpret_cast<const qint16 *>(audioBuffer.data());

      // Ensure enough buffers are available
      QVector<qreal> rms, avg;
      for (unsigned i=0; i<channels; i++)
      {
        rms.append(0.0);
        avg.append(0.0);
      }

      // Compute average value (to compensate for a DC offset)
      for (unsigned i=0; i<numSamples; i++)
      for (unsigned j=0; j<channels; j++)
        avg[j] += qreal(samples[i * channels + j]);

      for (unsigned i=0; i<channels; i++)
        avg[i] /= qreal(numSamples);

      // And compute RMS (root-mean-square)
      for (unsigned i=0; i<numSamples; i++)
      for (unsigned j=0; j<channels; j++)
      {
        const qreal s = qreal(samples[i * channels + j]) - avg[j];
        rms[j] += s * s;
      }

      for (unsigned i=0; i<channels; i++)
        rms[i] = sqrt(rms[i] / qreal(numSamples)) / 32768.0;

      // Update peaks and store latest RMS values.
      QMutexLocker l(&mutex);

      while (peaks.count() < int(channels))
        peaks.append(QQueue<qreal>());

      while (peaks.count() > int(channels))
        peaks.pop_back();

      for (unsigned i=0; i<channels; i++)
      {
        peaks[i].enqueue(rms[i]);
        while (peaks[i].count() > 25)
          peaks[i].dequeue();
      }

      rms = rms;
      format = audioBuffer.format();
      needsUpdate = true;
    }
  }
}

void SVuMeter::paintEvent(QPaintEvent *e)
{
  needsUpdate = false;

  QFrame::paintEvent(e);

  myRect = rect().adjusted(frameWidth(), frameWidth(), -frameWidth(), -frameWidth());
  QPainter p(this);
  p.setCompositionMode(QPainter::CompositionMode_Source); // Ignore alpha

  foreach (const QRect &rect, e->region().rects())
    p.fillRect(myRect & rect, palette().button());

  if (!lastRms.isEmpty())
  {
    const int ch = myRect.height() / lastRms.count();

    for (int i=0; i<lastRms.count(); i++)
    {
      const qreal linRms = 1.0 - pow(1.0 - (lastRms[i] / maxRms), 10);
      const QRect bar(myRect.left(),
                      myRect.top() + (i * ch) + 1,
                      qMin(myRect.width() - 1, int(qreal(myRect.width()) * linRms)),
                      ch - 1);

      foreach (const QRect &rect, e->region().rects())
        p.fillRect(bar & rect, palette().highlight());

      qreal peak = 0.0;
      foreach (qreal v, lastPeaks[i])
        peak = qMax(peak, v);

      const qreal linPeak = 1.0 - pow(1.0 - (peak / maxRms), 10);
      const int po = qMin(myRect.width() - 1, int(qreal(myRect.width()) * linPeak));
        p.fillRect(QRect(myRect.left() + qBound(0, po, myRect.width() - 2),
                         bar.top(),
                         linPeak < 0.98 ? 2 : 4,
                         ch - 1),
                 linPeak < 0.98 ? palette().text() : QBrush(Qt::red));
    }
  }
}

void SVuMeter::timerEvent(QTimerEvent *)
{
  if (needsUpdate)
  {
    QMutexLocker l(&mutex);

    if (lastRms.isEmpty() || rms.isEmpty())
    {
      update();
    }
    else // Determine which part to update
    {
      int left = width(), right = 0;
      foreach (int x, determinePos(lastRms, lastPeaks) + determinePos(rms, peaks))
      {
        left = qMin(x, left);
        right = qMax(x, right);
      }

      left = qMax(0, left - 4);
      right = qMin(width() - 1, right + 4);
      update(QRect(left, 0, right - left, height()));
    }

    lastRms = rms;
    lastPeaks = peaks;
  }
}

QVector<int> SVuMeter::determinePos(const QVector<qreal> &rms, const QVector< QQueue<qreal> > &peaks) const
{
  QVector<int> result;

  if (!rms.isEmpty())
  for (int i=0; i<rms.count(); i++)
  {
    const qreal linRms = 1.0 - pow(1.0 - (rms[i] / maxRms), 10);
    result += myRect.left() + qMin(myRect.width() - 1, int(qreal(myRect.width()) * linRms));

    qreal peak = 0.0;
    foreach (qreal v, peaks[i])
      peak = qMax(peak, v);

    const qreal linPeak = 1.0 - pow(1.0 - (peak / maxRms), 10);
    result += myRect.left() + qMin(myRect.width() - 1, int(qreal(myRect.width()) * linPeak));
  }

  return result;
}


} // End of namespace
