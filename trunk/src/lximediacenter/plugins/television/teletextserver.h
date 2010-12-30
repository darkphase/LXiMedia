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

#ifndef TELETEXTSERVER_H
#define TELETEXTSERVER_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStream>

namespace LXiMediaCenter {

class EpgDatabase;
class TelevisionBackend;

class TeletextServer : public BackendServer
{
Q_OBJECT
public:
                                TeletextServer(EpgDatabase *, MasterServer *server, Plugin *);
  virtual                       ~TeletextServer();

  virtual SearchResultList      search(const QStringList &) const;
  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

  bool                          needsUpdate(const QString &);
  SNode                       * createTeletextNode(const QString &, QObject *);

  SDataBuffer::TeletextPage     readPage(const QString &, int, int) const;
  QSet<int>                     allPages(const QString &) const;
  QSet<int>                     allSubPages(const QString &, int) const;
  int                           firstPage(const QString &) const;

private slots:
  void                          cleanDatabase(void);

private:
  void                          storePages(const QString &, QMap<quint32, SDataBuffer::TeletextPage> *);
  bool                          handleHtmlRequest(const QUrl &, const QString &, QAbstractSocket *);

private:
  static const int              updateTime = 43200; // Update once every 12 hours.

  class DataNode;
  EpgDatabase           * const epgDatabase;
  Plugin                * const plugin;
  QByteArray                    head;
  QTimer                        cleanTimer;

private:
  static const char     * const cssMain;
  static const char     * const cssColor;

  static const char     * const htmlMain;
  static const char     * const htmlChannel;
  static const char     * const htmlDisabledChannel;
  static const char     * const htmlSubpage;
};

} // End of namespace

#endif
