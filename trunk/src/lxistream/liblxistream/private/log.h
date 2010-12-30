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

#ifndef LXSTREAM_LOG_H
#define LXSTREAM_LOG_H

#include <QtCore>
#include <QtNetwork>

namespace LXiStream {
namespace Private {

class ExceptionHandler;
class Trace;

/*! This class handles the management of logfiles, logging can be performed
    using the standard Qt qDebug(), qWarning(), qCritical() and qFatal()
    functions.
 */
class Log
{
friend class ExceptionHandler;
friend class Trace;
public:
  static void                   initialize(const QString &preferredLogDir = QString::null);

  static QStringList            errorLogFiles(void);
  static QStringList            allLogFiles(void);
  static const QString        & logFileName(void) __attribute__((pure));
  
  static void                   redirectLogMsg(const char *msg);

  static QMutex               & logMutex(void) __attribute__((pure));
  static void                   startLogMsg(const char *type) __attribute__((nonnull));
  static void                   endLogMsg(void);
  static void                   write(const char *msg) __attribute__((nonnull));
  static int                    printf(const char *format, ...) __attribute__((nonnull));

private:
  static bool                   checkLogFile(const QString &);

  static void                   logMessage(QtMsgType, const char *);

  static QString              & logDir(const QString &preferredLogDir = QString::null);
  static QStringList          & errorLogFileList(void) __attribute__((pure));
  static FILE                 * logFile(void) __attribute__((pure));
  static QNetworkAccessManager& networkAccessManager(void) __attribute__((pure));

private:
  static QtMsgHandler           defaultMsgHandler;
  static bool                   useFile, useStderr;
  static qint64                 pid;
};


class LogNetworkCleaner : public QObject
{
Q_OBJECT
public slots:
  void                          finished(QNetworkReply *);
};


} } // End of namespaces

#endif
