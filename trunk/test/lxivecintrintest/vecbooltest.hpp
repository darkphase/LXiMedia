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

#include "vecbooltest.h"

void VecBoolTest::PREFIX(Bool8)(void)
{
  PREFIX(test)< lxivec::_private::BoolVector<1, 1> >();
  PREFIX(test)< lxivec::_private::BoolVector<1, 16> >();
  PREFIX(test)< lxivec::_private::BoolVector<1, 33> >();
}

void VecBoolTest::PREFIX(Bool16)(void)
{
  PREFIX(test)< lxivec::_private::BoolVector<2, 1> >();
  PREFIX(test)< lxivec::_private::BoolVector<2, 16> >();
  PREFIX(test)< lxivec::_private::BoolVector<2, 33> >();
}

void VecBoolTest::PREFIX(Bool32)(void)
{
  PREFIX(test)< lxivec::_private::BoolVector<4, 1> >();
  PREFIX(test)< lxivec::_private::BoolVector<4, 16> >();
  PREFIX(test)< lxivec::_private::BoolVector<4, 33> >();
}

void VecBoolTest::PREFIX(Bool64)(void)
{
  PREFIX(test)< lxivec::_private::BoolVector<8, 1> >();
  PREFIX(test)< lxivec::_private::BoolVector<8, 16> >();
  PREFIX(test)< lxivec::_private::BoolVector<8, 33> >();
}

template<class _vector>
void VecBoolTest::PREFIX(test)(void)
{
  _vector vector;
  for (size_t i=0; i<sizeof(vector.data.val)/sizeof(vector.data.val[0]); i++)
    vector.data.val[i] = 0;

  _vector cvector = vector == vector;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] != 0);

  cvector = vector != cvector;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] != 0);

  cvector = vector == cvector;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] == 0);

  cvector = ~cvector;
  for (int i=0; i<cvector.count; i++)
    QVERIFY(cvector.data.val[i] != 0);

  _vector bvector = vector && (vector == vector);
  for (int i=0; i<bvector.count; i++)
    QVERIFY(bvector.data.val[i] == 0);

  bvector = vector || (vector == vector);
  for (int i=0; i<bvector.count; i++)
    QVERIFY(bvector.data.val[i] != 0);
}
