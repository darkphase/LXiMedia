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

#ifndef LXICORE_SSERIALIZABLE_H
#define LXICORE_SSERIALIZABLE_H

#include <QtCore>
#include <QtXml>
#include "sglobal.h"

namespace LXiCore {

class S_DSO_PUBLIC SSerializable
{
public:
  virtual                       ~SSerializable();

  virtual QDomNode              toXml(QDomDocument &) const = 0;
  virtual void                  fromXml(const QDomNode &) = 0;

  QByteArray                    toByteArray(int indent = 1) const;
  void                          fromByteArray(const QByteArray &);

protected:
                                SSerializable(void);

  static QDomElement            createElement(QDomDocument &doc, const QString &name);
  static QDomElement            findElement(const QDomNode &elm, const QString &name);
  static void                   addElement(QDomDocument &doc, QDomNode &elm, const QString &name, const QString &value);
  static QString                element(const QDomNode &elm, const QString &name);

  static inline const char    * trueFalse(bool b)                               { return b ? "true" : "false"; }
  static inline bool            trueFalse(const QString &s)                     { return s == "true"; }
};

/*! This list template adds a toXml() and fromXml() feature to QList.
 */
template <typename _type>
class S_DSO_PUBLIC SSerializableList : public QList<_type>,
                                     public SSerializable
{
public:
  inline                        SSerializableList(void) : QList<_type>() { }
  inline                        SSerializableList(const QList<_type> &c) : QList<_type>(c) { }
  inline                        SSerializableList(const SSerializableList<_type> &c) : QList<_type>(c) { }

  virtual QDomNode              toXml(QDomDocument &) const;
  virtual void                  fromXml(const QDomNode &);
};


template <typename _type>
QDomNode SSerializableList<_type>::toXml(QDomDocument &doc) const
{
  QDomElement list = createElement(doc, "list");

  foreach (const _type &element, *this)
    list.appendChild(element.toXml(doc));

  return list;
}

template <typename _type>
void SSerializableList<_type>::fromXml(const QDomNode &elm)
{
  QDomElement list = findElement(elm, "list");

  QList<_type>::clear();
  for (QDomNode n=list.firstChild(); !n.isNull(); n=n.nextSibling())
  {
    QDomElement e = n.toElement();
    if (!e.isNull())
    {
      _type item;
      item.fromXml(e);
      append(item);
    }
  }
}

} // End of namespace

#endif
