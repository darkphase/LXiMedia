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
#include <QtNetwork>
#include <LXiServer>

class SandboxTest : public QObject
{
Q_OBJECT
public:
  static int                    startSandbox(const QString &);

public:
  inline explicit               SandboxTest(QObject *parent) : QObject(parent), sandboxClient(NULL), responseCount(0) { }

private slots:
  void                          initTestCase(void);
  void                          cleanupTestCase(void);

  void                          sendRequests(void);

  void                          handleResponse(const SHttpEngine::ResponseMessage &);

private:
  SApplication                * mediaApp;
  LXiServer::SSandboxClient   * sandboxClient;
  static const int              numResponses;
  int                           responseCount;
};
