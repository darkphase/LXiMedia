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
#ifndef TRAYICON_ONLY
#include <LXiStream>
#endif

namespace LXiMediaCenter {

class GlobalSettings : public QSettings
{
public:
#ifndef TRAYICON_ONLY
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
#endif

public:
                                GlobalSettings(void);

public:
  static QList<QHostAddress>    defaultBackendInterfaces(void) __attribute__((pure));
  static quint16                defaultBackendHttpPort(void) __attribute__((pure));

  static QUuid                  serverUuid(void) __attribute__((pure));
  static QString                settingsFile(void) __attribute__((pure));
  static QString                databaseFile(void) __attribute__((pure));
  static QString                applicationDataDir(void) __attribute__((pure));

#ifndef TRAYICON_ONLY
  static QList<TranscodeSize>   allTranscodeSizes(void) __attribute__((pure));
  static QString                defaultTranscodeSizeName(void) __attribute__((pure));
  static QString                defaultTranscodeCropName(void) __attribute__((pure));
  static QString                defaultEncodeModeName(void) __attribute__((pure));
  static QList<TranscodeChannel> allTranscodeChannels(void) __attribute__((pure));
  static QString                defaultTranscodeChannelName(void) __attribute__((pure));
  static QString                defaultTranscodeMusicChannelName(void) __attribute__((pure));
#endif
};

} // End of namespace

#endif
