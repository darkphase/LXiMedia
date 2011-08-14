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

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

class MediaDatabase;

class ConfigServer : public BackendServer,
                     protected SHttpServer::Callback
{
Q_OBJECT
public:
                                ConfigServer(const QString &, QObject * = NULL);
  virtual                       ~ConfigServer();

  virtual void                  initialize(MasterServer *);
  virtual void                  close(void);

  virtual QString               pluginName(void) const;
  virtual QString               serverName(void) const;
  virtual QString               serverIconPath(void) const;

  _lxi_pure static const QSet<QString> & hiddenDirs(void);
  static bool                   isHidden(const QString &path);

protected: // From SHttpServer::Callback
  virtual SHttpServer::ResponseMessage httpRequest(const SHttpServer::RequestMessage &, QIODevice *);

private:
  SHttpServer::ResponseMessage  handleHtmlRequest(const SHttpServer::RequestMessage &, const MediaServer::File &);
  void                          generateDirs(HtmlParser &, const QFileInfoList &, int, const QStringList &, const QStringList &);

  void                          scanDrives(void);

private:
  static const Qt::CaseSensitivity caseSensitivity;
  static const char             dirSplit;
  MasterServer                * masterServer;
  MediaDatabase               * mediaDatabase;

  QMap<QString, QFileInfo>      driveInfoList;
  QMap<QString, QString>        driveLabelList;

private:
  static const char     * const htmlMain;
  static const char     * const htmlDirTreeIndex;
  static const char     * const htmlDirTreeDir;
  static const char     * const htmlDirTreeIndent;
  static const char     * const htmlDirTreeExpand;
  static const char     * const htmlDirTreeCheck;
  static const char     * const htmlDirTreeCheckLink;
};

} } // End of namespaces

#endif
