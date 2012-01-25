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
#include <LXiStream>

class CoreTest : public QObject
{
Q_OBJECT
public:
  inline explicit               CoreTest(QObject *parent) : QObject(parent), mediaApp(NULL) { }

private slots:
  void                          initTestCase(void);
  void                          cleanupTestCase(void);

private slots:
  void                          StringParser_CleanName(void);
  void                          StringParser_RawName(void);
  void                          StringParser_RawPath(void);
  void                          StringParser_FindMatch(void);
  void                          StringParser_ComputeMatch(void);

  void                          MemoryPool(void);

private:
  SApplication                * mediaApp;
};
