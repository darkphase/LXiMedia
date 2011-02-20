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

namespace LXiMediaCenter {


class ImdbClient : public QObject
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

public:
                                ImdbClient(QObject *parent);
  virtual                       ~ImdbClient();

  void                          obtainIMDBFiles(void);

  bool                          isDownloading(void);
  bool                          isAvailable(void);
  bool                          needUpdate(void);

  QString                       findEntry(const QString &title, Type);
  Entry                         readEntry(const QString &rawName);

  static const char     * const sentinelItem;

protected:
  virtual void                  customEvent(QEvent *);

private:
  void                          importIMDBDatabase(void);

  struct MoviesListLines;
  void                          readIMDBMoviesListLines(qint64);
  void                          insertIMDBMoviesListLines(const MoviesListLines &);

  struct PlotListLines;
  void                          readIMDBPlotListLines(qint64);
  void                          insertIMDBPlotListLines(const PlotListLines &);

  struct RatingListLines;
  void                          readIMDBRatingListLines(qint64);
  void                          insertIMDBRatingListLines(const RatingListLines &);
  void                          insertSentinelItem(void);

  static Entry                  decodeEntry(const QByteArray &);

private slots:
  void                          tryMirror(void);
  void                          finished(void);
  void                          error(void);

private:
  static const char     * const mirrors[];
  static const unsigned         readChunkSize;
  static const int              maxAge;
  static const int              basePriority = INT_MIN + 1;
  static const QEvent::Type     tryMirrorEventType;

  SThreadPool           * const threadPool;
  SDependency                   mutex;
  QDir                          cacheDir;
  QFile                         moviesFile;
  QFile                         plotFile;
  QFile                         ratingFile;
  int                           downloading;
  int                           available;

  QList<QFile *>                obtainFiles;
  unsigned                      useMirror, useAttempt;
  bool                          failed;
  QNetworkAccessManager * const manager;
  QNetworkReply               * reply;
};


} // End of namespace

#endif
