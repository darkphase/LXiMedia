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

#include <QtCore>

class VecBoolTest : public QObject
{
Q_OBJECT
public:
  inline explicit               VecBoolTest(QObject *parent) : QObject(parent) { }

private slots:
  void                          Scalar_Bool8(void);
  void                          Scalar_Bool16(void);
  void                          Scalar_Bool32(void);
  void                          Scalar_Bool64(void);

private:
  template<class _vector> void  Scalar_test(void);

#ifdef ENABLE_SSE
private slots:
  void                          SSE_Bool8(void);
  void                          SSE_Bool16(void);
  void                          SSE_Bool32(void);
  void                          SSE_Bool64(void);

private:
  template<class _vector> void  SSE_test(void);
#endif

#ifdef ENABLE_SSE2
private slots:
  void                          SSE2_Bool8(void);
  void                          SSE2_Bool16(void);
  void                          SSE2_Bool32(void);
  void                          SSE2_Bool64(void);

private:
  template<class _vector> void  SSE2_test(void);
#endif

#ifdef ENABLE_SSE3
private slots:
  void                          SSE3_Bool8(void);
  void                          SSE3_Bool16(void);
  void                          SSE3_Bool32(void);
  void                          SSE3_Bool64(void);

private:
  template<class _vector> void  SSE3_test(void);
#endif

#ifdef ENABLE_SSSE3
private slots:
  void                          SSSE3_Bool8(void);
  void                          SSSE3_Bool16(void);
  void                          SSSE3_Bool32(void);
  void                          SSSE3_Bool64(void);

private:
  template<class _vector> void  SSSE3_test(void);
#endif

#ifdef ENABLE_SSE4
private slots:
  void                          SSE4_Bool8(void);
  void                          SSE4_Bool16(void);
  void                          SSE4_Bool32(void);
  void                          SSE4_Bool64(void);

private:
  template<class _vector> void  SSE4_test(void);
#endif
};
