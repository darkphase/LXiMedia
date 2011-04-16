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

#include "smemorypool.h"
#include "sapplication.h"
#if defined(Q_OS_UNIX)
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

#ifndef QT_NO_DEBUG
#define USE_GUARD_PAGES
#endif

namespace LXiCore {

struct SMemoryPool::Init : SApplication::Initializer
{
  virtual void startup(void)
  {
#if defined(Q_OS_UNIX)
    pageSize = int(::sysconf(_SC_PAGESIZE));
#endif

    if (pageSize <= 0)
      pageSize = 8192;

    zeroDev = ::open("/dev/zero", O_RDWR);

    // Ensure static initializers are called.
    QMutexLocker l(mutex());
    freePool().clear();
    allocPool().clear();
  }

  virtual void shutdown(void)
  {
    QMutexLocker l(mutex());

    if (!allocPool().isEmpty())
    {
      qWarning() << "SMemoryPool: Not all memory was freed; leaked" << allocPool().count() << "buffers.";
      foreach (const Block &block, freePool().values())
        freePages(block.addr, block.size);
    }

    foreach (const Block &block, freePool().values())
      freePages(block.addr, block.size);

    freePool().clear();
    allocPool().clear();

    ::close(zeroDev);
    zeroDev = 0;
    pageSize = 0;
  }
};

SMemoryPool::Init SMemoryPool::init;
int               SMemoryPool::pageSize = 0;
#if defined(Q_OS_UNIX)
  int             SMemoryPool::zeroDev = 0;
#endif

size_t SMemoryPool::poolSize(Pool pool)
{
  QMutexLocker l(mutex());

  size_t result = 0;

  if (pool == Pool_Free)
  {
    foreach (const Block &block, freePool())
      result += block.size;
  }
  else if (pool == Pool_Alloc)
  {
    foreach (const Block &block, allocPool())
      result += block.size;
  }

  return result;
}

size_t SMemoryPool::align(size_t size)
{
  Q_ASSERT(pageSize);

  return ((size + (pageSize - 1)) / pageSize) * pageSize;
}

void * SMemoryPool::alloc(size_t size)
{
  size = align(size);
  if (size > 0)
  {
    QMutexLocker l(mutex());

    QMultiMap<size_t, Block>::Iterator i = freePool().lowerBound(size);
    if ((i != freePool().end()) && (i.key() <= (size + (size / 2))))
    {
      void * const result = i->addr;
      allocPool().insert(result, *i);
      freePool().erase(i);
      return result;
    }
    else // Allocate new
    {
      void * const result = allocPages(size);
      allocPool().insert(result, Block(result, size));
      return result;
    }
  }

  return NULL;
}

void SMemoryPool::free(void *ptr)
{
  if (ptr)
  {
    QMutexLocker l(mutex());

    QHash<void *, Block>::Iterator i = allocPool().find(ptr);
    if (i != allocPool().end())
    {
      freePool().insert(i->size, *i);
      allocPool().erase(i);
    }
    else
      qFatal("SMemoryPool: Could not free buffer %p", ptr);

    // Flush all free buffers if there are no more buffers in use.
    if (allocPool().isEmpty())
    {
#ifndef QT_NO_DEBUG
      size_t size = 0;
      foreach (const Block &block, freePool())
        size += block.size;

      qDebug() << "SMemoryPool: flushing free pool of" << (size / 1048576) << "MiB";
#endif

      foreach (const Block &block, freePool().values())
        freePages(block.addr, block.size);

      freePool().clear();
    }
  }
}

void * SMemoryPool::allocPages(size_t size)
{
  Q_ASSERT(pageSize);

#if defined(Q_OS_UNIX)
# ifndef USE_GUARD_PAGES
  void * const result =
      ::mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, zeroDev, 0);
# else
  quint8 * const result = reinterpret_cast<quint8 *>(
      ::mmap(0, size + (pageSize * 2), PROT_READ | PROT_WRITE, MAP_PRIVATE, zeroDev, 0));
# endif

  if (result != MAP_FAILED)
  {
# ifndef USE_GUARD_PAGES
    return result;
# else
    // Guard pages
    ::mprotect(result, pageSize, PROT_NONE);
    ::mprotect(result + pageSize + size, pageSize, PROT_NONE);
    return result + pageSize;
# endif
  }
  else
    qFatal("SMemoryPool: Failed to allocate a block of size %u", unsigned(size));
#endif

  return NULL;
}

void SMemoryPool::freePages(void *addr, size_t size)
{
  Q_ASSERT(pageSize);

#if defined(Q_OS_UNIX)
# ifndef USE_GUARD_PAGES
  ::munmap(i->addr, i->size);
# else
  ::munmap(reinterpret_cast<quint8 *>(addr) - pageSize, size + (pageSize * 2));
# endif
#endif
}

QMutex * SMemoryPool::mutex(void)
{
  static QMutex m;

  return &m;
}

QMultiMap<size_t, SMemoryPool::Block> & SMemoryPool::freePool(void)
{
  static QMultiMap<size_t, Block> p;

  return p;
}

QHash<void *, SMemoryPool::Block> & SMemoryPool::allocPool(void)
{
  static QHash<void *, Block> p;

  return p;
}

} // End of namespace
