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

class CoreTest : public QObject
{
Q_OBJECT
public:
  inline explicit               CoreTest(QObject *parent) : QObject(parent), streamApp(NULL) { }

private slots:
  void                          initTestCase(void);
  void                          cleanupTestCase(void);

private slots: // SDebug
  void                          SDebug_Log(void);
  void                          SDebug_MutexLocker(void);

private slots: // SStringParser
  void                          StringParser_CleanName(void);
  void                          StringParser_RawName(void);
  void                          StringParser_RawPath(void);
  void                          StringParser_FindMatch(void);
  void                          StringParser_ComputeMatch(void);

private slots: // Engine
  void                          BufferData(void);
  void                          BufferClone(void);
  void                          BufferEnlarge(void);
  void                          BufferExternal(void);

  void                          AudioFormat(void);
  void                          AudioCodec(void);
  void                          VideoFormat(void);
  void                          VideoCodec(void);

  void                          Time(void);

private:
  static void                   fillBuffer(char *, int);

private:
  SApplication                * streamApp;
};