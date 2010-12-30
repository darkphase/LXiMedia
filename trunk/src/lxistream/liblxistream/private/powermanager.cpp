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

#include "powermanager.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace LXiStream {
namespace Private {


void PowerManager::addActiveObject(QObject *)
{
  if (activeObjects().fetchAndAddOrdered(1) == 0)
  {
    standardGovernor() = getGovernor();
    setGovernor("performance");

#ifdef Q_OS_WIN
    ::SetPriorityClass(::GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
#endif
  }
}

void PowerManager::removeActiveObject(QObject *)
{
  if (!activeObjects().deref())
  {
    setGovernor(standardGovernor());

#ifdef Q_OS_WIN
    ::SetPriorityClass(::GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
#endif
  }
}

bool PowerManager::hasActiveObjects(void)
{
  return activeObjects() != 0;
}

QStringList PowerManager::getCPUs(void)
{
  QStringList cpuList;

  #if defined (Q_OS_UNIX)
    QDir dir("/sys/devices/system/cpu/");

    foreach (QString cpu, dir.entryList())
    if (cpu.startsWith("cpu") && !cpu.endsWith("idle"))
      cpuList += cpu;
  #elif defined (Q_OS_WIN)
    for (int i=0; i<QThread::idealThreadCount(); i++)
      cpuList += QString::number(i);
  #endif

  return cpuList;
}

QStringList PowerManager::getGovernors(void)
{
  QStringList governorList;

  #ifdef Q_OS_UNIX
    foreach (QString cpu, getCPUs())
    {
      QFile file("/sys/devices/system/cpu/" + cpu + "/cpufreq/scaling_available_governors");
      if (file.open(QIODevice::ReadOnly))
      foreach (QString governor, QString(file.readAll()).split(' ', QString::SkipEmptyParts))
      if (!governorList.contains(governor))
        governorList += governor;
    }
  #endif

  return governorList;
}

QString PowerManager::getGovernor(void)
{
  const QStringList cpus = getCPUs();
  if (!cpus.isEmpty())
    return getGovernor(cpus.first());
  else
    return QString::null;
}

QString PowerManager::getGovernor(const QString &cpu)
{
  #ifdef Q_OS_UNIX
    QFile file("/sys/devices/system/cpu/" + cpu + "/cpufreq/scaling_governor");
    if (file.open(QIODevice::ReadOnly))
      return file.readAll().trimmed();
  #endif

  return QString::null;
}

bool PowerManager::setGovernor(const QString &governor)
{
  #ifdef Q_OS_UNIX
    if (getGovernors().contains(governor))
    foreach (QString cpu, getCPUs())
    {
      QFile file("/sys/devices/system/cpu/" + cpu + "/cpufreq/scaling_governor");
      if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        file.write((governor + "\n").toAscii());
    }
  #endif

  return true;
}

QAtomicInt & PowerManager::activeObjects(void)
{
  static QAtomicInt i = 0;

  return i;
}

QString & PowerManager::standardGovernor(void)
{
  static QString g = "ondemand";

  return g;
}


} } // End of namespaces
