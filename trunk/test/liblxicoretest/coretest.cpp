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

#include "coretest.h"
#include <QtGui>
#include <QtTest>

void CoreTest::initTestCase(void)
{
  mediaApp = SApplication::createForQTest(this);
}

void CoreTest::cleanupTestCase(void)
{
  delete mediaApp;
  mediaApp = NULL;
}

/*! Validates that the SStringParser::toCleanName method functions correctly.
 */
void CoreTest::StringParser_CleanName(void)
{
  QCOMPARE(SStringParser::toCleanName("**Clean-*!name?"), QString("Clean name"));
}

/*! Validates that the SStringParser::toRawName method functions correctly.
 */
void CoreTest::StringParser_RawName(void)
{
  QCOMPARE(SStringParser::toRawName("R&~a%%%W-**n   A#$m@@@__+@e?!!1/{  }2"), QString("RAWNAME12"));
}

/*! Validates that the SStringParser::toRawPath method functions correctly.
 */
void CoreTest::StringParser_RawPath(void)
{
  QCOMPARE(SStringParser::toRawPath("/R&~a%%%W-*/ /*n   A#$m@@@__+@e?/!!1{  }2/"), QString("RAW/NAME/12"));
}

/*! Validates that the SStringParser::findMatch method functions correctly.
 */
void CoreTest::StringParser_FindMatch(void)
{
  QCOMPARE(SStringParser::findMatch("long sentence", "weird sentinel"), QString(" sent"));
  QCOMPARE(SStringParser::findMatch("something new", "something old"), QString("something "));
  QCOMPARE(SStringParser::findMatch("the weirdest text", "the most cleanest text"), QString("est text"));

  QCOMPARE(SStringParser::findMatch("brown", "fax").length(), 0);
  QCOMPARE(SStringParser::findMatch("green", "bear").length(), 1);
}

/*! Validates that the SStringParser::computeMatch method functions correctly.
 */
void CoreTest::StringParser_ComputeMatch(void)
{
  QVERIFY(qFuzzyCompare(SStringParser::computeMatch("some text", "some text"), 1.0));
  QVERIFY(qFuzzyCompare(SStringParser::computeMatch("some text with additional useless information", "some text"), 1.0));
  QVERIFY(SStringParser::computeMatch("some text with additional useless information", "some text with useful information") > 0.1f);
  QVERIFY(SStringParser::computeMatch("some text with additional useless information", "some text") >
          SStringParser::computeMatch("some text with additional useless information", "some text with useful information"));
}
