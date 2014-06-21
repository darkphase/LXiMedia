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

#ifndef SCAN_H
#define SCAN_H

#include <QtCore>
#include <LXiStream>
#include <LXiStreamGui>
#include "televisionbackend.h"
#include "televisionserver.h"

namespace LXiMediaCenter {

class Scan : public QObject
{
Q_OBJECT
public:
  struct Channel
  {
    inline                      Channel(Type type,
                                        quint64 frequency, quint64 serviceID,
                                        const QString &name,
                                        const QString &programme,
                                        const QImage &thumbnail)
    : type(type), frequency(frequency), serviceID(serviceID), name(name),
      programme(programme), thumbnail(thumbnail) { }

    Type                        type;
    quint64                     frequency;
    quint64                     serviceID;
    QString                     name, programme;
    QImage                      thumbnail;
  };

  typedef QList<Channel>        ChannelList;

private:
  class ScanNode : public SNode
  {
  public:
    inline                      ScanNode(Scan *parent)
        : SNode(Behavior_None, 0, SImageBuffer::toImageCodecs(), parent),
        parent(parent) { }

  public: // From SNode
    inline virtual bool         prepare(const SCodecList &)                     { return true; }
    inline virtual bool         unprepare(void)                                 { parent->snapshot.takeDataOwnership(); return true; }
    virtual Result              processBuffer(const SBuffer &, SBufferList &);

  private:
    Scan                * const parent;
  };

public:
                                Scan(STerminal *, const QString &, quint64, QObject *);
  virtual                       ~Scan();

  inline bool                   isDvb(void) const                               { return dvb; }
  inline bool                   isBusy(void) const                              { return (signalTimer >= 0); }
  inline qreal                  completed(void) const                           { return qreal(frequency - startFrequency) / qreal(stopFrequency - startFrequency); }
  inline quint64                scanFrequency(void) const                       { return frequency; }
  inline const STerminal      * device(void) const                              { return terminal; }
  inline const QString        & deviceName(void) const                          { return devName; }
  inline const ChannelList    & channels(void) const                            { return channelsFound; }
  QImage                        signalPlot(void) const;
  QImage                        lastSnapshot(void) const;
  inline QString                lastChannel(void) const                         { return channelName; }
  inline QString                lastProgramme(void) const                       { return programmeName; }
  inline QString                lastProvider(void) const                        { return providerName; }

  inline bool                   setFrequency(quint64 f)                         { return tuner->setFrequency(f); }
  void                          stop(void);

protected:
  virtual void                  timerEvent(QTimerEvent *);

private:
  void                          parseChannels(void);

private:
  const QString                 devName;
  mutable QReadWriteLock        lock;
  SGraph                        graph;
  STerminal             * const terminal;
  STuner                * const tuner;
  SNode                       * captureNode;
  ScanNode                      scanNode;
  QQueue<STuner::Status>        statusHistory;
  SImageBuffer                  snapshot;
  QString                       channelName;
  QString                       programmeName;
  QString                       providerName;
  quint64                       frequency;
  quint64                       lastFrequency;
  quint64                       startFrequency;
  quint64                       stopFrequency;
  quint64                       stepFrequency;
  quint64                       bigStepFrequency;
  int                           scanTimer;
  int                           signalTimer;
  enum { Coarse, FineIn, FineOut, Name } mode;
  bool                          dvb;
  bool                          graphStarted;

  quint64                       fineBeginFrequency;
  int                           nameSettleTime;

  ChannelList                   channelsFound;
};

} // End of namespace

#endif
