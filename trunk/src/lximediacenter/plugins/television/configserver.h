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

#ifndef CONFIGSERVER_H
#define CONFIGSERVER_H

#include <QtCore>
#include <LXiMediaCenter>
#include <LXiStream>
#include "epgdatabase.h"

namespace LXiMediaCenter {

class Scan;
class TelevisionBackend;
class TelevisionServer;

class ConfigServer : public BackendServer
{
Q_OBJECT
private:
  class StartScanEvent : public QEvent
  {
  public:
    explicit                    StartScanEvent(const QString &device, quint64 freq = 0) : QEvent(startScanEventType), device(device), freq(freq) { }

    const QString               device;
    const quint64               freq;
  };

  class SetFreqEvent : public QEvent
  {
  public:
    explicit                    SetFreqEvent(quint64 freq = 0) : QEvent(setFreqEventType), freq(freq) { }

    const quint64               freq;
  };

public:
                                ConfigServer(TelevisionServer *, MasterServer *server, TelevisionBackend *);

  virtual bool                  handleConnection(const QHttpRequestHeader &, QAbstractSocket *);

protected:
  virtual void                  customEvent(QEvent *);

private:
  bool                          addChannels(const QHttpRequestHeader &, QAbstractSocket *, const QUrl &);
  static void                   addChannel(PluginSettings &, const QString &, quint64, quint16, const QString &, const QString &);
  bool                          updateChannels(const QHttpRequestHeader &, QAbstractSocket *, const QUrl &);
  static void                   updateChannel(PluginSettings &, const QString &, int, const QString &, int, const QStringList &);
  static void                   copyPresets(PluginSettings &, const QString &, int, int);
  bool                          sendScanSnapshot(QAbstractSocket *, quint64, int, int) const;
  bool                          sendSignalPlot(QAbstractSocket *) const;
  bool                          handleHtmlRequest(const QUrl &, const QString &, QAbstractSocket *);

private:
  static const QEvent::Type     startScanEventType;
  static const QEvent::Type     stopScanEventType;
  static const QEvent::Type     setFreqEventType;

  TelevisionBackend      * const plugin;
  TelevisionServer      * const televisionServer;
  mutable QReadWriteLock        lock;

  SGraph                        graph;

  Scan               * volatile runningScan;

private:
  static const char     * const htmlMain;
  static const char     * const htmlAnalogScan;
  static const char     * const htmlDigitalScan;
  static const char     * const htmlTunerItem;
  static const char     * const htmlChannelItem;
  static const char     * const htmlFoundChannelItem;
  static const char     * const htmlOption;
  static const char     * const htmlDeviceItem;
};

} // End of namespace

#endif
