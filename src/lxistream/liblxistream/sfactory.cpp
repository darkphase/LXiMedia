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

#include "sfactory.h"
#include "sapplication.h"

namespace LXiStream {

SFactory::SFactory(void)
{
  SApplication::factories().append(this);
}

SFactory::~SFactory()
{
  SApplication::factories().removeAll(this);
}

void SFactory::registerClass(const QMetaObject &metaObject, CreateFunc createFunc, const Scheme &scheme)
{
  for (const QMetaObject *m = &metaObject; m; m = m->superClass())
  {
    ClassMap::Iterator i = classMap.find(m->className());
    if (i == classMap.end())
      i = classMap.insert(m->className(), QMultiMap<Scheme, CreateFunc>());

    i->insert(scheme, createFunc);
  }
}

QList<QByteArray> SFactory::registredClasses(void)
{
  return classMap.keys();
}

SFactory::SchemeList SFactory::registredSchemes(const char *className)
{
  ClassMap::ConstIterator i = classMap.find(className);
  if (i != classMap.end())
    return i->keys();

  return SchemeList();
}

void SFactory::clear(void)
{
  classMap.clear();
}

QObject * SFactory::createObject(const char *className, QObject *parent, const QString &scheme, bool nonNull) const
{
  ClassMap::ConstIterator i = classMap.find(className);
  if (i != classMap.end())
  {
    for (QMultiMap<Scheme, CreateFunc>::ConstIterator j=i->begin(); j!=i->end(); j++)
    if ((j.key().name() == scheme) || (scheme.isEmpty()))
      return (*j)(j.key().name(), parent);
  }

  if (nonNull)
    qFatal("Could not find class for \"%s\" with scheme \"%s\".", className, scheme.toAscii().data());

  return NULL;
}

QList<QObject *> SFactory::createObjects(const char *className, QObject *parent) const
{
  QList<QObject *> result;

  ClassMap::ConstIterator i = classMap.find(className);
  if (i != classMap.end())
  for (QMultiMap<Scheme, CreateFunc>::ConstIterator j=i->begin(); j!=i->end(); j++)
    result << (*j)(j.key().name(), parent);

  return result;
}


SFactory::Scheme::Scheme(const char *name)
{
  d.priority = 0;
  d.name = name;
}

SFactory::Scheme::Scheme(const QByteArray &name)
{
  d.priority = 0;
  d.name = name;
}

SFactory::Scheme::Scheme(const QString &name)
{
  d.priority = 0;
  d.name = name;
}

SFactory::Scheme::Scheme(int priority)
{
  d.priority = priority;
  d.name = QString::null;
}

SFactory::Scheme::Scheme(int priority, const QString &name)
{
  d.priority = priority;
  d.name = name;
}

bool SFactory::Scheme::operator<(const Scheme &c) const
{
  if (d.priority > c.d.priority) // Highest priority first.
    return true;
  else if (d.priority == c.d.priority)
    return d.name < c.d.name;
  else
    return false;
}

bool SFactory::Scheme::operator==(const Scheme &c) const
{
  return (d.priority == c.d.priority) && (d.name == c.d.name);
}


} // End of namespace
