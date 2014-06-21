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

#include "vecfloattest.h"

void VecFloatTest::PREFIX(Float)(void)
{
  PREFIX(test)< lxivec::_private::FloatVector<1> >();
  PREFIX(test)< lxivec::_private::FloatVector<4> >();
  PREFIX(test)< lxivec::_private::FloatVector<9> >();
}

void VecFloatTest::PREFIX(Double)(void)
{
  PREFIX(test)< lxivec::_private::DoubleVector<1> >();
  PREFIX(test)< lxivec::_private::DoubleVector<2> >();
  PREFIX(test)< lxivec::_private::DoubleVector<5> >();
}

template<class _vector>
void VecFloatTest::PREFIX(test)(void)
{
  typedef typename _vector::single single;

  single lxivec_align data[_vector::count];
  for (int i=0; i<_vector::count; i++)
    data[i] = single(i + 8);

  _vector vector;
  lxivec::_private::load(vector.data, data);
  for (int i=0; i<vector.count; i++)
    QVERIFY(qAbs(vector.data.val[i] - single(i + 8)) < 0.1);

  vector = abs(vector);
  for (int i=0; i<vector.count; i++)
    QVERIFY(qAbs(vector.data.val[i] - single(i + 8)) < 0.1);

  _vector avector = vector + vector;
  for (int i=0; i<avector.count; i++)
    QVERIFY(qAbs(avector.data.val[i] - single((i + 8) * 2)) < 0.1);

  avector = avector - avector;
  for (int i=0; i<avector.count; i++)
    QVERIFY(qAbs(avector.data.val[i] - single(0.0)) < 0.1);

  _vector vectorplus8 = vector + 8;
  for (int i=0; i<vectorplus8.count; i++)
    QVERIFY(qAbs(vectorplus8.data.val[i] - (vector.data.val[i] + single(8.0))) < 0.1);

  _vector vectormin8 = vector - 8;
  for (int i=0; i<vectormin8.count; i++)
    QVERIFY(qAbs(vectormin8.data.val[i] - (vector.data.val[i] - single(8.0))) < 0.1);

  _vector vectormul3 = vector * 3;
  for (int i=0; i<vectormul3.count; i++)
    QVERIFY(qAbs(vectormul3.data.val[i] - (vector.data.val[i] * single(3.0))) < 0.1);

  _vector vectordiv8 = vector / 3;
  for (int i=0; i<vectordiv8.count; i++)
    QVERIFY(qAbs(vectordiv8.data.val[i] - (vector.data.val[i] / single(3.0))) < 0.1);

  _vector mvector = min(vector, vectormin8);
  for (int i=0; i<mvector.count; i++)
    QVERIFY(qAbs(mvector.data.val[i] - vectormin8.data.val[i]) < 0.1);

  mvector = max(vector, vectorplus8);
  for (int i=0; i<mvector.count; i++)
    QVERIFY(qAbs(mvector.data.val[i] - vectorplus8.data.val[i]) < 0.1);

  lxivec::_private::BoolVector<sizeof(single), _vector::count> cvector = vectormin8 == vectormin8;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] != 0);

  cvector = vectormin8 == vectorplus8;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] == 0);

  cvector = vectorplus8 > vectormin8;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] != 0);

  cvector = vectorplus8 > vectorplus8;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] == 0);

  cvector = vectormin8 < vectorplus8;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] != 0);

  cvector = vectormin8 < vectormin8;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] == 0);

  _vector svector = select(vectormin8 < vectorplus8, vectormin8, vectorplus8);
  for (int i=0; i<cvector.count; i++)
    QVERIFY(qAbs(svector.data.val[i] - vectormin8.data.val[i]) < 0.1);

  svector = select(vectormin8 < vectormin8, vectormin8, vectorplus8);
  for (int i=0; i<cvector.count; i++)
    QVERIFY(qAbs(svector.data.val[i] - vectorplus8.data.val[i]) < 0.1);
}

void VecFloatTest::PREFIX(Float_Int8)(void)
{
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int8_t, 1>,  lxivec::_private::FloatVector<1> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int8_t, 4>,  lxivec::_private::FloatVector<4> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int8_t, 9>,  lxivec::_private::FloatVector<9> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int8_t, 16>, lxivec::_private::FloatVector<16> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int8_t, 33>, lxivec::_private::FloatVector<33> >();
}

void VecFloatTest::PREFIX(Float_UInt8)(void)
{
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint8_t, 1>,  lxivec::_private::FloatVector<1> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint8_t, 4>,  lxivec::_private::FloatVector<4> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint8_t, 9>,  lxivec::_private::FloatVector<9> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint8_t, 16>, lxivec::_private::FloatVector<16> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint8_t, 33>, lxivec::_private::FloatVector<33> >();
}

void VecFloatTest::PREFIX(Float_Int16)(void)
{
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int16_t, 1>,  lxivec::_private::FloatVector<1> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int16_t, 4>,  lxivec::_private::FloatVector<4> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int16_t, 9>,  lxivec::_private::FloatVector<9> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int16_t, 17>, lxivec::_private::FloatVector<17> >();
}

void VecFloatTest::PREFIX(Float_UInt16)(void)
{
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint16_t, 1>,  lxivec::_private::FloatVector<1> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint16_t, 4>,  lxivec::_private::FloatVector<4> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint16_t, 9>,  lxivec::_private::FloatVector<9> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint16_t, 17>, lxivec::_private::FloatVector<17> >();
}

void VecFloatTest::PREFIX(Float_Int32)(void)
{
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int32_t, 1>,  lxivec::_private::FloatVector<1> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int32_t, 4>,  lxivec::_private::FloatVector<4> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int32_t, 9>,  lxivec::_private::FloatVector<9> >();
}

void VecFloatTest::PREFIX(Float_UInt32)(void)
{
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint32_t, 1>,  lxivec::_private::FloatVector<1> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint32_t, 4>,  lxivec::_private::FloatVector<4> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint32_t, 9>,  lxivec::_private::FloatVector<9> >();
}

void VecFloatTest::PREFIX(Float_Int64)(void)
{
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int64_t, 1>,  lxivec::_private::FloatVector<1> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int64_t, 4>,  lxivec::_private::FloatVector<4> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<int64_t, 9>,  lxivec::_private::FloatVector<9> >();
}

void VecFloatTest::PREFIX(Float_UInt64)(void)
{
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint64_t, 1>,  lxivec::_private::FloatVector<1> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint64_t, 2>,  lxivec::_private::FloatVector<2> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint64_t, 5>,  lxivec::_private::FloatVector<5> >();
  PREFIX(convertFloatVector)< lxivec::_private::IntVector<uint64_t, 9>,  lxivec::_private::FloatVector<9> >();
}

void VecFloatTest::PREFIX(Double_Int8)(void)
{
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int8_t, 1>,  lxivec::_private::DoubleVector<1> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int8_t, 2>,  lxivec::_private::DoubleVector<2> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int8_t, 5>,  lxivec::_private::DoubleVector<5> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int8_t, 16>, lxivec::_private::DoubleVector<16> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int8_t, 33>, lxivec::_private::DoubleVector<33> >();
}

void VecFloatTest::PREFIX(Double_UInt8)(void)
{
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint8_t, 1>,  lxivec::_private::DoubleVector<1> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint8_t, 2>,  lxivec::_private::DoubleVector<2> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint8_t, 5>,  lxivec::_private::DoubleVector<5> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint8_t, 16>, lxivec::_private::DoubleVector<16> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint8_t, 33>, lxivec::_private::DoubleVector<33> >();
}

void VecFloatTest::PREFIX(Double_Int16)(void)
{
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int16_t, 1>,  lxivec::_private::DoubleVector<1> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int16_t, 2>,  lxivec::_private::DoubleVector<2> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int16_t, 9>,  lxivec::_private::DoubleVector<9> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int16_t, 17>, lxivec::_private::DoubleVector<17> >();
}

void VecFloatTest::PREFIX(Double_UInt16)(void)
{
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint16_t, 1>,  lxivec::_private::DoubleVector<1> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint16_t, 2>,  lxivec::_private::DoubleVector<2> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint16_t, 9>,  lxivec::_private::DoubleVector<9> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint16_t, 17>, lxivec::_private::DoubleVector<17> >();
}

void VecFloatTest::PREFIX(Double_Int32)(void)
{
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int32_t, 1>,  lxivec::_private::DoubleVector<1> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int32_t, 2>,  lxivec::_private::DoubleVector<2> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int32_t, 5>,  lxivec::_private::DoubleVector<5> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int32_t, 9>,  lxivec::_private::DoubleVector<9> >();
}

void VecFloatTest::PREFIX(Double_UInt32)(void)
{
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint32_t, 1>,  lxivec::_private::DoubleVector<1> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint32_t, 2>,  lxivec::_private::DoubleVector<2> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint32_t, 5>,  lxivec::_private::DoubleVector<5> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint32_t, 9>,  lxivec::_private::DoubleVector<9> >();
}

void VecFloatTest::PREFIX(Double_Int64)(void)
{
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int64_t, 1>,  lxivec::_private::DoubleVector<1> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int64_t, 2>,  lxivec::_private::DoubleVector<2> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int64_t, 5>,  lxivec::_private::DoubleVector<5> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<int64_t, 9>,  lxivec::_private::DoubleVector<9> >();
}

void VecFloatTest::PREFIX(Double_UInt64)(void)
{
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint64_t, 1>,  lxivec::_private::DoubleVector<1> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint64_t, 2>,  lxivec::_private::DoubleVector<2> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint64_t, 5>,  lxivec::_private::DoubleVector<5> >();
  PREFIX(convertDoubleVector)< lxivec::_private::IntVector<uint64_t, 9>,  lxivec::_private::DoubleVector<9> >();
}

template<class _dstvector, class _srcvector>
void VecFloatTest::PREFIX(convertFloatVector)(void)
{
  typedef typename _dstvector::single dsttype;

  static const float values[] =
  {
    -5000000000.0f, -4000000000.0f, -3000000000.0f, -2000000000.0f,
    -70000.0f, -60000.0f, -40000.0f, -30000.0f,
    -300.0f, -200.0f, -100.0f,
     0.0f,
     100.0f, 200.0f, 300.0f,
     30000.0f,  40000.0f,  60000.0f,  70000.0f,
     2000000000.0f, 3000000000.0f, 4000000000.0f, 5000000000.0f
  };

  for (int v=0; v<int(sizeof(values)/sizeof(values[0])); v++)
  {
    float lxivec_align data[_srcvector::count];
    for (int i=0; i<_srcvector::count; i++)
      data[i] = values[v] + i;

    _srcvector srcvector;
    lxivec::_private::load(srcvector.data, data);

    _dstvector dstvector = srcvector;
    for (int i=0; i<dstvector.count; i++)
    {
      lxivec::_private::FloatVector<1> src;
      src.data.val[0] = srcvector.data.val[i];
      lxivec::_private::IntVector<dsttype, 1> dst = src;

      QCOMPARE(dstvector.data.val[i], dst.data.val[0]);
    }

    _srcvector retvector = dstvector;
    for (int i=0; i<retvector.count; i++)
    {
      lxivec::_private::IntVector<dsttype, 1> dst;
      dst.data.val[0] = dstvector.data.val[i];
      lxivec::_private::FloatVector<1> ret = dst;

      QVERIFY(qAbs(retvector.data.val[i] - ret.data.val[0]) < 0.1f);
    }
  }
}

template<class _dstvector, class _srcvector>
void VecFloatTest::PREFIX(convertDoubleVector)(void)
{
  typedef typename _dstvector::single dsttype;

  static const double values[] =
  {
    -5000000000.0, -4000000000.0, -3000000000.0, -2000000000.0,
    -70000.0, -60000.0, -40000.0, -30000.0,
    -300.0, -200.0, -100.0,
     0.0,
     100.0, 200.0, 300.0,
     30000.0,  40000.0,  60000.0,  70000.0,
     2000000000.0, 3000000000.0, 4000000000.0, 5000000000.0
  };

  for (int v=0; v<int(sizeof(values)/sizeof(values[0])); v++)
  {
    double lxivec_align data[_srcvector::count];
    for (int i=0; i<_srcvector::count; i++)
      data[i] = values[v] + i;

    _srcvector srcvector;
    lxivec::_private::load(srcvector.data, data);

    _dstvector dstvector = srcvector;
    for (int i=0; i<dstvector.count; i++)
    {
      lxivec::_private::DoubleVector<1> src;
      src.data.val[0] = srcvector.data.val[i];
      lxivec::_private::IntVector<dsttype, 1> dst = src;

      QCOMPARE(dstvector.data.val[i], dst.data.val[0]);
    }

    _srcvector retvector = dstvector;
    for (int i=0; i<retvector.count; i++)
    {
      lxivec::_private::IntVector<dsttype, 1> dst;
      dst.data.val[0] = dstvector.data.val[i];
      lxivec::_private::DoubleVector<1> ret = dst;

      QVERIFY(qAbs(retvector.data.val[i] - ret.data.val[0]) < 0.1);
    }
  }
}
