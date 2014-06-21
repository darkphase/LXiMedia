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

#ifndef TELEVISIONSERVER_H
#define TELEVISIONSERVER_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStream>
#include "epgdatabase.h"
#include "teletextserver.h"

namespace LXiMediaCenter {

class TelevisionBackend;
class ConfigServer;

class TelevisionServer : public VideoServer
{
Q_OBJECT
friend class ConfigServer;
private:
  struct Tuner
  {
    inline Tuner(STerminals::AudioVideoDevice *terminal = NULL)
        : refCount(0), terminal(terminal), transponder(0)
    {
    }

    volatile int                refCount;
    STerminals::AudioVideoDevice * terminal;
    quint64                     transponder;
  };

  struct CaptureStream
  {
    volatile int                refCount;
    QString                     fileName;
    QString                     rawChannel;
    QDateTime                   start;

    Tuner                     * tuner;
    SGraph                    * graph;
    SNode                     * captureNode;
    SNode                     * teletextNode;
    SAudioEncoderNode         * audioEncoder;
    SNodes::Video::WideScreenDetect * wideScreenDetect;
    SNodes::Video::AnalogVideoFilter * analogVideoFilter;
    SVideoEncoderNode         * videoEncoder;
    STerminals::FileStream    * sinkTerminal;
    SNode                     * sinkNode;
  };

  struct Stream : public VideoServer::Stream
  {
    QString                     rawChannel;
    QDateTime                   start;
    int                         startPos;

    CaptureStream             * captureStream;
  };

  struct Record
  {
    inline Record(void) : priority(0.0), isScheduled(false), captureStream(NULL) { }

    QString                     programmeName;
    QString                     rawChannel;
    QString                     tunerName;
    QDateTime                   begin;
    QDateTime                   end;

    qreal                       priority;
    bool                        isScheduled;
    CaptureStream             * captureStream;
  };

private:
  class ChannelDir : public DlnaServerDir
  {
  public:
                                ChannelDir(DlnaServer *, TelevisionServer *, const QString &name);

    virtual const DirMap      & listDirs(void);
    virtual const FileMap     & listFiles(void);

  private:
    TelevisionServer    * const parent;
    const QString               name;
    QString                     rawChannel;

    QTime                       lastDirsUpdate;
    QTime                       lastFilesUpdate;
  };

public:
                                TelevisionServer(EpgDatabase *, TeletextServer *, MasterServer *server, TelevisionBackend *);
  virtual                       ~TelevisionServer();

  inline bool                   hasTuners(void) const                           { return !tuners.isEmpty(); }
  QMap<QString, QSet<QString> > tunersByChannel(void) const;
  bool                          areSharingTransponder(const QString &, const QString &, const QString &);
  bool                          willRecord(const QString &channelName, const QDateTime &, const QDateTime &) const;

  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);
  virtual SearchResultList      search(const QStringList &) const;

protected:
  virtual bool                  streamVideo(const QHttpRequestHeader &, QAbstractSocket *, const StreamRequest &);
  virtual void                  removeStream(VideoServer::Stream *, unsigned);

private slots:
  void                          updateRecordPlan(void);
  void                          checkSchedule(void);
  void                          checkTeletextCaptures(void);
  void                          checkDiskspace(void);

private:
  QImage                        getThumbnail(const QString &id);
  bool                          handleHtmlRequest(const QUrl &, const QString &, QAbstractSocket *);

  CaptureStream               * startCapture(const QString &, const QString & = QString::null);
  void                          stopCapture(CaptureStream *);
  CaptureStream               * startTeletextCapture(const QString &);
  void                          stopTeletextCapture(CaptureStream *);
  QPair<Tuner *, quint64>       findAvailableTuner(const QString &rawName, bool, const QString & = QString::null);
  qint64                        getFreeDiskSpace(void);

  void                          updateRecordPlanTask(void);

  static QDateTime              roundTime(const QDateTime &);
  static QString                toID(const QString &, const QDateTime &utcDate, const QTime &);
  static bool                   fromID(const QString &, QString &, QDateTime &utcDate);

private:
  static const qint64           reservedSpace = Q_INT64_C(2) * Q_INT64_C(1073741824);
  static const int              maxStreams = 64;
  static const qreal            manualPriorityBoost = 10000.0;
  static const int              startEarlierSecs = 2 * 60;
  static const int              stopLaterSecs = 5 * 60;

  TelevisionBackend     * const plugin;
  EpgDatabase           * const epgDatabase;
  TeletextServer        * const teletextServer;
  QByteArray                    head;
  QDir                          timeshiftDir;
  bool                          timeshiftEnabled;
  QMap<QString, CaptureStream *> captureStreams;
  QMap<QString, CaptureStream *> teletextCaptureStreams;
  QMultiMap<QDateTime, Record>  records;
  bool                          buildingPlan;
  QTimer                        checkScheduleTimer;
  QTimer                        checkTeletextCapturesTimer;
  QTimer                        checkDiskspaceTimer;

  QMap<QString, Tuner *>        tuners;
  QMultiMap<int, QString>       tunerPrio;

private:
  static const int              epgNumCols, epgSecsPerCol;
  static const float            epgColWidth, epgRowHeight;
  static const float            epgChannelNameWidth;
  static const float            epgCellPadding, epgCellSpacing;

  static const char     * const cssEpg;

  static const char     * const htmlEpgMain;
  static const char     * const htmlEpgTime;
  static const char     * const htmlEpgTimeSpacer;
  static const char     * const htmlEpgHeadRow;
  static const char     * const htmlEpgHeadCol;
  static const char     * const htmlEpgChannel;
  static const char     * const htmlEpgProgramme;
  static const char     * const htmlEpgNoProgramme;
  static const char     * const htmlProgrammeItem;
  static const char     * const htmlProgramme;
  static const char     * const htmlProgrammeAction;
  static const char     * const htmlDays;
  static const char     * const htmlDaysItem;
  static const char     * const htmlDaysCurrentItem;
};

enum Type { Type_Unknown = 0, Type_Television, Type_Radio };
QString                         typeName(Type);

} // End of namespace

#endif
