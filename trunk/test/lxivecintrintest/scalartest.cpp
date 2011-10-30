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

#undef __SSE__
#undef __SSE2__
#undef __SSE3__
#undef __SSSE3__
#undef __SSE4A__
#include <lxivecintrin/vectypes>
#include <QtTest>

#define PREFIX(x) Scalar_ ## x

#include "vecbooltest.hpp"
#include "vecinttest.hpp"
#include "vecfloattest.hpp"

#include "scalartest.h"

void ScalarTest::Scalar_Abs(void)
{
  QCOMPARE(lxivec::abs(qint8(0x00)),                      qint8(0x00));
  QCOMPARE(lxivec::abs(qint8(0x7F)),                      qint8(0x7F));
  QCOMPARE(lxivec::abs(qint8(0xFF)),                      qint8(0x01));
  QCOMPARE(lxivec::abs(qint8(0x80)),                      qint8(0x80));

  QCOMPARE(lxivec::abs(qint16(0x0000)),                   qint16(0x0000));
  QCOMPARE(lxivec::abs(qint16(0x7FFF)),                   qint16(0x7FFF));
  QCOMPARE(lxivec::abs(qint16(0xFFFF)),                   qint16(0x0001));
  QCOMPARE(lxivec::abs(qint16(0x8000)),                   qint16(0x8000));

  QCOMPARE(lxivec::abs(qint32(0x00000000)),               qint32(0x00000000));
  QCOMPARE(lxivec::abs(qint32(0x7FFFFFFF)),               qint32(0x7FFFFFFF));
  QCOMPARE(lxivec::abs(qint32(0xFFFFFFFF)),               qint32(0x00000001));
  QCOMPARE(lxivec::abs(qint32(0x80000000)),               qint32(0x80000000));

  QCOMPARE(lxivec::abs(Q_INT64_C(0x0000000000000000)),    Q_INT64_C(0x0000000000000000));
  QCOMPARE(lxivec::abs(Q_INT64_C(0x7FFFFFFFFFFFFFFF)),    Q_INT64_C(0x7FFFFFFFFFFFFFFF));
  QCOMPARE(lxivec::abs(Q_INT64_C(0xFFFFFFFFFFFFFFFF)),    Q_INT64_C(0x0000000000000001));
  QCOMPARE(lxivec::abs(Q_INT64_C(0x8000000000000000)),    Q_INT64_C(0x8000000000000000));
}

void ScalarTest::Scalar_Min(void)
{
  QCOMPARE(lxivec::min(qint8(0x80),                       qint8(0x7F)),                     qint8(0x80));
  QCOMPARE(lxivec::min(qint16(0x8000),                    qint16(0x7FFF)),                  qint16(0x8000));
  QCOMPARE(lxivec::min(qint32(0x80000000),                qint32(0x7FFFFFFF)),              qint32(0x80000000));
  QCOMPARE(lxivec::min(Q_INT64_C(0x8000000000000000),     Q_INT64_C(0x7FFFFFFFFFFFFFFF)),   Q_INT64_C(0x8000000000000000));

  QCOMPARE(lxivec::min(quint8(0x80),                      quint8(0x7F)),                    quint8(0x7F));
  QCOMPARE(lxivec::min(quint16(0x8000),                   quint16(0x7FFF)),                 quint16(0x7FFF));
  QCOMPARE(lxivec::min(quint32(0x80000000),               quint32(0x7FFFFFFF)),             quint32(0x7FFFFFFF));
  QCOMPARE(lxivec::min(Q_UINT64_C(0x8000000000000000),    Q_UINT64_C(0x7FFFFFFFFFFFFFFF)),  Q_UINT64_C(0x7FFFFFFFFFFFFFFF));
}

void ScalarTest::Scalar_Max(void)
{
  QCOMPARE(lxivec::max(qint8(0x80),                       qint8(0x7F)),                     qint8(0x7F));
  QCOMPARE(lxivec::max(qint16(0x8000),                    qint16(0x7FFF)),                  qint16(0x7FFF));
  QCOMPARE(lxivec::max(qint32(0x80000000),                qint32(0x7FFFFFFF)),              qint32(0x7FFFFFFF));
  QCOMPARE(lxivec::max(Q_INT64_C(0x8000000000000000),     Q_INT64_C(0x7FFFFFFFFFFFFFFF)),   Q_INT64_C(0x7FFFFFFFFFFFFFFF));

  QCOMPARE(lxivec::max(quint8(0x80),                      quint8(0x7F)),                    quint8(0x80));
  QCOMPARE(lxivec::max(quint16(0x8000),                   quint16(0x7FFF)),                 quint16(0x8000));
  QCOMPARE(lxivec::max(quint32(0x80000000),               quint32(0x7FFFFFFF)),             quint32(0x80000000));
  QCOMPARE(lxivec::max(Q_UINT64_C(0x8000000000000000),    Q_UINT64_C(0x7FFFFFFFFFFFFFFF)),  Q_UINT64_C(0x8000000000000000));
}

void ScalarTest::Scalar_Bound(void)
{
  QCOMPARE(lxivec::bound(qint8(0x80),                     qint8(0x7F),                      qint8(0x00)),                     qint8(0x00));
  QCOMPARE(lxivec::bound(qint16(0x8000),                  qint16(0x7FFF),                   qint16(0x0000)),                  qint16(0x0000));
  QCOMPARE(lxivec::bound(qint32(0x80000000),              qint32(0x7FFFFFFF),               qint32(0x00000000)),              qint32(0x00000000));
  QCOMPARE(lxivec::bound(Q_INT64_C(0x8000000000000000),   Q_INT64_C(0x7FFFFFFFFFFFFFFF),    Q_INT64_C(0x0000000000000000)),   Q_INT64_C(0x0000000000000000));

  QCOMPARE(lxivec::bound(quint8(0x80),                    quint8(0x7F),                     quint8(0x00)),                    quint8(0x80));
  QCOMPARE(lxivec::bound(quint16(0x8000),                 quint16(0x7FFF),                  quint16(0x0000)),                 quint16(0x8000));
  QCOMPARE(lxivec::bound(quint32(0x80000000),             quint32(0x7FFFFFFF),              quint32(0x00000000)),             quint32(0x80000000));
  QCOMPARE(lxivec::bound(Q_UINT64_C(0x8000000000000000),  Q_UINT64_C(0x7FFFFFFFFFFFFFFF),   Q_UINT64_C(0x0000000000000000)),  Q_UINT64_C(0x8000000000000000));
}

void ScalarTest::Scalar_Select(void)
{
  QCOMPARE(lxivec::select(true, qint8(1), qint8(2)), qint8(1));
  QCOMPARE(lxivec::select(false, qint8(1), qint8(2)), qint8(2));
  QCOMPARE(lxivec::select(true, quint8(1), quint8(2)), quint8(1));
  QCOMPARE(lxivec::select(false, quint8(1), quint8(2)), quint8(2));

  QCOMPARE(lxivec::select(true, qint16(1), qint16(2)), qint16(1));
  QCOMPARE(lxivec::select(false, qint16(1), qint16(2)), qint16(2));
  QCOMPARE(lxivec::select(true, quint16(1), quint16(2)), quint16(1));
  QCOMPARE(lxivec::select(false, quint16(1), quint16(2)), quint16(2));

  QCOMPARE(lxivec::select(true, qint32(1), qint32(2)), qint32(1));
  QCOMPARE(lxivec::select(false, qint32(1), qint32(2)), qint32(2));
  QCOMPARE(lxivec::select(true, quint32(1), quint32(2)), quint32(1));
  QCOMPARE(lxivec::select(false, quint32(1), quint32(2)), quint32(2));

  QCOMPARE(lxivec::select(true, qint64(1), qint64(2)), qint64(1));
  QCOMPARE(lxivec::select(false, qint64(1), qint64(2)), qint64(2));
  QCOMPARE(lxivec::select(true, quint64(1), quint64(2)), quint64(1));
  QCOMPARE(lxivec::select(false, quint64(1), quint64(2)), quint64(2));
}
