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
  QByteArray data;
  for (int i=0, n=list.count(); i<n; i++)
    data += QDir::toNativeSeparators(list[i].filePath).toUtf8() + "\r\n";

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
