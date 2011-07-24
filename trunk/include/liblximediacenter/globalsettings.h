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

#ifndef LXMEDIACENTER_GLOBALSETTINGS_H
#define LXMEDIACENTER_GLOBALSETTINGS_H

#include <QtCore>
#include <QtNetwork>
#include <LXiCore>
#include <LXiStream>
#include "export.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC GlobalSettings : public QSettings
{
public:
  struct TranscodeSize
  {
    inline TranscodeSize(void) { }
    inline TranscodeSize(const QString &name, const SSize &size) : name(name), size(size) { }

    QString                     name;
    SSize                       size;
  };

  struct TranscodeChannel
  {
    inline TranscodeChannel(void) { }
    inline TranscodeChannel(const QString &name, const SAudioFormat::Channels &channels) : name(name), channels(channels) { }

    QString                     name;
    SAudioFormat::Channels      channels;
  };

public:
                                GlobalSettings(void);

public:
  _lxi_pure static QList<QHostAddress> defaultBackendInterfaces(void);
  _lxi_pure static quint16      defaultBackendHttpPort(void);
  _lxi_pure static QString      defaultDeviceName(void);

  _lxi_pure static QUuid        serverUuid(void);
  _lxi_pure static QString      settingsFile(void);
  _lxi_pure static QString      databaseFile(void);
  _lxi_pure static QString      applicationDataDir(void);

  _lxi_pure static QList<TranscodeSize> allTranscodeSizes(void);
  _lxi_pure static QString      defaultTranscodeSizeName(void);
  _lxi_pure static QString      defaultTranscodeCropName(void);
  _lxi_pure static QString      defaultEncodeModeName(void);
  _lxi_pure static QList<TranscodeChannel> allTranscodeChannels(void);
  _lxi_pure static QString      defaultTranscodeChannelName(void);
  _lxi_pure static QString      defaultTranscodeMusicChannelName(void);
  _lxi_pure static QString      defaultMusicModeName(void);
};

} // End of namespace

#endif
