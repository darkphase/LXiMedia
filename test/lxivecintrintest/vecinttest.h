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

#include <QtCore>

class VecIntTest : public QObject
{
Q_OBJECT
public:
  inline explicit               VecIntTest(QObject *parent) : QObject(parent) { }

private slots:
  void                          Scalar_Int8(void);
  void                          Scalar_UInt8(void);
  void                          Scalar_Int16(void);
  void                          Scalar_UInt16(void);
  void                          Scalar_Int32(void);
  void                          Scalar_UInt32(void);
  void                          Scalar_Int64(void);
  void                          Scalar_UInt64(void);

  void                          Scalar_Int8_UInt8(void);
  void                          Scalar_Int8_Int16(void);
  void                          Scalar_UInt8_Int16(void);
  void                          Scalar_Int8_UInt16(void);
  void                          Scalar_UInt8_UInt16(void);
  void                          Scalar_Int8_Int32(void);
  void                          Scalar_UInt8_Int32(void);
  void                          Scalar_Int8_UInt32(void);
  void                          Scalar_UInt8_UInt32(void);
  void                          Scalar_Int8_Int64(void);
  void                          Scalar_UInt8_Int64(void);
  void                          Scalar_Int8_UInt64(void);
  void                          Scalar_UInt8_UInt64(void);

  void                          Scalar_Int16_UInt16(void);
  void                          Scalar_Int16_Int8(void);
  void                          Scalar_UInt16_Int8(void);
  void                          Scalar_Int16_UInt8(void);
  void                          Scalar_UInt16_UInt8(void);
  void                          Scalar_Int16_Int32(void);
  void                          Scalar_UInt16_Int32(void);
  void                          Scalar_Int16_UInt32(void);
  void                          Scalar_UInt16_UInt32(void);
  void                          Scalar_Int16_Int64(void);
  void                          Scalar_UInt16_Int64(void);
  void                          Scalar_Int16_UInt64(void);
  void                          Scalar_UInt16_UInt64(void);

  void                          Scalar_Int32_UInt32(void);
  void                          Scalar_Int32_Int8(void);
  void                          Scalar_UInt32_Int8(void);
  void                          Scalar_Int32_UInt8(void);
  void                          Scalar_UInt32_UInt8(void);
  void                          Scalar_Int32_Int16(void);
  void                          Scalar_UInt32_Int16(void);
  void                          Scalar_Int32_UInt16(void);
  void                          Scalar_UInt32_UInt16(void);
  void                          Scalar_Int32_Int64(void);
  void                          Scalar_UInt32_Int64(void);
  void                          Scalar_Int32_UInt64(void);
  void                          Scalar_UInt32_UInt64(void);

  void                          Scalar_Int64_UInt64(void);
  void                          Scalar_Int64_Int8(void);
  void                          Scalar_UInt64_Int8(void);
  void                          Scalar_Int64_UInt8(void);
  void                          Scalar_UInt64_UInt8(void);
  void                          Scalar_Int64_Int16(void);
  void                          Scalar_UInt64_Int16(void);
  void                          Scalar_Int64_UInt16(void);
  void                          Scalar_UInt64_UInt16(void);
  void                          Scalar_Int64_Int32(void);
  void                          Scalar_UInt64_Int32(void);
  void                          Scalar_Int64_UInt32(void);
  void                          Scalar_UInt64_UInt32(void);

private:
  template<class _vector> void  Scalar_test(void);
  template<class _dstvector, class _srcvector> void Scalar_convert(void);

#ifdef ENABLE_SSE
private slots:
  void                          SSE_Int8(void);
  void                          SSE_UInt8(void);
  void                          SSE_Int16(void);
  void                          SSE_UInt16(void);
  void                          SSE_Int32(void);
  void                          SSE_UInt32(void);
  void                          SSE_Int64(void);
  void                          SSE_UInt64(void);

  void                          SSE_Int8_UInt8(void);
  void                          SSE_Int8_Int16(void);
  void                          SSE_UInt8_Int16(void);
  void                          SSE_Int8_UInt16(void);
  void                          SSE_UInt8_UInt16(void);
  void                          SSE_Int8_Int32(void);
  void                          SSE_UInt8_Int32(void);
  void                          SSE_Int8_UInt32(void);
  void                          SSE_UInt8_UInt32(void);
  void                          SSE_Int8_Int64(void);
  void                          SSE_UInt8_Int64(void);
  void                          SSE_Int8_UInt64(void);
  void                          SSE_UInt8_UInt64(void);

  void                          SSE_Int16_UInt16(void);
  void                          SSE_Int16_Int8(void);
  void                          SSE_UInt16_Int8(void);
  void                          SSE_Int16_UInt8(void);
  void                          SSE_UInt16_UInt8(void);
  void                          SSE_Int16_Int32(void);
  void                          SSE_UInt16_Int32(void);
  void                          SSE_Int16_UInt32(void);
  void                          SSE_UInt16_UInt32(void);
  void                          SSE_Int16_Int64(void);
  void                          SSE_UInt16_Int64(void);
  void                          SSE_Int16_UInt64(void);
  void                          SSE_UInt16_UInt64(void);

  void                          SSE_Int32_UInt32(void);
  void                          SSE_Int32_Int8(void);
  void                          SSE_UInt32_Int8(void);
  void                          SSE_Int32_UInt8(void);
  void                          SSE_UInt32_UInt8(void);
  void                          SSE_Int32_Int16(void);
  void                          SSE_UInt32_Int16(void);
  void                          SSE_Int32_UInt16(void);
  void                          SSE_UInt32_UInt16(void);
  void                          SSE_Int32_Int64(void);
  void                          SSE_UInt32_Int64(void);
  void                          SSE_Int32_UInt64(void);
  void                          SSE_UInt32_UInt64(void);

  void                          SSE_Int64_UInt64(void);
  void                          SSE_Int64_Int8(void);
  void                          SSE_UInt64_Int8(void);
  void                          SSE_Int64_UInt8(void);
  void                          SSE_UInt64_UInt8(void);
  void                          SSE_Int64_Int16(void);
  void                          SSE_UInt64_Int16(void);
  void                          SSE_Int64_UInt16(void);
  void                          SSE_UInt64_UInt16(void);
  void                          SSE_Int64_Int32(void);
  void                          SSE_UInt64_Int32(void);
  void                          SSE_Int64_UInt32(void);
  void                          SSE_UInt64_UInt32(void);

private:
  template<class _vector> void  SSE_test(void);
  template<class _dstvector, class _srcvector> void SSE_convert(void);
#endif

#ifdef ENABLE_SSE2
private slots:
  void                          SSE2_Int8(void);
  void                          SSE2_UInt8(void);
  void                          SSE2_Int16(void);
  void                          SSE2_UInt16(void);
  void                          SSE2_Int32(void);
  void                          SSE2_UInt32(void);
  void                          SSE2_Int64(void);
  void                          SSE2_UInt64(void);

  void                          SSE2_Int8_UInt8(void);
  void                          SSE2_Int8_Int16(void);
  void                          SSE2_UInt8_Int16(void);
  void                          SSE2_Int8_UInt16(void);
  void                          SSE2_UInt8_UInt16(void);
  void                          SSE2_Int8_Int32(void);
  void                          SSE2_UInt8_Int32(void);
  void                          SSE2_Int8_UInt32(void);
  void                          SSE2_UInt8_UInt32(void);
  void                          SSE2_Int8_Int64(void);
  void                          SSE2_UInt8_Int64(void);
  void                          SSE2_Int8_UInt64(void);
  void                          SSE2_UInt8_UInt64(void);

  void                          SSE2_Int16_UInt16(void);
  void                          SSE2_Int16_Int8(void);
  void                          SSE2_UInt16_Int8(void);
  void                          SSE2_Int16_UInt8(void);
  void                          SSE2_UInt16_UInt8(void);
  void                          SSE2_Int16_Int32(void);
  void                          SSE2_UInt16_Int32(void);
  void                          SSE2_Int16_UInt32(void);
  void                          SSE2_UInt16_UInt32(void);
  void                          SSE2_Int16_Int64(void);
  void                          SSE2_UInt16_Int64(void);
  void                          SSE2_Int16_UInt64(void);
  void                          SSE2_UInt16_UInt64(void);

  void                          SSE2_Int32_UInt32(void);
  void                          SSE2_Int32_Int8(void);
  void                          SSE2_UInt32_Int8(void);
  void                          SSE2_Int32_UInt8(void);
  void                          SSE2_UInt32_UInt8(void);
  void                          SSE2_Int32_Int16(void);
  void                          SSE2_UInt32_Int16(void);
  void                          SSE2_Int32_UInt16(void);
  void                          SSE2_UInt32_UInt16(void);
  void                          SSE2_Int32_Int64(void);
  void                          SSE2_UInt32_Int64(void);
  void                          SSE2_Int32_UInt64(void);
  void                          SSE2_UInt32_UInt64(void);

  void                          SSE2_Int64_UInt64(void);
  void                          SSE2_Int64_Int8(void);
  void                          SSE2_UInt64_Int8(void);
  void                          SSE2_Int64_UInt8(void);
  void                          SSE2_UInt64_UInt8(void);
  void                          SSE2_Int64_Int16(void);
  void                          SSE2_UInt64_Int16(void);
  void                          SSE2_Int64_UInt16(void);
  void                          SSE2_UInt64_UInt16(void);
  void                          SSE2_Int64_Int32(void);
  void                          SSE2_UInt64_Int32(void);
  void                          SSE2_Int64_UInt32(void);
  void                          SSE2_UInt64_UInt32(void);

private:
  template<class _vector> void  SSE2_test(void);
  template<class _dstvector, class _srcvector> void SSE2_convert(void);
#endif

#ifdef ENABLE_SSE3
private slots:
  void                          SSE3_Int8(void);
  void                          SSE3_UInt8(void);
  void                          SSE3_Int16(void);
  void                          SSE3_UInt16(void);
  void                          SSE3_Int32(void);
  void                          SSE3_UInt32(void);
  void                          SSE3_Int64(void);
  void                          SSE3_UInt64(void);

  void                          SSE3_Int8_UInt8(void);
  void                          SSE3_Int8_Int16(void);
  void                          SSE3_UInt8_Int16(void);
  void                          SSE3_Int8_UInt16(void);
  void                          SSE3_UInt8_UInt16(void);
  void                          SSE3_Int8_Int32(void);
  void                          SSE3_UInt8_Int32(void);
  void                          SSE3_Int8_UInt32(void);
  void                          SSE3_UInt8_UInt32(void);
  void                          SSE3_Int8_Int64(void);
  void                          SSE3_UInt8_Int64(void);
  void                          SSE3_Int8_UInt64(void);
  void                          SSE3_UInt8_UInt64(void);

  void                          SSE3_Int16_UInt16(void);
  void                          SSE3_Int16_Int8(void);
  void                          SSE3_UInt16_Int8(void);
  void                          SSE3_Int16_UInt8(void);
  void                          SSE3_UInt16_UInt8(void);
  void                          SSE3_Int16_Int32(void);
  void                          SSE3_UInt16_Int32(void);
  void                          SSE3_Int16_UInt32(void);
  void                          SSE3_UInt16_UInt32(void);
  void                          SSE3_Int16_Int64(void);
  void                          SSE3_UInt16_Int64(void);
  void                          SSE3_Int16_UInt64(void);
  void                          SSE3_UInt16_UInt64(void);

  void                          SSE3_Int32_UInt32(void);
  void                          SSE3_Int32_Int8(void);
  void                          SSE3_UInt32_Int8(void);
  void                          SSE3_Int32_UInt8(void);
  void                          SSE3_UInt32_UInt8(void);
  void                          SSE3_Int32_Int16(void);
  void                          SSE3_UInt32_Int16(void);
  void                          SSE3_Int32_UInt16(void);
  void                          SSE3_UInt32_UInt16(void);
  void                          SSE3_Int32_Int64(void);
  void                          SSE3_UInt32_Int64(void);
  void                          SSE3_Int32_UInt64(void);
  void                          SSE3_UInt32_UInt64(void);

  void                          SSE3_Int64_UInt64(void);
  void                          SSE3_Int64_Int8(void);
  void                          SSE3_UInt64_Int8(void);
  void                          SSE3_Int64_UInt8(void);
  void                          SSE3_UInt64_UInt8(void);
  void                          SSE3_Int64_Int16(void);
  void                          SSE3_UInt64_Int16(void);
  void                          SSE3_Int64_UInt16(void);
  void                          SSE3_UInt64_UInt16(void);
  void                          SSE3_Int64_Int32(void);
  void                          SSE3_UInt64_Int32(void);
  void                          SSE3_Int64_UInt32(void);
  void                          SSE3_UInt64_UInt32(void);

private:
  template<class _vector> void  SSE3_test(void);
  template<class _dstvector, class _srcvector> void SSE3_convert(void);
#endif

#ifdef ENABLE_SSSE3
private slots:
  void                          SSSE3_Int8(void);
  void                          SSSE3_UInt8(void);
  void                          SSSE3_Int16(void);
  void                          SSSE3_UInt16(void);
  void                          SSSE3_Int32(void);
  void                          SSSE3_UInt32(void);
  void                          SSSE3_Int64(void);
  void                          SSSE3_UInt64(void);

  void                          SSSE3_Int8_UInt8(void);
  void                          SSSE3_Int8_Int16(void);
  void                          SSSE3_UInt8_Int16(void);
  void                          SSSE3_Int8_UInt16(void);
  void                          SSSE3_UInt8_UInt16(void);
  void                          SSSE3_Int8_Int32(void);
  void                          SSSE3_UInt8_Int32(void);
  void                          SSSE3_Int8_UInt32(void);
  void                          SSSE3_UInt8_UInt32(void);
  void                          SSSE3_Int8_Int64(void);
  void                          SSSE3_UInt8_Int64(void);
  void                          SSSE3_Int8_UInt64(void);
  void                          SSSE3_UInt8_UInt64(void);

  void                          SSSE3_Int16_UInt16(void);
  void                          SSSE3_Int16_Int8(void);
  void                          SSSE3_UInt16_Int8(void);
  void                          SSSE3_Int16_UInt8(void);
  void                          SSSE3_UInt16_UInt8(void);
  void                          SSSE3_Int16_Int32(void);
  void                          SSSE3_UInt16_Int32(void);
  void                          SSSE3_Int16_UInt32(void);
  void                          SSSE3_UInt16_UInt32(void);
  void                          SSSE3_Int16_Int64(void);
  void                          SSSE3_UInt16_Int64(void);
  void                          SSSE3_Int16_UInt64(void);
  void                          SSSE3_UInt16_UInt64(void);

  void                          SSSE3_Int32_UInt32(void);
  void                          SSSE3_Int32_Int8(void);
  void                          SSSE3_UInt32_Int8(void);
  void                          SSSE3_Int32_UInt8(void);
  void                          SSSE3_UInt32_UInt8(void);
  void                          SSSE3_Int32_Int16(void);
  void                          SSSE3_UInt32_Int16(void);
  void                          SSSE3_Int32_UInt16(void);
  void                          SSSE3_UInt32_UInt16(void);
  void                          SSSE3_Int32_Int64(void);
  void                          SSSE3_UInt32_Int64(void);
  void                          SSSE3_Int32_UInt64(void);
  void                          SSSE3_UInt32_UInt64(void);

  void                          SSSE3_Int64_UInt64(void);
  void                          SSSE3_Int64_Int8(void);
  void                          SSSE3_UInt64_Int8(void);
  void                          SSSE3_Int64_UInt8(void);
  void                          SSSE3_UInt64_UInt8(void);
  void                          SSSE3_Int64_Int16(void);
  void                          SSSE3_UInt64_Int16(void);
  void                          SSSE3_Int64_UInt16(void);
  void                          SSSE3_UInt64_UInt16(void);
  void                          SSSE3_Int64_Int32(void);
  void                          SSSE3_UInt64_Int32(void);
  void                          SSSE3_Int64_UInt32(void);
  void                          SSSE3_UInt64_UInt32(void);

private:
  template<class _vector> void  SSSE3_test(void);
  template<class _dstvector, class _srcvector> void SSSE3_convert(void);
#endif

#ifdef ENABLE_SSE4
private slots:
  void                          SSE4_Int8(void);
  void                          SSE4_UInt8(void);
  void                          SSE4_Int16(void);
  void                          SSE4_UInt16(void);
  void                          SSE4_Int32(void);
  void                          SSE4_UInt32(void);
  void                          SSE4_Int64(void);
  void                          SSE4_UInt64(void);

  void                          SSE4_Int8_UInt8(void);
  void                          SSE4_Int8_Int16(void);
  void                          SSE4_UInt8_Int16(void);
  void                          SSE4_Int8_UInt16(void);
  void                          SSE4_UInt8_UInt16(void);
  void                          SSE4_Int8_Int32(void);
  void                          SSE4_UInt8_Int32(void);
  void                          SSE4_Int8_UInt32(void);
  void                          SSE4_UInt8_UInt32(void);
  void                          SSE4_Int8_Int64(void);
  void                          SSE4_UInt8_Int64(void);
  void                          SSE4_Int8_UInt64(void);
  void                          SSE4_UInt8_UInt64(void);

  void                          SSE4_Int16_UInt16(void);
  void                          SSE4_Int16_Int8(void);
  void                          SSE4_UInt16_Int8(void);
  void                          SSE4_Int16_UInt8(void);
  void                          SSE4_UInt16_UInt8(void);
  void                          SSE4_Int16_Int32(void);
  void                          SSE4_UInt16_Int32(void);
  void                          SSE4_Int16_UInt32(void);
  void                          SSE4_UInt16_UInt32(void);
  void                          SSE4_Int16_Int64(void);
  void                          SSE4_UInt16_Int64(void);
  void                          SSE4_Int16_UInt64(void);
  void                          SSE4_UInt16_UInt64(void);

  void                          SSE4_Int32_UInt32(void);
  void                          SSE4_Int32_Int8(void);
  void                          SSE4_UInt32_Int8(void);
  void                          SSE4_Int32_UInt8(void);
  void                          SSE4_UInt32_UInt8(void);
  void                          SSE4_Int32_Int16(void);
  void                          SSE4_UInt32_Int16(void);
  void                          SSE4_Int32_UInt16(void);
  void                          SSE4_UInt32_UInt16(void);
  void                          SSE4_Int32_Int64(void);
  void                          SSE4_UInt32_Int64(void);
  void                          SSE4_Int32_UInt64(void);
  void                          SSE4_UInt32_UInt64(void);

  void                          SSE4_Int64_UInt64(void);
  void                          SSE4_Int64_Int8(void);
  void                          SSE4_UInt64_Int8(void);
  void                          SSE4_Int64_UInt8(void);
  void                          SSE4_UInt64_UInt8(void);
  void                          SSE4_Int64_Int16(void);
  void                          SSE4_UInt64_Int16(void);
  void                          SSE4_Int64_UInt16(void);
  void                          SSE4_UInt64_UInt16(void);
  void                          SSE4_Int64_Int32(void);
  void                          SSE4_UInt64_Int32(void);
  void                          SSE4_Int64_UInt32(void);
  void                          SSE4_UInt64_UInt32(void);
#endif

private:
  template<class _vector> void  SSE4_test(void);
  template<class _dstvector, class _srcvector> void SSE4_convert(void);
};
