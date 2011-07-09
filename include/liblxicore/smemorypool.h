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

#ifndef LXICORE_SMEMORYPOOL_H
#define LXICORE_SMEMORYPOOL_H

#include <QtCore>
#include "splatform.h"
#include "export.h"

namespace LXiCore {

/*! The SMemoryPool singleton provides a fast memory allocator for large
    buffers.
 */
class LXICORE_PUBLIC SMemoryPool
{
private:
  struct Block
  {
    inline                      Block(void *addr, size_t size) : addr(addr), size(size) { }

    void                      * addr;
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
  static size_t                 align(size_t);
  static void                 * alloc(size_t);
  static void                   free(void *);

private:
  _lxi_internal static void   * allocPages(size_t);
  _lxi_internal static void     freePages(void *, size_t);

private:
  struct Init;
  _lxi_internal static Init     init;
  _lxi_internal static int      pageSize;
  _lxi_internal static int      maxFreeCount;
#if defined(Q_OS_UNIX)
  _lxi_internal static int      zeroDev;
#endif

  _lxi_pure _lxi_internal static QMutex * mutex(void);
  _lxi_pure _lxi_internal static QMultiMap<size_t, Block> & freePool(void);
  _lxi_pure _lxi_internal static QList<size_t> & freeQueue(void);
  _lxi_pure _lxi_internal static QHash<void *, Block> & allocPool(void);
};

} // End of namespace

#endif
