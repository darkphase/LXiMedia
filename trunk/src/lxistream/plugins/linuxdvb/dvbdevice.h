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

#ifndef LINUXDVBBACKEND_DVBDEVICE_H
#define LINUXDVBBACKEND_DVBDEVICE_H

#include <QtCore>
#include <LXiStream>
#include <liblxistream/common/mpeg.h>

namespace LXiStream {
namespace LinuxDvbBackend {

class DVBInput;
class DVBTuner;

class DVBDevice : public STerminals::AudioVideoDevice,
                  private Common::MPEG::Stream
{
Q_OBJECT
friend class DVBInput;
friend class DVBTuner;
public:
  struct Channel
  {
    inline                      Channel(void) : serviceID(0), videoPID(0), teletextPID(0), subtitlesPID(0), ac3PID(0) { }

    quint16                     serviceID;
    QString                     provider;
    QString                     name;
    QVector<quint16>            audioPIDs;
    quint16                     videoPID;
    quint16                     teletextPID;
    quint16                     subtitlesPID;
    quint16                     ac3PID;
  };

  struct Program
  {
    inline                      Program(void) : audioPID(0), videoPID(0), dataPID(0) { }

    quint16                     audioPID;
    quint16                     videoPID;
    quint16                     dataPID;
  };

  struct Filter
  {
    inline                      Filter(void) : pid(0), tid(0), tidExt(0), timeout(0) { }
    inline                      Filter(unsigned pid, unsigned tid, unsigned tidExt, unsigned timeout) : pid(pid), tid(tid), tidExt(tidExt), timeout(timeout) { }

    unsigned                    pid;
    unsigned                    tid;
    unsigned                    tidExt;
    unsigned                    timeout;
  };

private:
  class Thread : public QThread
  {
  public:
    inline explicit             Thread(DVBDevice *parent) : parent(parent) { }

  protected:
    virtual void                run(void);

  private:
    DVBDevice           * const parent;
  };

  class RetuneEvent : public QEvent
  {
  public:
    inline RetuneEvent(QSemaphore *sem)
      : QEvent(retuneEventType), sem(sem)
    {
    }

    QSemaphore          * const sem;
  };

  class ProcessDemuxEvent : public QEvent
  {
  public:
    inline ProcessDemuxEvent(QSemaphore *sem, const quint8 *data)
      : QEvent(processDemuxEventType), sem(sem), data(data)
    {
    }

    QSemaphore          * const sem;
    const quint8        * const data;
  };

public:
  static SSystem::DeviceEntryList listDevices(void);

public:
                                DVBDevice(QObject *parent);
  virtual                       ~DVBDevice();

  virtual inline bool           audioEnabled(void) const                        { return outputAudio; }
  virtual inline void           setAudioEnabled(bool e)                         { outputAudio = e; }
  virtual inline bool           videoEnabled(void) const                        { return outputVideo; }
  virtual inline void           setVideoEnabled(bool e)                         { outputVideo = e; }

public: // From STerminal
  virtual bool                  open(const QUrl &);

  virtual QString               friendlyName(void) const;
  virtual QString               longName(void) const;
  virtual Types                 terminalType(void) const;

  virtual QStringList           inputs(void) const;
  virtual bool                  selectInput(const QString &);
  virtual STuner              * tuner(void) const;

  virtual QList<STerminal::Stream> inputStreams(void) const;
  virtual STerminal::Stream     inputStream(quint64 serviceID) const;
  virtual QList<STerminal::Stream> outputStreams(void) const;
  virtual SNode               * openStream(const STerminal::Stream &);

public:
  static dmx_pes_type_t         getPESType(dmx_pes_type_t, unsigned);

protected:
  virtual void                  customEvent(QEvent *);

private:
  void                          tsPacketReceived(const Common::MPEG::TSPacket *);
  static bool                   setFilter(int, const Filter &);
  bool                          findDescriptor(quint8, const uchar *, unsigned);

  void                          parseSection(const uchar *);
  void                          parsePMT(const uchar *, unsigned, Channel *);
  void                          parseSDT(const uchar *, unsigned);
  void                          parseServiceDesc(const uchar *, unsigned, Channel *);

private:
  static const QEvent::Type     retuneEventType;
  static const QEvent::Type     processDemuxEventType;

  static const unsigned         bufferSize = 1024 * 1024;
  static const unsigned         numStreams = 4;
  static QMap<QString, int>     foundDevices;
  static bool                   initialized;
  static quint8                 reversedBits[256];

  bool                          outputAudio;
  bool                          outputVideo;

  bool                          running;
  int                           adapterID;
  mutable QMutex                mutex;
  Thread                        thread;
  dvb_frontend_info             frontendInfo;
  int                           frontendDesc;
  int                           demuxDesc;
  int                           dvrDesc;

  bool                          retuned;
  QTime                         lastTune;
  QMap<quint16, Channel>        channels;
  QQueue<Filter>                filters;

  Common::MPEG::TSPacketStream  tsPacketStream;
  Common::MPEG::PESPacketStream * volatile pesPacketStream[Common::MPEG::maxPID + 1];

  DVBInput           * volatile inputDev[numStreams];
  DVBTuner                    * tunerDev;
};


} } // End of namespaces

#endif
