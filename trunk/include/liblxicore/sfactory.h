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

#ifndef LXICORE_SFACTORY_H
#define LXICORE_SFACTORY_H

#include <QtCore>

namespace LXiCore {

class SFactory
{
public:
  class Scheme
  {
  public:
                                Scheme(const char *name);
                                Scheme(const QByteArray &name);
                                Scheme(const QString &name);
                                Scheme(int priority);
                                Scheme(int priority, const QString &name);

    inline                      operator const QString &() const                { return d.name; }
    bool                        operator<(const Scheme &) const;
    bool                        operator==(const Scheme &) const;

    inline int                  priority(void) const                            { return d.priority; }
    inline const QString      & name(void) const                                { return d.name; }

  private:
    struct
    {
      int                       priority;
      QString                   name;
    }                           d;
  };

  typedef QList<Scheme>         SchemeList;

private:
  typedef QObject             * (* CreateFunc)(const QString &, QObject *);
  typedef QMap< QByteArray, QMultiMap<Scheme, CreateFunc> > ClassMap;

public:
                                SFactory(void);
                                ~SFactory();

  template <class _class>
  inline void                   registerClass(const Scheme &);
  void                          registerClass(const QMetaObject &, CreateFunc, const Scheme &);

  QList<QByteArray>             registredClasses(void);

  template <class _class>
  inline SchemeList             registredSchemes(void);
  SchemeList                    registredSchemes(const char *);

  void                          clear(void);

  template <class _interface>
  inline _interface           * createObject(QObject *, const QString &, bool nonNull = true) const;
  QObject                     * createObject(const char *, QObject *, const QString &, bool nonNull = true) const;

  template <class _interface>
  inline QList<_interface *>    createObjects(QObject *) const;
  QList<QObject *>              createObjects(const char *, QObject *) const;

private:
  template <class _class>
  static QObject              * createFunc(const QString &, QObject *);

private:
  ClassMap                      classMap;
};


template <class _interface>
class SFactorizable
{
public:
  template <class _instance>
  inline static void            registerClass(const SFactory::Scheme &);

public: // Implemented in sfactory.hpp; instantiate only when needed.
  static _interface           * create(QObject *, const QString &, bool nonNull = true);
  static QStringList            available(void);

protected: // Implemented in sfactory.hpp; instantiate only when needed.
  static SFactory             & factory(void);
};


template <class _class>
void SFactory::registerClass(const Scheme &scheme)
{
  registerClass(_class::staticMetaObject, &SFactory::createFunc<_class>, scheme);
}

template <class _class>
SFactory::SchemeList SFactory::registredSchemes(void)
{
  return registredSchemes(_class::staticMetaObject.className());
}

template <class _interface>
_interface * SFactory::createObject(QObject *parent, const QString &scheme, bool nonNull) const
{
  return static_cast<_interface *>(createObject(_interface::staticMetaObject.className(), parent, scheme, nonNull));
}

template <class _interface>
QList<_interface *> SFactory::createObjects(QObject *parent) const
{
  QList<_interface *> result;
  foreach (QObject *object, createObjects(_interface::staticMetaObject.className(), parent))
    result << static_cast<_interface *>(object);

  return result;
}

template <class _class>
QObject * SFactory::createFunc(const QString &scheme, QObject *parent)
{
  return new _class(scheme, parent);
}


template <class _interface>
template <class _instance>
void SFactorizable<_interface>::registerClass(const SFactory::Scheme &scheme)
{
  factory().registerClass<_instance>(scheme);
}

} // End of namespace

#endif
