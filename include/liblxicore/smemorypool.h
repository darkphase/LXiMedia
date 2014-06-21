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

#ifndef LXICORE_SMEMORYPOOL_H
#define LXICORE_SMEMORYPOOL_H

#include <QtCore>
#include "export.h"

namespace LXiCore {

/*! The SMemoryPool singleton provides a fast memory allocator for large
    buffers.
 */
class LXICORE_PUBLIC SMemoryPool
{
private:
  struct Header
  {
    quint8                    * addr;
    size_t                      size;
  };

public:
  enum Pool { Pool_Free, Pool_Alloc };

private: // No instances
                                SMemoryPool();
                                SMemoryPool(const SMemoryPool &);
  SMemoryPool                 & operator=(const SMemoryPool &);

public:
  static size_t                 poolSize(Pool);
  static quintptr               alignAddr(quintptr);
  static size_t                 alignSize(size_t);
  static void                 * alloc(size_t);
  static void                   free(void *);

private:
  static Header * allocMemory(size_t);
  static void     freeMemory(Header *);

private:
  struct Init;
  static Init                   init;
  static const int              addrAlign;
  static const int              sizeAlign;
  static int                    maxFreeCount;
  static size_t                 allocSize;

  static QMutex               * mutex(void);
  static QMultiMap<size_t, Header *> & freePool(void);
  static QList<size_t>        & freeQueue(void);
};

} // End of namespace

#endif
