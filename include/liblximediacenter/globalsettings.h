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
#ifndef TRAYICON_ONLY
#include <LXiStream>
#endif
#include "export.h"

namespace LXiMediaCenter {

class LXIMEDIACENTER_PUBLIC GlobalSettings : public QSettings
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
  pure static QList<QHostAddress> defaultBackendInterfaces(void);
  pure static quint16           defaultBackendHttpPort(void);

  pure static QUuid             serverUuid(void);
  pure static QString           settingsFile(void);
  pure static QString           databaseFile(void);
  pure static QString           applicationDataDir(void);

#ifndef TRAYICON_ONLY
  pure static QList<TranscodeSize> allTranscodeSizes(void);
  pure static QString           defaultTranscodeSizeName(void);
  pure static QString           defaultTranscodeCropName(void);
  pure static QString           defaultEncodeModeName(void);
  pure static QList<TranscodeChannel> allTranscodeChannels(void);
  pure static QString           defaultTranscodeChannelName(void);
  pure static QString           defaultTranscodeMusicChannelName(void);
#endif
};

} // End of namespace

#endif
