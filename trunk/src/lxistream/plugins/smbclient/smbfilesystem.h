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

#ifndef SMBFILESYSTEM_H
#define SMBFILESYSTEM_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace SMBClientBackend {

/*! Implements a SMB/CIFS filesystem. Uses libsmbclient on unix and UNC paths
    on Windows.
 */
class SMBFilesystem : public SInterfaces::Filesystem
{
Q_OBJECT
private:
#if !defined(Q_OS_WIN) || defined(USE_LIBSMBCLIENT_ON_WIN)
  class File : public QIODevice
  {
  public:
    explicit                    File(const QUrl &);
    virtual                     ~File();

    virtual bool                open(OpenMode mode);
    virtual void                close(void);
    virtual bool                isSequential(void) const;
    virtual bool                seek(qint64 pos);
    virtual qint64              size(void) const;

  protected:
    virtual qint64              readData(char *data, qint64 maxSize);
    virtual qint64              writeData(const char * data, qint64 maxSize);

  private:
    const QUrl                  path;
    int                         fd;
  };
#endif

public:
  static void                   init(void);
  static void                   close(void);
#if !defined(Q_OS_WIN) || defined(USE_LIBSMBCLIENT_ON_WIN)
  static QByteArray             version(void);
#endif

                                SMBFilesystem(const QString &, QObject *parent);

public: // From SInterfaces::Filesystem
  virtual bool                  openDirectory(const QUrl &);

  virtual QStringList           entryList(QDir::Filters, QDir::SortFlags) const;
  virtual QUrl                  filePath(const QString &fileName) const;
  virtual Info                  readInfo(const QString &fileName) const;
  virtual QIODevice           * openFile(const QString &fileName) const;

public:
  static const char             scheme[];

private:
#if !defined(Q_OS_WIN) || defined(USE_LIBSMBCLIENT_ON_WIN)
  static QMutex               * mutex(void);
  static QMap<QString, QUrl>  & paths(void);
  static void                   authenticate(const char *srv, const char *shr,
                                             char *wg, int wglen,
                                             char *un, int unlen,
                                             char *pw, int pwlen);
#endif

private:
  QUrl                          path;
#if defined(Q_OS_WIN) && !defined(USE_LIBSMBCLIENT_ON_WIN)
  QDir                          dir;
#endif
};

} } // End of namespaces

#endif
