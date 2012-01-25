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

class VecFloatTest : public QObject
{
Q_OBJECT
public:
  inline explicit               VecFloatTest(QObject *parent) : QObject(parent) { }

private slots:
  void                          Scalar_Float(void);
  void                          Scalar_Double(void);

  void                          Scalar_Float_Int8(void);
  void                          Scalar_Float_UInt8(void);
  void                          Scalar_Float_Int16(void);
  void                          Scalar_Float_UInt16(void);
  void                          Scalar_Float_Int32(void);
  void                          Scalar_Float_UInt32(void);
  void                          Scalar_Float_Int64(void);
  void                          Scalar_Float_UInt64(void);

  void                          Scalar_Double_Int8(void);
  void                          Scalar_Double_UInt8(void);
  void                          Scalar_Double_Int16(void);
  void                          Scalar_Double_UInt16(void);
  void                          Scalar_Double_Int32(void);
  void                          Scalar_Double_UInt32(void);
  void                          Scalar_Double_Int64(void);
  void                          Scalar_Double_UInt64(void);

private:
  template<class _vector> void  Scalar_test(void);
  template<class _dstvector, class _srcvector> void Scalar_convertFloatVector(void);
  template<class _dstvector, class _srcvector> void Scalar_convertDoubleVector(void);

#ifdef ENABLE_SSE
private slots:
  void                          SSE_Float(void);
  void                          SSE_Double(void);

  void                          SSE_Float_Int8(void);
  void                          SSE_Float_UInt8(void);
  void                          SSE_Float_Int16(void);
  void                          SSE_Float_UInt16(void);
  void                          SSE_Float_Int32(void);
  void                          SSE_Float_UInt32(void);
  void                          SSE_Float_Int64(void);
  void                          SSE_Float_UInt64(void);

  void                          SSE_Double_Int8(void);
  void                          SSE_Double_UInt8(void);
  void                          SSE_Double_Int16(void);
  void                          SSE_Double_UInt16(void);
  void                          SSE_Double_Int32(void);
  void                          SSE_Double_UInt32(void);
  void                          SSE_Double_Int64(void);
  void                          SSE_Double_UInt64(void);

private:
  template<class _vector> void  SSE_test(void);
  template<class _dstvector, class _srcvector> void SSE_convertFloatVector(void);
  template<class _dstvector, class _srcvector> void SSE_convertDoubleVector(void);
#endif

#ifdef ENABLE_SSE2
private slots:
  void                          SSE2_Float(void);
  void                          SSE2_Double(void);

  void                          SSE2_Float_Int8(void);
  void                          SSE2_Float_UInt8(void);
  void                          SSE2_Float_Int16(void);
  void                          SSE2_Float_UInt16(void);
  void                          SSE2_Float_Int32(void);
  void                          SSE2_Float_UInt32(void);
  void                          SSE2_Float_Int64(void);
  void                          SSE2_Float_UInt64(void);

  void                          SSE2_Double_Int8(void);
  void                          SSE2_Double_UInt8(void);
  void                          SSE2_Double_Int16(void);
  void                          SSE2_Double_UInt16(void);
  void                          SSE2_Double_Int32(void);
  void                          SSE2_Double_UInt32(void);
  void                          SSE2_Double_Int64(void);
  void                          SSE2_Double_UInt64(void);

private:
  template<class _vector> void  SSE2_test(void);
  template<class _dstvector, class _srcvector> void SSE2_convertFloatVector(void);
  template<class _dstvector, class _srcvector> void SSE2_convertDoubleVector(void);
#endif

#ifdef ENABLE_SSE3
private slots:
  void                          SSE3_Float(void);
  void                          SSE3_Double(void);

  void                          SSE3_Float_Int8(void);
  void                          SSE3_Float_UInt8(void);
  void                          SSE3_Float_Int16(void);
  void                          SSE3_Float_UInt16(void);
  void                          SSE3_Float_Int32(void);
  void                          SSE3_Float_UInt32(void);
  void                          SSE3_Float_Int64(void);
  void                          SSE3_Float_UInt64(void);

  void                          SSE3_Double_Int8(void);
  void                          SSE3_Double_UInt8(void);
  void                          SSE3_Double_Int16(void);
  void                          SSE3_Double_UInt16(void);
  void                          SSE3_Double_Int32(void);
  void                          SSE3_Double_UInt32(void);
  void                          SSE3_Double_Int64(void);
  void                          SSE3_Double_UInt64(void);

private:
  template<class _vector> void  SSE3_test(void);
  template<class _dstvector, class _srcvector> void SSE3_convertFloatVector(void);
  template<class _dstvector, class _srcvector> void SSE3_convertDoubleVector(void);
#endif

#ifdef ENABLE_SSSE3
private slots:
  void                          SSSE3_Float(void);
  void                          SSSE3_Double(void);

  void                          SSSE3_Float_Int8(void);
  void                          SSSE3_Float_UInt8(void);
  void                          SSSE3_Float_Int16(void);
  void                          SSSE3_Float_UInt16(void);
  void                          SSSE3_Float_Int32(void);
  void                          SSSE3_Float_UInt32(void);
  void                          SSSE3_Float_Int64(void);
  void                          SSSE3_Float_UInt64(void);

  void                          SSSE3_Double_Int8(void);
  void                          SSSE3_Double_UInt8(void);
  void                          SSSE3_Double_Int16(void);
  void                          SSSE3_Double_UInt16(void);
  void                          SSSE3_Double_Int32(void);
  void                          SSSE3_Double_UInt32(void);
  void                          SSSE3_Double_Int64(void);
  void                          SSSE3_Double_UInt64(void);

private:
  template<class _vector> void  SSSE3_test(void);
  template<class _dstvector, class _srcvector> void SSSE3_convertFloatVector(void);
  template<class _dstvector, class _srcvector> void SSSE3_convertDoubleVector(void);
#endif

#ifdef ENABLE_SSE4
private slots:
  void                          SSE4_Float(void);
  void                          SSE4_Double(void);

  void                          SSE4_Float_Int8(void);
  void                          SSE4_Float_UInt8(void);
  void                          SSE4_Float_Int16(void);
  void                          SSE4_Float_UInt16(void);
  void                          SSE4_Float_Int32(void);
  void                          SSE4_Float_UInt32(void);
  void                          SSE4_Float_Int64(void);
  void                          SSE4_Float_UInt64(void);

  void                          SSE4_Double_Int8(void);
  void                          SSE4_Double_UInt8(void);
  void                          SSE4_Double_Int16(void);
  void                          SSE4_Double_UInt16(void);
  void                          SSE4_Double_Int32(void);
  void                          SSE4_Double_UInt32(void);
  void                          SSE4_Double_Int64(void);
  void                          SSE4_Double_UInt64(void);

private:
  template<class _vector> void  SSE4_test(void);
  template<class _dstvector, class _srcvector> void SSE4_convertFloatVector(void);
  template<class _dstvector, class _srcvector> void SSE4_convertDoubleVector(void);
#endif
};
