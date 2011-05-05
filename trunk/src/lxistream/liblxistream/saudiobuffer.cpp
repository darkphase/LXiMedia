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

#include "saudiobuffer.h"

namespace LXiStream {

/*! \class SAudioBuffer
    \brief The SAudioBuffer class provides access to raw audio samples.

    An SAudioBuffer can be used to store raw audio samples. It uses implicit
    sharing to avoid needless copying of data. The SAudioBuffer contains an
    SAudioFormat describing the actual type of raw data.

    \code
      SAudioBuffer inBuffer(SAudioFormat(SAudioFormat::Format_PCM_S16,
                                         SAudioFormat::Channel_Mono,
                                         8000),
                            2048);

      for (unsigned i=0; i<2048; i+=2)
      {
        reinterpret_cast<qint16 *>(inBuffer.data())[i]   = 32;
        reinterpret_cast<qint16 *>(inBuffer.data())[i+1] = -32;
      }
    \endcode

    \sa SAudioFormat
 */

/*! Constructs an SAudioBuffer with the specified format and number of samples.
    \sa setNumSamples()
 */
SAudioBuffer::SAudioBuffer(const SAudioFormat &format, unsigned numSamples)
{
  setFormat(format);
  setNumSamples(numSamples);
}

/*! Constructs an SAudioBuffer with the specified format and an implicitly
    shared buffer. This constructor can be used to wrap an external buffer.

    \code
      class Memory : public SBuffer::Memory
      {
      public:
        Memory(int capacity, char *data, int size, MyDevice *device)
          : SBuffer::Memory(capacity, data, size), device(device)
        {
        }

        virtual ~Memory()
        {
          device->release(data);
        }

      private:
        MyDevice * const device;
      };
    \endcode
 */
SAudioBuffer::SAudioBuffer(const SAudioFormat &format, const MemoryPtr &memory)
  : SBuffer(memory)
{
  d.format = format;
}

/*! Concatenates all audio buffers from the specified list into a single buffer.
 */
SAudioBuffer::SAudioBuffer(const SAudioBufferList &from)
{
  SAudioBuffer::operator=(from);
}

/*! Concatenates all audio buffers from the specified list into a single buffer.
 */
SAudioBuffer & SAudioBuffer::operator=(const SAudioBufferList &from)
{
  if (!from.isEmpty())
  {
    int i;
    for (i=0; i<from.count(); i++)
    if (!from[i].isNull())
    {
      SAudioBuffer::operator=(from[i]);
      break;
    }

    for (i=i+1; i<from.count(); i++)
    if (!from[i].isNull())
    {
      if (format() == from[i].format())
        append(from[i].data(), from[i].size());
      else
        qWarning() << "SAudioBuffer: Cannot merge buffers with different formats.";
    }

    return *this;
  }
  else
  {
    clear();
    return *this;
  }
}

/*! \fn const SAudioFormat & SAudioBuffer::format() const

    Returns the sample format of the buffer.
 */

/*! Sets the sample format of the buffer.
 */
void SAudioBuffer::setFormat(const SAudioFormat &format)
{
  d.format = format;
}

/*! \fn STime SAudioBuffer::timeStamp() const

    Returns the timestamp of the buffer.
 */

/*! \fn void SAudioBuffer::setTimeStamp(STime) const

    Sets the timestamp of the buffer.
 */

/*! Computes the number of samples in this buffer by dividing numBytes() by
    format().sampleSize() * format().numChannels().
 */
unsigned SAudioBuffer::numSamples(void) const
{
  Q_ASSERT((d.format.sampleSize() > 0) && (d.format.numChannels() > 0));

  if ((size() >= 0) && (d.format.sampleSize() > 0) && (d.format.numChannels() > 0))
    return size() / (d.format.sampleSize() * d.format.numChannels());

  return 0;
}

/*! Sets the number of samples in this buffer by multiplying the argument by
    format().sampleSize() * format().numChannels().
 */
void SAudioBuffer::setNumSamples(unsigned s)
{
  Q_ASSERT((d.format.sampleSize() > 0) && (d.format.numChannels() > 0));

  resize(d.format.sampleSize() * d.format.numChannels() * s);
}

/*! Returns the duration of the buffer.
 */
STime SAudioBuffer::duration(void) const
{
  if (d.format.sampleRate() > 0)
    return STime::fromClock(numSamples(), d.format.sampleRate());

  return STime();
}

/*! Returns an SAudioBuffer with only the samples of the specified channel.
 */
SAudioBuffer SAudioBuffer::getChannel(SAudioFormat::Channel channel) const
{
  const int channelPos = format().channelPos(channel);

  if (channelPos >= 0)
  {
    SAudioFormat dstFormat(format());
    dstFormat.setChannelSetup(channel);

    const unsigned count = numSamples();
    SAudioBuffer dstBuffer(dstFormat, count);

    const char * const srcData = data();
    char * const dstData = dstBuffer.data();

    const unsigned sampleSize = format().sampleSize();
    const unsigned sampleIncr = format().numChannels() * sampleSize;
    for (unsigned i=0, r=channelPos*sampleSize, w=0;
         i<count;
         i++, r+=sampleIncr, w+=sampleSize)
    {
      memcpy(dstData + w, srcData + r, sampleSize);
    }

    return dstBuffer;
  }
  else
    return SAudioBuffer();
}

/*! Extracts the samples of the specified channel from the specified buffer and
    sets them as the samples for that channel for this buffer.
 */
void SAudioBuffer::setChannel(const SAudioBuffer &buffer, SAudioFormat::Channel channel)
{
  const int srcChannelPos = buffer.format().channelPos(channel);
  const int dstChannelPos = format().channelPos(channel);

  if ((srcChannelPos >= 0) && (dstChannelPos >= 0) &&
      (buffer.format().format() == format().format()))
  {
    const unsigned count = qMin(buffer.numSamples(), numSamples());
    const char * const srcData = buffer.data();
    char * const dstData = data();

    const unsigned sampleSize = format().sampleSize();
    const unsigned srcSampleIncr = buffer.format().numChannels() * sampleSize;
    const unsigned dstSampleIncr = format().numChannels() * sampleSize;
    for (unsigned i=0, w=dstChannelPos*sampleSize, r=srcChannelPos*sampleSize;
         i<count;
         i++, w+=dstSampleIncr, r+=srcSampleIncr)
    {
      memcpy(dstData + w, srcData + r, sampleSize);
    }
  }
}

/*! Extracts the samples for all channels from the specified buffer and
    sets them as the samples for those channel for this buffer.
 */
void SAudioBuffer::setChannels(const SAudioBuffer &buffer)
{
  for (int i=0; i<32; i++)
  {
    const quint32 c = quint32(1) << i;
    if (((format().channelSetup() & c) != 0) && ((buffer.format().channelSetup() & c) != 0))
      setChannel(buffer, SAudioFormat::Channel(c));
  }
}

} // End of namespace
