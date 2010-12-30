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

#include "sterminal.h"

namespace LXiStream {


STerminal::STerminal(QObject *parent)
          :QObject(parent)
{
}

/*! Sets a property on the node. See QObject::setProperty().
 */
bool STerminal::setProperty(const char *name, const QVariant &value)
{
  return QObject::setProperty(name, value);
}

/*! Sets a property on the node. See QObject::property().
 */
QVariant STerminal::property(const char *name) const
{
  return QObject::property(name);
}

QStringList STerminal::inputs(void) const
{
  return QStringList() << QString::null;
}

bool STerminal::selectInput(const QString &)
{
  return true;
}

QStringList STerminal::outputs(void) const
{
  return QStringList() << QString::null;
}

bool STerminal::selectOutput(const QString &)
{
  return true;
}

STuner * STerminal::STerminal::tuner(void) const
{
  return NULL;
}

STerminal::Stream STerminal::inputStream(quint64 serviceID) const
{
  foreach (const Stream &stream, inputStreams())
  if (stream.serviceID == serviceID)
    return stream;

  return Stream();
}

STerminal::Stream STerminal::outputStream(quint64 serviceID) const
{
  foreach (const Stream &stream, outputStreams())
  if (stream.serviceID == serviceID)
    return stream;

  return Stream();
}


} // End of namespace
