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

#include <QtCore>
#include <LXiStream>

class StreamTest : public QObject
{
Q_OBJECT
public:
  inline explicit               StreamTest(QObject *parent) : QObject(parent), mediaApp(NULL) { }

  static SAudioBuffer           makeTestBuffer(unsigned numSamples);

private slots:
  void                          initTestCase(void);
  void                          cleanupTestCase(void);

private slots:
  void                          BufferData(void);
  void                          BufferClone(void);
  void                          BufferEnlarge(void);
  void                          BufferExternal(void);

  void                          AudioBuffer(void);
  void                          AudioFormat(void);
  void                          AudioCodec(void);
  void                          VideoFormat(void);
  void                          VideoCodec(void);

  void                          Time(void);

private:
  static void                   fillBuffer(char *, int);

private:
  SApplication                * mediaApp;
};
