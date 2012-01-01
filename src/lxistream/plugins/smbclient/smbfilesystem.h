/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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

#ifndef SMBFILESYSTEM_H
#define SMBFILESYSTEM_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace SMBClientBackend {

class SMBFilesystem : public SInterfaces::Filesystem
{
Q_OBJECT
private:
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

public:
  static void                   init(void);
  static void                   close(void);
  static QByteArray             version(void);

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
  static QMutex               * mutex(void);
  static QMap<QString, QUrl>  & paths(void);
  static void                   authenticate(const char *srv, const char *shr,
                                             char *wg, int wglen,
                                             char *un, int unlen,
                                             char *pw, int pwlen);

private:
  QUrl                          path;
};

} } // End of namespaces

#endif
