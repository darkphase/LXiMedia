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
  static QList<QHostAddress>    defaultBackendInterfaces(void);
  static quint16                defaultBackendHttpPort(void);
  static QString                defaultDeviceName(void);

  static QUuid                  serverUuid(void);
  static QString                settingsFile(void);
  static QString                databaseFile(void);
  static QString                applicationDataDir(void);

  static QList<TranscodeSize>   allTranscodeSizes(void);
  static QString                defaultTranscodeSizeName(void);
  static QString                defaultTranscodeCropName(void);
  static QString                defaultEncodeModeName(void);
  static QList<TranscodeChannel> allTranscodeChannels(void);
  static QString                defaultTranscodeChannelName(void);
  static QString                defaultTranscodeMusicChannelName(void);
  static bool                   defaultMusicAddBlackVideo(void);
};

} // End of namespace

#endif
