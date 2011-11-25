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
#include <LXiStreamGui>

class IOTest : public QObject
{
Q_OBJECT
public:
  inline explicit               IOTest(QObject *parent) : QObject(parent), mediaApp(NULL) { }

private slots:
  void                          initTestCase(void);
  void                          cleanupTestCase(void);

private slots:
  void                          MediaFileInfoImage(void);

  void                          AudioResamplerHalfRate(void);
  void                          AudioResamplerDoubleRate(void);

  void                          BufferSerializerLoopback(void);

private slots:
  void                          receive(const QByteArray &);
  void                          receive(const SAudioBuffer &);
  void                          receive(const SVideoBuffer &);

private:
  SAudioBuffer                  createAudioBuffer(unsigned sampleRate);
  SVideoBuffer                  createVideoBuffer(const SSize &);

private:
  SApplication                * mediaApp;
  QList<QByteArray>             messageList;
  SAudioBufferList              audioBufferList;
  SVideoBufferList              videoBufferList;
};
