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

#ifndef __BUFFERREADER_H
#define __BUFFERREADER_H

#include <QtCore>
#include <LXiStream>
#define this self
#include <dvdnav/dvdnav.h>
#undef this

namespace LXiStream {
namespace DVDNavBackend {

class BufferReader : public SInterfaces::BufferReader

{
Q_OBJECT
private:
  class DvdDevice : public QIODevice
  {
  public:
    explicit                    DvdDevice(BufferReader *parent);

    QString                     discName(void) const;
    int                         numTitles(void) const;
    QList<STime>                chapters(void) const;

    quint16                     audioLanguage(int) const;
    quint16                     subtitleLanguage(int) const;
    qint8                       subtitleLogicalStream(int) const;

  public: // From QIODevice
    virtual bool                open(OpenMode mode);
    virtual void                close(void);

    virtual bool                reset(void);
    virtual bool                seek(qint64 pos);
    virtual qint64              size(void) const;

    virtual qint64              readData(char *data, qint64 maxSize);
    virtual qint64              writeData(const char *data, qint64 maxSize);

  private:
    bool                        resetStream(void);

  private:
    BufferReader        * const parent;
    ::dvdnav_t                * dvdHandle;
  };

public:
  static const char             formatName[];

  static bool                   isDiscPath(const QString &path);

  explicit                      BufferReader(const QString &, QObject *);
  virtual                       ~BufferReader();

  inline QIODevice            * getIODevice(void)                               { return &dvdDevice; }

  inline QString                discName(void) const                            { return dvdDevice.discName(); }
  bool                          selectTitle(int);
  bool                          openBufferReader(void);

  QList<AudioStreamInfo>        filterAudioStreams(const QList<AudioStreamInfo> &) const;
  QList<VideoStreamInfo>        filterVideoStreams(const QList<VideoStreamInfo> &) const;
  QList<DataStreamInfo>         filterDataStreams(const QList<DataStreamInfo> &) const;

public: // From SInterfaces::BufferReader
  virtual bool                  openFormat(const QString &);

  virtual bool                  start(QIODevice *, SInterfaces::BufferReader::ProduceCallback *, bool streamed);
  virtual void                  stop(void);
  virtual bool                  process(void);

  virtual STime                 duration(void) const;
  virtual bool                  setPosition(STime);
  virtual STime                 position(void) const;
  virtual QList<Chapter>        chapters(void) const;

  virtual int                   numTitles(void) const;
  virtual QList<AudioStreamInfo> audioStreams(int title) const;
  virtual QList<VideoStreamInfo> videoStreams(int title) const;
  virtual QList<DataStreamInfo>  dataStreams(int title) const;
  virtual void                  selectStreams(int title, const QVector<StreamId> &);

private:
  DvdDevice                     dvdDevice;

  QString                       path;
  int                           currentTitle;
  int                           currentChapter;
  QList<STime>                  titleChapters;
  STime                         titleDuration;

  SInterfaces::BufferReader::ProduceCallback * produceCallback;
  SInterfaces::BufferReader   * bufferReader;
  QVector<StreamId>             selectedStreams;

  static const unsigned         blockSize = 2048;
};

} } // End of namespaces

#endif
