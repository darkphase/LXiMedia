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

#include "vecinttest.h"

void VecIntTest::PREFIX(Int8)(void)
{
  PREFIX(test)< lxivec::_private::IntVector<int8_t, 1> >();
  PREFIX(test)< lxivec::_private::IntVector<int8_t, 16> >();
  PREFIX(test)< lxivec::_private::IntVector<int8_t, 33> >();
}

void VecIntTest::PREFIX(UInt8)(void)
{
  PREFIX(test)< lxivec::_private::IntVector<uint8_t, 1> >();
  PREFIX(test)< lxivec::_private::IntVector<uint8_t, 16> >();
  PREFIX(test)< lxivec::_private::IntVector<uint8_t, 33> >();
}

void VecIntTest::PREFIX(Int16)(void)
{
  PREFIX(test)< lxivec::_private::IntVector<int16_t, 1> >();
  PREFIX(test)< lxivec::_private::IntVector<int16_t, 8> >();
  PREFIX(test)< lxivec::_private::IntVector<int16_t, 17> >();
}

void VecIntTest::PREFIX(UInt16)(void)
{
  PREFIX(test)< lxivec::_private::IntVector<uint16_t, 1> >();
  PREFIX(test)< lxivec::_private::IntVector<uint16_t, 8> >();
  PREFIX(test)< lxivec::_private::IntVector<uint16_t, 17> >();
}

void VecIntTest::PREFIX(Int32)(void)
{
  PREFIX(test)< lxivec::_private::IntVector<int32_t, 1> >();
  PREFIX(test)< lxivec::_private::IntVector<int32_t, 4> >();
  PREFIX(test)< lxivec::_private::IntVector<int32_t, 9> >();
}

void VecIntTest::PREFIX(UInt32)(void)
{
  PREFIX(test)< lxivec::_private::IntVector<uint32_t, 1> >();
  PREFIX(test)< lxivec::_private::IntVector<uint32_t, 4> >();
  PREFIX(test)< lxivec::_private::IntVector<uint32_t, 9> >();
}

void VecIntTest::PREFIX(Int64)(void)
{
  PREFIX(test)< lxivec::_private::IntVector<int64_t, 1> >();
  PREFIX(test)< lxivec::_private::IntVector<int64_t, 2> >();
  PREFIX(test)< lxivec::_private::IntVector<int64_t, 5> >();
}

void VecIntTest::PREFIX(UInt64)(void)
{
  PREFIX(test)< lxivec::_private::IntVector<uint64_t, 1> >();
  PREFIX(test)< lxivec::_private::IntVector<uint64_t, 2> >();
  PREFIX(test)< lxivec::_private::IntVector<uint64_t, 5> >();
}

template<class _vector>
void VecIntTest::PREFIX(test)(void)
{
  typedef typename _vector::single single;

  _vector vector;
  vector = _vector::set(3);
  for (int i=0; i<vector.count; i++)
    QCOMPARE(vector.data.val[i], single(3));

  vector = _vector::set(3, 4);
  for (int i=0; i+2<=vector.count; i+=2)
  {
    QCOMPARE(vector.data.val[i    ], single(3));
    QCOMPARE(vector.data.val[i + 1], single(4));
  }

  vector = _vector::set(3, 4, 5, 6);
  for (int i=0; i+4<=vector.count; i+=4)
  {
    QCOMPARE(vector.data.val[i    ], single(3));
    QCOMPARE(vector.data.val[i + 1], single(4));
    QCOMPARE(vector.data.val[i + 2], single(5));
    QCOMPARE(vector.data.val[i + 3], single(6));
  }

  single lxivec_align data[_vector::count];
  for (int i=0; i<_vector::count; i++)
    data[i] = single(i + 8);

  vector = _vector::set(0);
  lxivec::_private::load(vector.data, data, 1);
  QCOMPARE(vector.data.val[0], single(8));

  lxivec::_private::load(vector.data, data);
  for (int i=0; i<vector.count; i++)
    QCOMPARE(vector.data.val[i], single(i + 8));

  vector = abs(vector);
  for (int i=0; i<vector.count; i++)
    QCOMPARE(vector.data.val[i], single(i + 8));

  _vector avector = vector + vector;
  for (int i=0; i<avector.count; i++)
    QCOMPARE(avector.data.val[i], single((i + 8) * 2));

  avector = avector - avector;
  for (int i=0; i<avector.count; i++)
    QCOMPARE(avector.data.val[i], single(0));

  _vector vectorplus = vector + vector;
  for (int i=0; i<vectorplus.count; i++)
    QCOMPARE(vectorplus.data.val[i], single(vector.data.val[i] + vector.data.val[i]));

  vectorplus = vector + 8;
  for (int i=0; i<vectorplus.count; i++)
    QCOMPARE(vectorplus.data.val[i], single(vector.data.val[i] + 8));

  _vector vectormin = vector - vectorplus;
  for (int i=0; i<vectormin.count; i++)
    QCOMPARE(vectormin.data.val[i], single(vector.data.val[i] - vectorplus.data.val[i]));

  vectormin = vector - 8;
  for (int i=0; i<vectormin.count; i++)
    QCOMPARE(vectormin.data.val[i], single(vector.data.val[i] - 8));

  _vector vectormul = vector * vectorplus;
  for (int i=0; i<vectormul.count; i++)
    QCOMPARE(vectormul.data.val[i], single(vector.data.val[i] * vectorplus.data.val[i]));

  vectormul = vector * 3;
  for (int i=0; i<vectormul.count; i++)
    QCOMPARE(vectormul.data.val[i], single(vector.data.val[i] * 3));

  _vector mvector = min(vector, vectormin);
  for (int i=0; i<mvector.count; i++)
    QCOMPARE(mvector.data.val[i], vectormin.data.val[i]);

  mvector = max(vector, vectorplus);
  for (int i=0; i<mvector.count; i++)
    QCOMPARE(mvector.data.val[i], vectorplus.data.val[i]);

  lxivec::_private::IntVector<single, _vector::count / 2> havector = hadd(vector);
  for (int i=0; i<havector.count; i++)
    QCOMPARE(havector.data.val[i], single(vector.data.val[i * 2] + vector.data.val[i * 2 + 1]));

  single hsresult = 0;
  for (int i=0; i<vector.count; i++)
    hsresult += vector.data.val[i];

  QCOMPARE(hsum(vector), hsresult);

  lxivec::_private::IntVector<single, _vector::count * 2> ivector = interleave(vectormin, vectorplus);
  for (int i=0; i<vectormin.count; i++)
  {
    QCOMPARE(ivector.data.val[i * 2    ], vectormin.data.val[i]);
    QCOMPARE(ivector.data.val[i * 2 + 1], vectorplus.data.val[i]);
  }

  lxivec::_private::BoolVector<sizeof(single), _vector::count> cvector = vectormin == vectormin;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] != 0);

  cvector = vectormin == vectorplus;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] == 0);

  cvector = vectorplus > vectormin;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] != 0);

  cvector = vectorplus > vectorplus;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] == 0);

  cvector = vectormin < vectorplus;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] != 0);

  cvector = vectormin < vectormin;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] == 0);

  _vector svector = select(vectormin < vectorplus, vectormin, vectorplus);
  for (int i=0; i<svector.count; i++)
    QCOMPARE(svector.data.val[i], vectormin.data.val[i]);

  svector = select(vectormin < vectormin, vectormin, vectorplus);
  for (int i=0; i<svector.count; i++)
    QCOMPARE(svector.data.val[i], vectorplus.data.val[i]);

  _vector bvector = vectorplus & vectormin;
  for (int i=0; i<bvector.count; i++)
    QCOMPARE(bvector.data.val[i], single(vectorplus.data.val[i] & vectormin.data.val[i]));

  bvector = vectorplus | vectormin;
  for (int i=0; i<bvector.count; i++)
    QCOMPARE(bvector.data.val[i], single(vectorplus.data.val[i] | vectormin.data.val[i]));

  bvector = vectorplus & 1;
  for (int i=0; i<bvector.count; i++)
    QCOMPARE(bvector.data.val[i], single(vectorplus.data.val[i] & 1));

  bvector = vectorplus | 1;
  for (int i=0; i<bvector.count; i++)
    QCOMPARE(bvector.data.val[i], single(vectorplus.data.val[i] | 1));

  bvector = vectorplus << 1;
  for (int i=0; i<bvector.count; i++)
    QCOMPARE(bvector.data.val[i], single(vectorplus.data.val[i] << 1));

  bvector = vectorplus >> 1;
  for (int i=0; i<bvector.count; i++)
    QCOMPARE(bvector.data.val[i], single(vectorplus.data.val[i] >> 1));
}

void VecIntTest::PREFIX(Int8_UInt8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 1>,  lxivec::_private::IntVector<int8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 16>, lxivec::_private::IntVector<int8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 33>, lxivec::_private::IntVector<int8_t, 33> >();
}

void VecIntTest::PREFIX(Int8_Int16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 1>,  lxivec::_private::IntVector<int8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 16>, lxivec::_private::IntVector<int8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 33>, lxivec::_private::IntVector<int8_t, 33> >();
}

void VecIntTest::PREFIX(UInt8_Int16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 1>,  lxivec::_private::IntVector<uint8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 16>, lxivec::_private::IntVector<uint8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 33>, lxivec::_private::IntVector<uint8_t, 33> >();
}

void VecIntTest::PREFIX(Int8_UInt16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 1>,  lxivec::_private::IntVector<int8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 16>, lxivec::_private::IntVector<int8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 33>, lxivec::_private::IntVector<int8_t, 33> >();
}

void VecIntTest::PREFIX(UInt8_UInt16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 1>,  lxivec::_private::IntVector<uint8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 16>, lxivec::_private::IntVector<uint8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 33>, lxivec::_private::IntVector<uint8_t, 33> >();
}

void VecIntTest::PREFIX(Int8_Int32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 1>,  lxivec::_private::IntVector<int8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 16>, lxivec::_private::IntVector<int8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 33>, lxivec::_private::IntVector<int8_t, 33> >();
}

void VecIntTest::PREFIX(UInt8_Int32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 1>,  lxivec::_private::IntVector<uint8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 16>, lxivec::_private::IntVector<uint8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 33>, lxivec::_private::IntVector<uint8_t, 33> >();
}

void VecIntTest::PREFIX(Int8_UInt32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 1>,  lxivec::_private::IntVector<int8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 16>, lxivec::_private::IntVector<int8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 33>, lxivec::_private::IntVector<int8_t, 33> >();
}

void VecIntTest::PREFIX(UInt8_UInt32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 1>,  lxivec::_private::IntVector<uint8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 16>, lxivec::_private::IntVector<uint8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 33>, lxivec::_private::IntVector<uint8_t, 33> >();
}

void VecIntTest::PREFIX(Int8_Int64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 1>,  lxivec::_private::IntVector<int8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 16>, lxivec::_private::IntVector<int8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 33>, lxivec::_private::IntVector<int8_t, 33> >();
}

void VecIntTest::PREFIX(UInt8_Int64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 1>,  lxivec::_private::IntVector<uint8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 16>, lxivec::_private::IntVector<uint8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 33>, lxivec::_private::IntVector<uint8_t, 33> >();
}

void VecIntTest::PREFIX(Int8_UInt64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 1>,  lxivec::_private::IntVector<int8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 16>, lxivec::_private::IntVector<int8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 33>, lxivec::_private::IntVector<int8_t, 33> >();
}

void VecIntTest::PREFIX(UInt8_UInt64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 1>,  lxivec::_private::IntVector<uint8_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 16>, lxivec::_private::IntVector<uint8_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 33>, lxivec::_private::IntVector<uint8_t, 33> >();
}

void VecIntTest::PREFIX(Int16_UInt16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 1>,  lxivec::_private::IntVector<int16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 8>,  lxivec::_private::IntVector<int16_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 17>, lxivec::_private::IntVector<int16_t, 17> >();
}

void VecIntTest::PREFIX(Int16_Int8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 1>,  lxivec::_private::IntVector<int16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 16>, lxivec::_private::IntVector<int16_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 33>, lxivec::_private::IntVector<int16_t, 33> >();
}

void VecIntTest::PREFIX(UInt16_Int8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 1>,  lxivec::_private::IntVector<uint16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 16>, lxivec::_private::IntVector<uint16_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 33>, lxivec::_private::IntVector<uint16_t, 33> >();
}

void VecIntTest::PREFIX(Int16_UInt8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 1>,  lxivec::_private::IntVector<int16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 16>, lxivec::_private::IntVector<int16_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 33>, lxivec::_private::IntVector<int16_t, 33> >();
}

void VecIntTest::PREFIX(UInt16_UInt8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 1>,  lxivec::_private::IntVector<uint16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 16>, lxivec::_private::IntVector<uint16_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 33>, lxivec::_private::IntVector<uint16_t, 33> >();
}

void VecIntTest::PREFIX(Int16_Int32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 1>,  lxivec::_private::IntVector<int16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 8>,  lxivec::_private::IntVector<int16_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 17>, lxivec::_private::IntVector<int16_t, 17> >();
}

void VecIntTest::PREFIX(UInt16_Int32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 1>,  lxivec::_private::IntVector<uint16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 8>,  lxivec::_private::IntVector<uint16_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 17>, lxivec::_private::IntVector<uint16_t, 17> >();
}

void VecIntTest::PREFIX(Int16_UInt32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 1>,  lxivec::_private::IntVector<int16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 8>,  lxivec::_private::IntVector<int16_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 17>, lxivec::_private::IntVector<int16_t, 17> >();
}

void VecIntTest::PREFIX(UInt16_UInt32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 1>,  lxivec::_private::IntVector<uint16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 8>,  lxivec::_private::IntVector<uint16_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 17>, lxivec::_private::IntVector<uint16_t, 17> >();
}

void VecIntTest::PREFIX(Int16_Int64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 1>,  lxivec::_private::IntVector<int16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 8>,  lxivec::_private::IntVector<int16_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 17>, lxivec::_private::IntVector<int16_t, 17> >();
}

void VecIntTest::PREFIX(UInt16_Int64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 1>,  lxivec::_private::IntVector<uint16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 8>,  lxivec::_private::IntVector<uint16_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 17>, lxivec::_private::IntVector<uint16_t, 17> >();
}

void VecIntTest::PREFIX(Int16_UInt64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 1>,  lxivec::_private::IntVector<int16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 8>,  lxivec::_private::IntVector<int16_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 17>, lxivec::_private::IntVector<int16_t, 17> >();
}

void VecIntTest::PREFIX(UInt16_UInt64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 1>,  lxivec::_private::IntVector<uint16_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 8>,  lxivec::_private::IntVector<uint16_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 17>, lxivec::_private::IntVector<uint16_t, 17> >();
}

void VecIntTest::PREFIX(Int32_UInt32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 1>, lxivec::_private::IntVector<int32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 4>, lxivec::_private::IntVector<int32_t, 4> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 9>, lxivec::_private::IntVector<int32_t, 9> >();
}

void VecIntTest::PREFIX(Int32_Int8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 1>,  lxivec::_private::IntVector<int32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 16>, lxivec::_private::IntVector<int32_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 33>, lxivec::_private::IntVector<int32_t, 33> >();
}

void VecIntTest::PREFIX(UInt32_Int8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 1>,  lxivec::_private::IntVector<uint32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 16>, lxivec::_private::IntVector<uint32_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 33>, lxivec::_private::IntVector<uint32_t, 33> >();
}

void VecIntTest::PREFIX(Int32_UInt8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 1>,  lxivec::_private::IntVector<int32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 16>, lxivec::_private::IntVector<int32_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 33>, lxivec::_private::IntVector<int32_t, 33> >();
}

void VecIntTest::PREFIX(UInt32_UInt8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 1>,  lxivec::_private::IntVector<uint32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 16>, lxivec::_private::IntVector<uint32_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 33>, lxivec::_private::IntVector<uint32_t, 33> >();
}

void VecIntTest::PREFIX(Int32_Int16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 1>,  lxivec::_private::IntVector<int32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 8>,  lxivec::_private::IntVector<int32_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 17>, lxivec::_private::IntVector<int32_t, 17> >();
}

void VecIntTest::PREFIX(UInt32_Int16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 1>,  lxivec::_private::IntVector<uint32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 8>,  lxivec::_private::IntVector<uint32_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 17>, lxivec::_private::IntVector<uint32_t, 17> >();
}

void VecIntTest::PREFIX(Int32_UInt16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 1>,  lxivec::_private::IntVector<int32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 8>,  lxivec::_private::IntVector<int32_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 17>, lxivec::_private::IntVector<int32_t, 17> >();
}

void VecIntTest::PREFIX(UInt32_UInt16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 1>,  lxivec::_private::IntVector<uint32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 8>,  lxivec::_private::IntVector<uint32_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 17>, lxivec::_private::IntVector<uint32_t, 17> >();
}

void VecIntTest::PREFIX(Int32_Int64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 1>, lxivec::_private::IntVector<int32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 4>, lxivec::_private::IntVector<int32_t, 4> >();
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 9>, lxivec::_private::IntVector<int32_t, 9> >();
}

void VecIntTest::PREFIX(UInt32_Int64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 1>, lxivec::_private::IntVector<uint32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 4>, lxivec::_private::IntVector<uint32_t, 4> >();
  PREFIX(convert)< lxivec::_private::IntVector<int64_t, 9>, lxivec::_private::IntVector<uint32_t, 9> >();
}

void VecIntTest::PREFIX(Int32_UInt64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 1>, lxivec::_private::IntVector<int32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 4>, lxivec::_private::IntVector<int32_t, 4> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 9>, lxivec::_private::IntVector<int32_t, 9> >();
}

void VecIntTest::PREFIX(UInt32_UInt64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 1>, lxivec::_private::IntVector<uint32_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 4>, lxivec::_private::IntVector<uint32_t, 4> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 9>, lxivec::_private::IntVector<uint32_t, 9> >();
}

void VecIntTest::PREFIX(Int64_UInt64)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 1>, lxivec::_private::IntVector<int64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 2>, lxivec::_private::IntVector<int64_t, 2> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint64_t, 5>, lxivec::_private::IntVector<int64_t, 5> >();
}

void VecIntTest::PREFIX(Int64_Int8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 1>,  lxivec::_private::IntVector<int64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 16>, lxivec::_private::IntVector<int64_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 33>, lxivec::_private::IntVector<int64_t, 33> >();
}

void VecIntTest::PREFIX(UInt64_Int8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 1>,  lxivec::_private::IntVector<uint64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 16>, lxivec::_private::IntVector<uint64_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<int8_t, 33>, lxivec::_private::IntVector<uint64_t, 33> >();
}

void VecIntTest::PREFIX(Int64_UInt8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 1>,  lxivec::_private::IntVector<int64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 16>, lxivec::_private::IntVector<int64_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 33>, lxivec::_private::IntVector<int64_t, 33> >();
}

void VecIntTest::PREFIX(UInt64_UInt8)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 1>,  lxivec::_private::IntVector<uint64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 16>, lxivec::_private::IntVector<uint64_t, 16> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint8_t, 33>, lxivec::_private::IntVector<uint64_t, 33> >();
}

void VecIntTest::PREFIX(Int64_Int16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 1>,  lxivec::_private::IntVector<int64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 8>,  lxivec::_private::IntVector<int64_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 17>, lxivec::_private::IntVector<int64_t, 17> >();
}

void VecIntTest::PREFIX(UInt64_Int16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 1>,  lxivec::_private::IntVector<uint64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 8>,  lxivec::_private::IntVector<uint64_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<int16_t, 17>, lxivec::_private::IntVector<uint64_t, 17> >();
}

void VecIntTest::PREFIX(Int64_UInt16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 1>,  lxivec::_private::IntVector<int64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 8>,  lxivec::_private::IntVector<int64_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 17>, lxivec::_private::IntVector<int64_t, 17> >();
}

void VecIntTest::PREFIX(UInt64_UInt16)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 1>,  lxivec::_private::IntVector<uint64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 8>,  lxivec::_private::IntVector<uint64_t, 8> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint16_t, 17>, lxivec::_private::IntVector<uint64_t, 17> >();
}

void VecIntTest::PREFIX(Int64_Int32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 1>, lxivec::_private::IntVector<int64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 4>, lxivec::_private::IntVector<int64_t, 4> >();
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 9>, lxivec::_private::IntVector<int64_t, 9> >();
}

void VecIntTest::PREFIX(UInt64_Int32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 1>, lxivec::_private::IntVector<uint64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 4>, lxivec::_private::IntVector<uint64_t, 4> >();
  PREFIX(convert)< lxivec::_private::IntVector<int32_t, 9>, lxivec::_private::IntVector<uint64_t, 9> >();
}

void VecIntTest::PREFIX(Int64_UInt32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 1>, lxivec::_private::IntVector<int64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 4>, lxivec::_private::IntVector<int64_t, 4> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 9>, lxivec::_private::IntVector<int64_t, 9> >();
}

void VecIntTest::PREFIX(UInt64_UInt32)(void)
{
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 1>, lxivec::_private::IntVector<uint64_t, 1> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 4>, lxivec::_private::IntVector<uint64_t, 4> >();
  PREFIX(convert)< lxivec::_private::IntVector<uint32_t, 9>, lxivec::_private::IntVector<uint64_t, 9> >();
}

template<class _dstvector, class _srcvector>
void VecIntTest::PREFIX(convert)(void)
{
  typedef typename _srcvector::single srctype;
  typedef typename _dstvector::single dsttype;

  static const srctype values[] =
  {
    srctype(-5000000000ll), srctype(-4000000000ll), srctype(-3000000000ll), srctype(-2000000000ll),
    srctype(-70000l), srctype(-60000l), srctype(-40000l), srctype(-30000l),
    srctype(-300), srctype(-200), srctype(-100),
    srctype(0),
    srctype(100), srctype(200), srctype(300),
    srctype(30000l), srctype(40000l), srctype(60000l), srctype(70000l),
    srctype(2000000000ll), srctype(3000000000ll), srctype(4000000000ll), srctype(5000000000ll)
  };

  for (int v=0; v<int(sizeof(values)/sizeof(values[0])); v++)
  {
    srctype lxivec_align data[_srcvector::count];
    for (int i=0; i<_srcvector::count; i++)
      data[i] = values[v] + i;

    _srcvector srcvector;
    lxivec::_private::load(srcvector.data, data);

    _dstvector dstvector = srcvector;
    for (int i=0; i<dstvector.count; i++)
    {
      lxivec::_private::IntVector<srctype, 1> src;
      src.data.val[0] = srcvector.data.val[i];
      lxivec::_private::IntVector<dsttype, 1> dst = src;

      QCOMPARE(dstvector.data.val[i], dst.data.val[0]);
    }
  }
}
