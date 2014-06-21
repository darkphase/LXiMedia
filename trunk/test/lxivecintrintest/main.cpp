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
#include <QTest>
#include "scalartest.h"
#include "vecbooltest.h"
#include "vecfloattest.h"
#include "vecinttest.h"

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  if (QTest::qExec(new ScalarTest(&app), app.arguments()) != 0)     return 1;
  if (QTest::qExec(new VecBoolTest(&app), app.arguments()) != 0)    return 1;
  if (QTest::qExec(new VecFloatTest(&app), app.arguments()) != 0)   return 1;
  if (QTest::qExec(new VecIntTest(&app), app.arguments()) != 0)     return 1;

  return 0;
}
