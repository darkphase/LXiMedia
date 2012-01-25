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

#define __DVDREADTEST_H

#include <QtCore>
#include <LXiStream>

class DVDNavTest : public QObject
{
Q_OBJECT
public:
  inline explicit               DVDNavTest(QObject *parent) : QObject(parent), mediaApp(NULL) { }

private slots:
  void                          initTestCase(void);
  void                          cleanupTestCase(void);

private:
  SApplication                * mediaApp;
};
