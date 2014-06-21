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

#ifndef EPGDATABASE_H
#define EPGDATABASE_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStream>

namespace LXiMediaCenter {

class TelevisionBackend;

class EpgDatabase : public QObject
{
Q_OBJECT
public:
  struct Programme
  {
    inline Programme(void) : recordPriority(0) { }

    QString                     name;
    QString                     category;
    QString                     description;
    QString                     channelName;
    QDateTime                   utcDate;
    QDateTime                   stationDate;
    int                         recordPriority;
  };

  struct View
  {
    QDateTime                   begin;
    QDateTime                   end;
  };

  struct Record : View
  {
    QString                     fileName;
  };

  static const int              daysInHistory = 12 * 7; // Keep 12 weeks in history

private:
  class XmlTvProgramme : public SSerializable,
                         public Programme
  {
  public:
    QDomNode                    toXml(QDomDocument &) const;
    virtual void                fromXml(const QDomNode &);
  };

public:
                                EpgDatabase(TelevisionBackend *plugin);
  virtual                       ~EpgDatabase();

  void                          importFile(const QString &fileName, bool remove = false);
  void                          importXml(const QByteArray &data);

  QStringList                   getChannels(void) const;
  QString                       getChannelName(const QString &rawName, QMap<QString, QString> *cache = NULL) const;
  QDateTime                     getLocalTime(const QString &, const QDateTime &) const;
  void                          setLocalTime(const QString &, const QTime &);

  Programme                     getProgramme(const QString &, const QDateTime &) const;
  Programme                     getNextProgramme(const QString &, const QDateTime &time) const;
  Programme                     getCurrentProgramme(const QString &) const;
  QList<Programme>              getProgrammes(const QString &, const QDateTime &, const QDateTime &) const;
  QList<Programme>              queryProgrammes(const QStringList &, const QDateTime &, const QDateTime &) const;
  void                          gatherEPG(const QString &, const SDataBuffer::TeletextPage &, int);
  void                          addProgramme(Programme);
  void                          recordProgramme(const QString &, const QDateTime &, int);
  QList<Programme>              getProgrammesToRecord(const QDateTime &, const QDateTime &) const;
  QList<Programme>              getProgrammesByName(const QString &, const QDateTime &, const QDateTime &) const;

  Record                        getRecord(const QString &, const QDateTime &) const;
  Record                        getActiveRecord(const QString &, const QDateTime &) const;
  QList<Record>                 getRecords(const QString &, const QDateTime &, const QDateTime &) const;
  void                          storeRecord(const QString &, const QString &, const QDateTime &, const QDateTime &);
  QList<Programme>              getRecordedProgrammes(const QDateTime &, const QDateTime &) const;

  View                          getView(const QString &, const QDateTime &) const;
  QList<View>                   getViews(const QString &, const QDateTime &, const QDateTime &) const;
  void                          storeView(const QString &, const QDateTime &, const QDateTime &);
  QList<Programme>              getViewedProgrammes(const QDateTime &, const QDateTime &) const;

private slots:
  void                          importExternalEpg(void);
  void                          cleanDatabase(void);

private:
  QDate                         dayFor(const QString &channelName, const QDateTime &utcDateTime) const;

  static QTime                  isTime(const QString &);
  static bool                   isPage(const QString &);
  static QString                filterName(QString);

  static QString                toDateString(const QDateTime &localDate, const QDateTime &utcDate);
  static void                   fromDateString(const QString &, QDateTime &localDate, QDateTime &utcDate);
  static QString                toDateString(const QDateTime &utcDate);
  static QDateTime              fromDateString(const QString &);

private:
  TelevisionBackend     * const plugin;

  QTimer                        extEpgTimer;
  QTimer                        cleanTimer;
};

} // End of namespace

#endif
