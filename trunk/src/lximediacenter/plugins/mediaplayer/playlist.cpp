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

#include "playlist.h"

namespace LXiMediaCenter {

Playlist::Playlist(MediaDatabase *mediaDatabase, QObject *parent)
  : QObject(parent),
    mediaDatabase(mediaDatabase),
    mutex(QMutex::Recursive),
    allFiles(false),
    played(0)
{
  // Seed the random number generator.
  qsrand(int(QDateTime::currentDateTime().toTime_t()));
}

Playlist::~Playlist()
{
}

void Playlist::append(MediaDatabase::UniqueID uid)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (!allFiles)
    list.append(uid);
  else
    pending += uid;
}

void Playlist::remove(MediaDatabase::UniqueID uid)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (!allFiles)
  {
    for (QVector<Entry>::Iterator i=list.begin(); i!=list.end(); )
    if (i->uid == uid)
    {
      if (i->played)
        played--;

      i = list.erase(i);
    }
    else
      i++;
  }
  else
    pending.removeAll(uid);
}

void Playlist::clear(void)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  if (!allFiles)
    list.clear();

  played = 0;
}

int Playlist::count(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  return list.count();
}

MediaDatabase::UniqueID Playlist::checkout(void)
{
  SDebug::MutexLocker ml(&mutex, __FILE__, __LINE__);

  if (allFiles)
  {
    next();

    return pending.takeFirst();
  }
  else
    return random();
}

QList<MediaDatabase::UniqueID> Playlist::next(void)
{
  SDebug::MutexLocker ml(&mutex, __FILE__, __LINE__);

  if (allFiles)
  {
    while (pending.count() < 5)
    {
      const MediaDatabase::UniqueID uid = random();
      if (uid != 0)
        pending += uid;
      else
        break;
    }

    return pending;
  }
  else
  {
    QList<MediaDatabase::UniqueID> result;

    for (int i=0, n=list.count(); i<n; i++)
      result += list[i].uid;

    return result;
  }
}

MediaDatabase::UniqueID Playlist::random(void)
{
  const int fresh = list.count() - played;
  if (fresh > 0)
  {
    int next = qrand() % fresh;

    for (int i=0, n=list.count(); i<n; i++)
    if (!list[i].played && (next-- == 0))
    {
      list[i].played = true;
      played++;

      if (!allFiles)
        return list[i].uid;
      else
        return mediaDatabase->getSong(i);
    }
  }

  return 0;
}

QByteArray Playlist::serialize(void) const
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  QByteArray data = "#EXTM3U\n\n";

  for (int i=0, n=list.count(); i<n; i++)
  {
    const SMediaInfo node = mediaDatabase->readNode(!allFiles ? list[i].uid : mediaDatabase->getSong(i));
    if (!node.isNull())
    {
      data.append("#EXTINF:");
      if (node.duration().isPositive())
        data.append(QByteArray::number(node.duration().toSec()));
      else
        data.append("-1");

      if (!node.author().isEmpty())
        data.append(',' + node.author());
      else
        data.append(",");

      if (!node.title().isEmpty())
        data.append(" - " + node.title());

      data.append('\n');
      data.append(QDir::toNativeSeparators(node.filePath()).toUtf8());
      data.append("\n\n");
    }
  }

  return data;
}

bool Playlist::deserialize(const QByteArray &data)
{
  SDebug::MutexLocker l(&mutex, __FILE__, __LINE__);

  allFiles = false;
  clear();

  foreach (QByteArray line, data.split('\n'))
  if (!line.startsWith('#'))
  {
    // Remove DOS line endings.
    if (line.endsWith('\r'))
      line = line.left(line.length() - 1);

    if (!line.isEmpty())
    {
      const MediaDatabase::UniqueID uid = mediaDatabase->fromPath(QDir::fromNativeSeparators(QString::fromUtf8(line)));
      if (uid != 0)
        list.append(uid);
    }
  }

  return !list.isEmpty();
}

Playlist * Playlist::createAllFiles(MediaDatabase *mediaDatabase, QObject *parent)
{
  Playlist * const p = new Playlist(mediaDatabase, parent);
  p->allFiles = true;
  p->list.resize(mediaDatabase->numSongs());

  return p;
}

} // End of namespace
