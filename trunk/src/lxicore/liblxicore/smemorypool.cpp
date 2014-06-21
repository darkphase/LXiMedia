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

#include "smemorypool.h"
#include "sapplication.h"

namespace LXiCore {

struct SMemoryPool::Init : SApplication::Initializer
{
  virtual void startup(void)
  {
    // Ensure static initializers are called.
    QMutexLocker l(mutex());
    freePool().clear();
    freeQueue().clear();

    allocSize = 0;
  }

  virtual void shutdown(void)
  {
    QMutexLocker l(mutex());

    if (allocSize > 0)
      qWarning() << "SMemoryPool: Not all memory was freed; leaked" << (allocSize / 1024) << "KiB.";

    foreach (Header *header, freePool().values())
      freeMemory(header);

    freePool().clear();
    freeQueue().clear();
  }
};

SMemoryPool::Init SMemoryPool::init;
const int         SMemoryPool::addrAlign = 128;
const int         SMemoryPool::sizeAlign = 4096;
int               SMemoryPool::maxFreeCount = 128;
size_t            SMemoryPool::allocSize = 0;

/*! Returns the size of the specified pool.
 */
size_t SMemoryPool::poolSize(Pool pool)
{
  if (pool == Pool_Free)
  {
    QMutexLocker l(mutex());

    size_t result = 0;
    foreach (const Header *header, freePool())
      result += header[-1].size;

    return result;
  }
  else if (pool == Pool_Alloc)
    return allocSize;

  return 0;
}

/*! Aligns the specified address.
 */
quintptr SMemoryPool::alignAddr(quintptr size)
{
  return ((size + (addrAlign - 1)) / addrAlign) * addrAlign;
}

/*! Aligns the specified size to a multiple that can be allocated by the pool.
 */
size_t SMemoryPool::alignSize(size_t size)
{
  return ((size + (sizeAlign - 1)) / sizeAlign) * sizeAlign;
}

/*! Returns a buffer of the specified size, either from the pool or newly
    allocated. The specified size needs to be aligned using align().

    \note Memory is not reset to zero.
 */
void * SMemoryPool::alloc(size_t size)
{
  size = alignSize(size);
  if (size > 0)
  {
    QMutexLocker l(mutex());

    QMultiMap<size_t, Header *>::Iterator i = freePool().find(size);
    if (i != freePool().end())
    {
      Header * const result = *i;

      for (QList<size_t>::Iterator j=freeQueue().begin(); j!=freeQueue().end(); j++)
      if (*j == i.key())
      {
        freeQueue().erase(j);
        break;
      }

      allocSize += result[-1].size;
      freePool().erase(i);

      Q_ASSERT(freePool().count() == freeQueue().count());
      return result;
    }
    else // Allocate new
    {
      Header * const result = allocMemory(size);
      allocSize += result[-1].size;
      return result;
    }
  }

  return NULL;
}

/*! Returns the specified buffer back into the pool. All memory in the pool is
    freed if this was the last allocated buffer.
 */
void SMemoryPool::free(void *ptr)
{
  if (ptr)
  {
    Header * const header = reinterpret_cast<Header *>(ptr);

    QMutexLocker l(mutex());

    freePool().insert(header[-1].size, header);
    freeQueue().prepend(header[-1].size);

    Q_ASSERT(allocSize >= header[-1].size);
    allocSize -= header[-1].size;

    // Flush all free buffers if there are no more buffers in use.
    if (allocSize == 0)
    {
      size_t size = 0;
      foreach (Header *header, freePool().values())
      {
        size += header[-1].size;
        freeMemory(header);
      }

      freePool().clear();
      freeQueue().clear();

//      qDebug() << "SMemoryPool: flushed free pool of" << (size / 1024) << "KiB";
    }

    while (freeQueue().count() > maxFreeCount)
    {
      QMultiMap<size_t, Header *>::Iterator i = freePool().find(freeQueue().takeLast());
      if (i != freePool().end())
      {
        freeMemory(*i);
        freePool().erase(i);
      }
    }

    Q_ASSERT(freePool().count() == freeQueue().count());
  }
}

SMemoryPool::Header * SMemoryPool::allocMemory(size_t size)
{
  quint8 * const buffer = new quint8[sizeof(Header) + size + addrAlign];
  Header * const result = (Header *)alignAddr(quintptr(buffer + sizeof(Header)));
  result[-1].addr = buffer;
  result[-1].size = size;

  return result;
}

void SMemoryPool::freeMemory(Header *addr)
{
  delete [] addr[-1].addr;
}

QMutex * SMemoryPool::mutex(void)
{
  static QMutex m;

  return &m;
}

QMultiMap<size_t, SMemoryPool::Header *> & SMemoryPool::freePool(void)
{
  static QMultiMap<size_t, Header *> p;

  return p;
}

QList<size_t> & SMemoryPool::freeQueue(void)
{
  static QList<size_t> q;

  return q;
}

} // End of namespace
