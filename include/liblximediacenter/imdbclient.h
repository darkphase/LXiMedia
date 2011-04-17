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

#ifndef LXMEDIACENTER_IMDBCLIENT_H
#define LXMEDIACENTER_IMDBCLIENT_H

#include <QtCore>
#include <QtNetwork>
#include <LXiStream>
#include "export.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC ImdbClient : public QObject
{
Q_OBJECT
public:
  enum Type
  {
    Type_None                 = 0,
    Type_Movie                = 1,
    Type_TvShow               = 2
  };

  struct Entry
  {
    inline Entry(void)
        : type(Type_None), year(0), episodeNumber(0), seasonNumber(0),
          rating(0.0)
    {
    }

    inline bool                 isNull(void) const                              { return rawName.isEmpty(); }

    QString                     rawName;
    QString                     title;
    Type                        type;
    unsigned                    year;

    QString                     episodeName;
    unsigned                    episodeNumber;
    unsigned                    seasonNumber;

    QString                     plot;
    qreal                       rating;
  };

private:
  class ReadMoviesListEvent;
  class ReadPlotListEvent;
  class ReadRatingListEvent;

public:
                                ImdbClient(QObject *parent);
  virtual                       ~ImdbClient();

  void                          obtainIMDBFiles(void);

  bool                          isDownloading(void) const;
  bool                          isAvailable(void) const;
  bool                          needUpdate(void) const;

  Entry                         readEntry(const QString &rawName);
  QStringList                   findSimilar(const QString &title, Type);
  QString                       findBest(const QString &title, const QStringList &);

  static const char     * const sentinelItem;

protected:
  virtual void                  customEvent(QEvent *);

private:
  _lxi_internal void            importIMDBDatabase(void);

  _lxi_internal static Entry    decodeEntry(const QByteArray &);

private slots:
  _lxi_internal void            tryMirror(void);
  _lxi_internal void            finished(void);
  _lxi_internal void            error(void);

private:
  _lxi_internal static const char * const mirrors[];
  _lxi_internal static const QEvent::Type tryMirrorEventType;
  _lxi_internal static const QEvent::Type readMoviesListEventType;
  _lxi_internal static const QEvent::Type readPlotListEventType;
  _lxi_internal static const QEvent::Type readRatingListEventType;

  struct Data;
  Data                  * const d;
};

} // End of namespace

#endif
