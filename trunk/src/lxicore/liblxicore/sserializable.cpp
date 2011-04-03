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

#include "sserializable.h"


namespace LXiCore {


SSerializable::SSerializable(void)
{
}

SSerializable::~SSerializable()
{
}

QDomElement SSerializable::createElement(QDomDocument &doc, const QString &name)
{
  return doc.createElement(name);
}

QDomElement SSerializable::findElement(const QDomNode &elm, const QString &name)
{
  if (elm.isElement())
  {
    const QDomElement e = elm.toElement();

    return (e.tagName() == name) ? e : e.firstChildElement(name);
  }
  else
    return elm.firstChildElement(name);
}

void SSerializable::addElement(QDomDocument &doc, QDomNode &elm, const QString &name, const QString &value)
{
  QDomElement e = doc.createElement(name);
  e.appendChild(doc.createTextNode(value));
  elm.appendChild(e);
}

QString SSerializable::element(const QDomNode &elm, const QString &name)
{
  const QDomElement te = elm.firstChildElement(name);

  if (!te.isNull())
    return te.text();
  else
    return QString::null;
}

QByteArray SSerializable::toByteArray(int indent) const
{
  QDomDocument doc("");
  doc.appendChild(toXml(doc));

  return doc.toByteArray(indent);
}

void SSerializable::fromByteArray(const QByteArray &text)
{
  QDomDocument doc("");
  doc.setContent(text);
  fromXml(doc.documentElement());
}

} // End of namespace
