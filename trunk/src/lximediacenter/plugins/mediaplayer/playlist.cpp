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
namespace MediaPlayerBackend {

Playlist::Playlist(MediaDatabase *mediaDatabase, QObject *parent)
  : QObject(parent),
    mediaDatabase(mediaDatabase),
    played(0)
{
  // Seed the random number generator.
  qsrand(int(QDateTime::currentDateTime().toTime_t()));
}

Playlist::~Playlist()
{
}

void Playlist::append(const QString &filePath)
{
  list.append(Entry(filePath));
}

void Playlist::remove(const QString &filePath)
{
  for (QVector<Entry>::Iterator i=list.begin(); i!=list.end(); )
  if (i->filePath == filePath)
  {
    if (i->played)
      played--;

    i = list.erase(i);
  }
  else
    i++;
}

void Playlist::clear(void)
{
  list.clear();
  played = 0;
}

int Playlist::count(void) const
{
  return list.count();
}

QString Playlist::checkout(void)
{
  return random();
}

QStringList Playlist::next(void)
{
  QStringList result;
  for (int i=0, n=list.count(); i<n; i++)
    result += list[i].filePath;

  return result;
}

QString Playlist::random(void)
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

      return list[i].filePath;
    }
  }

  return QString::null;
}

QByteArray Playlist::serialize(void) const
{
  QByteArray data = "#EXTM3U\n\n";

  for (int i=0, n=list.count(); i<n; i++)
  {
    const FileNode node = mediaDatabase->readNode(list[i].filePath);
    if (!node.isNull())
    {
      data.append("#EXTINF:");
      if (node.duration().isPositive())
        data.append(QByteArray::number(node.duration().toSec()));
      else
        data.append("-1");

      const QString author = node.metadata("author").toString();
      if (!author.isEmpty())
        data.append(',' + author);
      else
        data.append(",");

      const QString title = node.metadata("title").toString();
      if (!title.isEmpty())
        data.append(" - " + title);

      data.append('\n');
      data.append(QDir::toNativeSeparators(node.filePath()).toUtf8());
      data.append("\n\n");
    }
  }

  return data;
}

bool Playlist::deserialize(const QByteArray &data)
{
  clear();

  foreach (QByteArray line, data.split('\n'))
  if (!line.startsWith('#'))
  {
    // Remove DOS line endings.
    if (line.endsWith('\r'))
      line = line.left(line.length() - 1);

    if (!line.isEmpty())
      list.append(Entry(QDir::fromNativeSeparators(QString::fromUtf8(line))));
  }

  return !list.isEmpty();
}

} } // End of namespaces
