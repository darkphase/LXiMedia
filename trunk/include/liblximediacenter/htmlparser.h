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

#ifndef LXMEDIACENTER_HTMLPARSER_H
#define LXMEDIACENTER_HTMLPARSER_H

#include <QtCore>

namespace LXiMediaCenter {


class HtmlParser
{
public:
  struct Palette
  {
    struct Rgb
    {
      inline                    Rgb(void) : r(0), g(0), b(0), a(0) { }
      inline                    Rgb(quint8 r, quint8 g, quint8 b, quint8 a = 0) : r(r), g(g), b(b), a(a) { }
      QByteArray                toByteArray(void);

      quint8                    r, g, b, a;
    };

    Rgb                         window, windowText;
    Rgb                         base, altBase, text;
    Rgb                         button, buttonText;
  };

public:
                                HtmlParser(void);
                                ~HtmlParser();

  void                          clear(void);

  void                          setField(const QByteArray &, const QByteArray &);
  void                          setField(const QByteArray &, const QString &);
  void                          appendField(const QByteArray &, const QByteArray &);
  void                          appendField(const QByteArray &, const QString &);
  void                          copyField(const QByteArray &, const QByteArray &);
  QByteArray                    field(const QByteArray &) const;

  QByteArray                    parse(const char *) const;
  QByteArray                    parse(QByteArray &) const;
  QByteArray                    parseFile(const QString &) const;
  
  static void                   setPalette(const Palette &);

protected:
  static QByteArray             parseAmp(QByteArray);

private:
  static Palette              & palette(void) __attribute__((pure));
  
private:
  QMap<QByteArray, QByteArray>  fields;
};


} // End of namespace

#endif
