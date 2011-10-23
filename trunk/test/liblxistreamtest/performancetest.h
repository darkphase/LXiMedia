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
#include <LXiStream>

class PerformanceTest : public QObject
{
Q_OBJECT
public:
  inline explicit               PerformanceTest(QObject *parent) : QObject(parent) { }

private slots:
  void                          initTestCase(void);
  void                          cleanupTestCase(void);

private slots: // Data
  void                          DataSwap(void);

private slots: // Audio
  void                          AudioResample(void);

  void                          ConvertS8S16(void);
  void                          ConvertU8S16(void);
  void                          ConvertU8U16(void);
  void                          ConvertS16S32(void);
  void                          ConvertU16U32(void);
  void                          ConvertS16F32(void);
  void                          ConvertS32F32(void);
  void                          ConvertS16F64(void);
  void                          ConvertS32F64(void);
  void                          ConvertU16S16(void);
  void                          ConvertU32S32(void);

private slots: // Video
  void                          DeinterlaceBlend(void);
  void                          DeinterlaceSmartBlend(void);

  void                          ConvertYUYVtoRGB(void);
  void                          ConvertUYVYtoRGB(void);
  void                          ConvertBGRtoRGB(void);
  void                          ConvertYUV1toRGB(void);
  void                          ConvertYUV2toRGB(void);
  void                          ConvertYUYVtoYUV2(void);
  void                          ConvertUYVYtoYUV2(void);
  void                          MergeUVlines(void);

private slots:
  void                          Performance(void);

private:
  static void                 * AudioResampleUp(void *, const void *, size_t);
  static void                 * AudioResampleDown(void *, const void *, size_t);

  template<typename _rettype, typename _dsttype, typename _srctype, typename _sizetype>
  static float                  measureSpeed(_rettype (*)(_dsttype *, const _srctype *, _sizetype), const char *);
  template<typename _rettype, typename _dsttype, typename _srctype1, typename _srctype2, typename _sizetype>
  static float                  measureSpeed(_rettype (*)(_dsttype *, const _srctype1 *, const _srctype2 *, _sizetype), const char *);
  template<typename _rettype, typename _dsttype, typename _srctype1, typename _srctype2, typename _srctype3, typename _sizetype>
  static float                  measureSpeed(_rettype (*)(_dsttype *, const _srctype1 *, const _srctype2 *, const _srctype3 *, _sizetype), const char *);
  template<typename _rettype, typename _dsttype1, typename _dsttype2, typename _dsttype3, typename _srctype, typename _sizetype>
  static float                  measureSpeed(_rettype (*)(_dsttype1 *, _dsttype2 *, _dsttype3 *, const _srctype *, _sizetype), const char *);
};
