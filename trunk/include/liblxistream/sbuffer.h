/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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

#ifndef LXISTREAM_SBUFFER_H
#define LXISTREAM_SBUFFER_H

#include <QtCore>
#include <LXiCore>
#include "export.h"

namespace LXiStream {

/*! This class represents a generic buffer.

    An SBuffer class manages an implicit sharing prointer to an instance of an
    SBuffer::Memory object. These objects are usually constructed by the SBuffer
    internally, but can be allocated manually to let the SBuffer manage external
    memory (e.g. buffers allocated by a capture device). This means that
    multiple buffers can point to the same memory, an implicit copy is made when
    more than one SBuffer instances point to the same memory and the data is
    accessed by a non-const method on one of the instances.

    The term capacity describes the total amount of memory that can be used by
    the buffer, the term size describes the actually used memory. For example,
    a buffer can have a capacity of 65536 bytes and have a size of 46732 bytes,
    appending 4096 bytes can occur without copying the data to a new, larger,
    block of memory.

    Usually the SBuffer class is not used directly, but a more specific one like
    SAudioBuffer, SVideoBuffer, SEncodedAudioBuffer or SEncodedVideoBuffer.
 */
class LXISTREAM_PUBLIC SBuffer
{
public:
  /*! This class manages a block of memory. This class is usually only used by
      SBuffer iternally and only needs to be instantiated manually when a
      specific block of memory needs to be managed by an SBuffer (e.g. a
      buffer from a capture device).
   */
  class LXISTREAM_PUBLIC Memory : public QSharedData
  {
  friend class SBuffer;
  public:
                                Memory(int capacity, char *data, int size);
                                Memory(const Memory &);
    virtual                     ~Memory();

  private:
                                Memory(void);
    explicit                    Memory(int capacity);
                                Memory(const char *data, int size);
                                Memory(const QByteArray &data);

  public:
    /*! Unique identifier of the memory block. This ID can be used if two
        SBuffer instances point to the same memory block, do not use the data
        pointer for this as buffers may be re-used over time.
     */
    const int                   uid;

  private:
    const int                   capacity;
    char                * const data;
    const bool                  owner;
    int                         size;
  };

  typedef QSharedDataPointer<Memory> MemoryPtr;

public:
  inline                        SBuffer(void) : d(new Memory())                 { } //!< Constructs an empty buffer.
  inline                        SBuffer(const MemoryPtr &d) : d(d)              { } //!< Constructs a buffer with a pointer to a specific Memory object.
  inline explicit               SBuffer(int capacity) : d(new Memory(capacity)) { } //!< Constructs an empty buffer with the specified capacity.
  inline                        SBuffer(const char *data, int size) : d(new Memory(data, size)) { } //!< Constructs a buffer from the specified data.
  inline                        SBuffer(const QByteArray &data) : d(new Memory(data)) { } //!< Constructs a buffer from the specified data.
  inline                        SBuffer(const SBuffer &c) : d(c.d)              { }
  inline                        ~SBuffer(void)                                  { }

  inline                        operator QByteArray() const                     { return QByteArray::fromRawData(d->data, d->size); }
  inline SBuffer              & operator=(const QByteArray &data)               { d = new Memory(data); return *this; }

  void                          append(const char *data, int len);
  inline int                    capacity(void) const                            { return d->capacity; } //!< Returns the capacity of the buffer.
  inline void                   clear(void)                                     { d = new Memory(); } //!< Sets the buffer to a null buffer, the memory is released if no other buffers refer to it.
  inline const char           * constData(void) const                           { return d->data; } //!< Returns a pointer to the first byte of data.
  inline char                 * data(void)                                      { return d->data; } //!< Returns a pointer to the first byte of data, an implicit copy is made if more buffers refer to this memory.
  inline const char           * data(void) const                                { return d->data; } //!< Returns a pointer to the first byte of data.
  inline void                   setData(const char *data, int size)             { d = new Memory(data, size); } //!< Copies the data into a new buffer.
  inline void                   setData(const QByteArray &data)                 { d = new Memory(data); } //!< Copies the data into a new buffer.
  inline bool                   isEmpty(void) const                             { return d->size == 0; } //!< Returns true if the size of the buffer is 0.
  inline bool                   isNull(void) const                              { return d->capacity == 0; } //!< Returns true if the capacity of the buffer is 0.
  void                          reserve(int size);
  void                          resize(int size);
  inline int                    size(void) const                                { return d->size; } //!< Returns the size of the buffer.
  void                          squeeze(void);

  inline const MemoryPtr      & memory(void) const                              { return d; } //!< Return s a pointer to the Memory object used by this buffer.

public: // Alignment methods
  /*! This constant specifies the minimum value by which data should be aligned
      in memory to prevent crashes (e.g. 16 bytes for SSE).
   */
  static const int              minimumAlignVal = 16;

  /*! This constant specifies the optimal value by which data should be aligned
      in memory for best performance (e.g. the size of a cache line).
   */
  static const int              optimalAlignVal = 64;

  /*! This helper method aligns the specified buffer size to the optimal CPU
      alignment. The returned value is at least size and at most
      size + alignVal - 1.
   */
  static inline int             align(int size, int alignVal = optimalAlignVal)
  {
    return (size + alignVal - 1) & ~(alignVal - 1);
  }

  /*! This helper method aligns the specified buffer to the optimal CPU
      alignment. Please ensure that at least alignVal additional bytes is
      allocated, this should be two times alignVal if the buffer size is also
      not a multiple of alignVal.
   */
  template <typename _type>
  static inline _type         * align(_type *ptr, int alignVal = optimalAlignVal)
  {
    return (_type *)((intptr_t(ptr) + alignVal - 1) & ~intptr_t(alignVal - 1));
  }

  /*! This constant specifies the number of bytes padding that are appended to
      the buffer, these bytes are always set to 0. These bytes make implementing
      SIMD (e.g. SSE) reading simpler.

      \note The capacity for the buffer is already compensated for the padding.
            Thus if capacity() returns 2000, the actual allocated amount of
            memory is 2000 + numPaddingBytes.
   */
  static const int              numPaddingBytes = minimumAlignVal;

private:
  _lxi_internal static QAtomicInt uidCounter;

  MemoryPtr                     d;
};

typedef QList<SBuffer>          SBufferList;

} // End of namespace

#endif
