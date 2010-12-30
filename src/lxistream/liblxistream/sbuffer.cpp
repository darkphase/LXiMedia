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

#include "sbuffer.h"

namespace LXiStream {

QAtomicInt  SBuffer::uidCounter(1);
const int   SBuffer::minimumAlignVal = 16;
const int   SBuffer::optimalAlignVal = 64;
const int   SBuffer::numPaddingBytes = minimumAlignVal;

/*! Appends the specified data to the existing buffer, the buffer is resized if
    the capacity is not sufficient.
 */
void SBuffer::append(const char *data, int len)
{
  if (len > 0)
  {
    if (d->size + len <= d->capacity)
    {
      memcpy(d->data + d->size, data, len);
      memset(d->data + d->size + len, 0, numPaddingBytes);
      d->size += len;
    }
    else
    {
      Memory * const newMemory = new Memory(((d->size + len + 4095) / 4096) * 4096);
      memcpy(newMemory->data, d->data, d->size);
      memcpy(newMemory->data + d->size, data, len);
      memset(newMemory->data + d->size + len, 0, numPaddingBytes);
      newMemory->size = d->size + len;

      d = newMemory;
    }
  }
}

/*! Ensures the capacity of the buffer is equal or larger than the specified
    size.
 */
void SBuffer::reserve(int size)
{
  if (d->capacity < size)
  {
    Memory * const newMemory = new Memory(size);
    memcpy(newMemory->data, d->data, d->size);
    memset(newMemory->data + d->size, 0, numPaddingBytes);
    newMemory->size = d->size;

    d = newMemory;
  }
}

/*! Sets the size of the buffer, the buffer is resized if the capacity is not
    sufficient.
 */
void SBuffer::resize(int size)
{
  if (d->capacity < size)
  {
    Memory * const newMemory = new Memory(size);
    memcpy(newMemory->data, d->data, qMin(d->size, size));
    memset(newMemory->data + qMin(d->size, size), 0, numPaddingBytes);
    newMemory->size = size;

    d = newMemory;
  }
  else
  {
    d->size = size;
    memset(d->data + size, 0, numPaddingBytes);
  }
}

/*! Reduces the capacity of the buffer such that it is just large enough to
    accomodate its data. Note that the entire buffer is copied into a new
    memory block if needed.
 */
void SBuffer::squeeze(void)
{
  if (d->size < d->capacity - optimalAlignVal)
    d = new Memory(d->data, d->size);
}


/*! Creates an Memory instance for the specified block of memory. The memory is
    not released when this class is destructed, reimplement the destructor to
    release the memory if needed.

    \param capacity The total capacity of the block of memory (including any
                    padding for alignment.
    \param data     A pointer to the first byte in the blovk.
    \param size     The actual size of the used part of the memory.
 */
SBuffer::Memory::Memory(int capacity, char *data, int size)
  : QSharedData(),
    uid((data != NULL) ? uidCounter.fetchAndAddRelaxed(1) : 0),
    capacity(capacity),
    size(size),
    unaligned(NULL),
    data(data)
{
}

SBuffer::Memory::Memory(const Memory &c)
  : QSharedData(c),
    uid(uidCounter.fetchAndAddRelaxed(1)),
    capacity(align(c.size)),
    size(c.size),
    unaligned(new char[this->capacity + optimalAlignVal + numPaddingBytes]),
    data(align(unaligned))
{
  memcpy(data, c.data, size);
  memset(data + size, 0, numPaddingBytes);
}

SBuffer::Memory::~Memory()
{
  delete [] unaligned;
}

SBuffer::Memory::Memory(void)
  : QSharedData(),
    uid(0),
    capacity(0),
    size(0),
    unaligned(NULL),
    data(NULL)
{
}

SBuffer::Memory::Memory(int capacity)
  : QSharedData(),
    uid(uidCounter.fetchAndAddRelaxed(1)),
    capacity(align(capacity)),
    size(0),
    unaligned(new char[this->capacity + optimalAlignVal + numPaddingBytes]),
    data(align(unaligned))
{
  memset(data, 0, numPaddingBytes);
}

SBuffer::Memory::Memory(const char *data, int size)
  : QSharedData(),
    uid(uidCounter.fetchAndAddRelaxed(1)),
    capacity(align(size)),
    size(size),
    unaligned(new char[this->capacity + optimalAlignVal + numPaddingBytes]),
    data(align(unaligned))
{
  memcpy(this->data, data, size);
  memset(this->data + size, 0, numPaddingBytes);
}

SBuffer::Memory::Memory(const QByteArray &data)
  : QSharedData(),
    uid(uidCounter.fetchAndAddRelaxed(1)),
    capacity(align(data.size())),
    size(data.size()),
    unaligned(new char[this->capacity + optimalAlignVal + numPaddingBytes]),
    data(align(unaligned))
{
  memcpy(this->data, data.data(), data.size());
  memset(this->data + data.size(), 0, numPaddingBytes);
}


} // End of namespace
