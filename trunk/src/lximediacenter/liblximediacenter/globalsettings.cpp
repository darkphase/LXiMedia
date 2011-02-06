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

#include "globalsettings.h"
#include <QtGui/QImage>

namespace LXiMediaCenter {

GlobalSettings::GlobalSettings(void)
               :QSettings(settingsFile(), QSettings::IniFormat)
{
}

const char * GlobalSettings::productAbbr(void)
{
  return "lximc";
}

const char * GlobalSettings::productUri(void)
{
  return "https://sourceforge.net/projects/lximedia/";
}

const QImage & GlobalSettings::productLogo(void)
{
  static const QImage logo(":/lximediacenter/appicon-large.png");

  return logo;
}

/*! Returns the version identifier for the active build of LXiMediaCenter.
 */
const char * GlobalSettings::version(void)
{
  return
#include "version.h"
      " (" __DATE__ " " __TIME__")";
}

QList<QHostAddress> GlobalSettings::defaultBackendInterfaces(void)
{
  QList<QHostAddress> interfaces;

  foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
  if (address.protocol() == QAbstractSocket::IPv4Protocol)
  {
    const quint32 addr = address.toIPv4Address();
    if (((addr & 0xFF000000u) == 0x0A000000u) || //10.0.0.0/8
        ((addr & 0xFF000000u) == 0x7F000000u) || //127.0.0.0/8
        ((addr & 0xFFFF0000u) == 0xA9FE0000u) || //169.254.0.0/16
        ((addr & 0xFFF00000u) == 0xAC100000u) || //172.16.0.0/12
        ((addr & 0xFFFF0000u) == 0xC0A80000u))   //192.168.0.0/16
    {
      interfaces += address;
    }
  }

  return interfaces;
}

quint16 GlobalSettings::defaultBackendHttpPort(void)
{
  return 4280;
}

QString GlobalSettings::settingsFile(void)
{
#if defined(Q_OS_UNIX)
  return QDir(applicationDataDir()).absoluteFilePath(
      QDir(QCoreApplication::applicationFilePath()).dirName() + ".conf");
#elif defined(Q_OS_WIN)
  const QString appName = QDir(QCoreApplication::applicationFilePath()).dirName();

  return QDir(applicationDataDir()).absoluteFilePath(
      appName.left(appName.length() - 4) + ".conf"); // remove .exe
#else
#error not implemented
#endif
}

QString GlobalSettings::databaseFile(void)
{
#if defined(Q_OS_UNIX)
  return QDir(applicationDataDir()).absoluteFilePath(
      QDir(QCoreApplication::applicationFilePath()).dirName() + ".db");
#elif defined(Q_OS_WIN)
  const QString appName = QDir(QCoreApplication::applicationFilePath()).dirName();

  return QDir(applicationDataDir()).absoluteFilePath(
      appName.left(appName.length() - 4) + ".db"); // remove .exe
#else
#error not implemented
#endif
}

/*! This method returns the path of the directory where this application can
    store its data. Do not use this directory for settings. The returned path is
    guaranteed not to end with a directory separator (as QDir::cleanPath()).

    \note This directory does not have to exist yet.
 */
QString GlobalSettings::applicationDataDir(void)
{
  static QString dir = QString::null;
  if (dir.length() > 0)
    return dir;

#if defined(Q_OS_UNIX)
  if (QDir::home().dirName() == productAbbr()) // homedir of the lximediacenter user
    dir = QDir::cleanPath(QDir::homePath());
  else
    dir = QDir::cleanPath(QDir::homePath() + "/." + productAbbr());
#elif defined(Q_OS_WIN)
  foreach (const QString &var, QProcess::systemEnvironment())
  if (var.startsWith("ALLUSERSPROFILE=", Qt::CaseInsensitive))
  {
    dir = QDir::cleanPath(QDir::fromNativeSeparators(var.mid(16)) + "/Application Data/" + productAbbr());
    break;
  }

  // We use Local Settings here because the backend is storing large databases
  // in this directory; this is not preferable in a roaming profile.
  if (dir.length() == 0)
    dir = QDir::cleanPath(QDir::homePath() + "/Local Settings/Application Data/" + productAbbr());
#else
  dir = QDir::cleanPath(QDir::homePath() + "/." + productAbbr());
#endif

  if (!QDir(dir).exists())
    QDir(dir).mkpath(dir);

  return dir;
}

#ifndef TRAYICON_ONLY
QList<GlobalSettings::TranscodeSize> GlobalSettings::allTranscodeSizes(void)
{
  QList<TranscodeSize> sizes;
  sizes << TranscodeSize("Webcam 4:3",      SSize(352,  288,  1.0f))
        << TranscodeSize("DVD/NTSC 4:3",    SSize(640,  480,  1.0f))
        << TranscodeSize("DVD/NTSC 16:9",   SSize(704,  480,  1.21307f))
        << TranscodeSize("DVD/PAL 4:3",     SSize(720,  576,  1.06666f))
        << TranscodeSize("DVD/PAL 16:9",    SSize(720,  576,  1.42222f))
        << TranscodeSize("HDTV 720p 16:9",  SSize(1280, 720,  1.0f))
        << TranscodeSize("HDTV 720p 21:9",  SSize(1280, 544,  1.0f))
        << TranscodeSize("HDTV 1080p 16:9", SSize(1920, 1080, 1.0f))
        << TranscodeSize("HDTV 1080p 21:9", SSize(1920, 832,  1.0f));

  return sizes;
}

QString GlobalSettings::defaultTranscodeSizeName(void)
{
  if (QThread::idealThreadCount() > 2)
    return "HDTV 720p 16:9";
  else
    return "DVD/PAL 16:9";
}

QString GlobalSettings::defaultTranscodeCropName(void)
{
  return "Box";
}

QString GlobalSettings::defaultEncodeModeName(void)
{
  if (QThread::idealThreadCount() > 3)
    return "Slow";
  else
    return "Fast";
}

QList<GlobalSettings::TranscodeChannel> GlobalSettings::allTranscodeChannels(void)
{
  QList<TranscodeChannel> channels;
  channels << TranscodeChannel("2.0 Stereo",        SAudioFormat::Channel_Stereo)
           << TranscodeChannel("3.0 Surround",      SAudioFormat::Channel_Surround_3_0)
           << TranscodeChannel("4.0 Quadraphonic",  SAudioFormat::Channel_Quadraphonic)
           << TranscodeChannel("5.0 Surround",      SAudioFormat::Channel_Surround_5_0)
           << TranscodeChannel("5.1 Surround",      SAudioFormat::Channel_Surround_5_1)
           << TranscodeChannel("7.1 Surround",      SAudioFormat::Channel_Surround_7_1);

  return channels;
}

QString GlobalSettings::defaultTranscodeChannelName(void)
{
  return "5.1 Surround";
}

QString GlobalSettings::defaultTranscodeMusicChannelName(void)
{
  return "2.0 Stereo";
}
#endif


} // End of namespace
