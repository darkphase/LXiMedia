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

#include "htmlparser.h"
#include <QtNetwork>

namespace LXiMediaCenter {


HtmlParser::HtmlParser(void)
{
  clear();
}

HtmlParser::~HtmlParser()
{
}

void HtmlParser::clear(void)
{
  fields.clear();
}

void HtmlParser::setField(const QByteArray &name, const QByteArray &content)
{
  fields[name] = content;
}

void HtmlParser::setField(const QByteArray &name, const QString &content)
{
  fields[name] = parseAmp(content.toUtf8());
}

void HtmlParser::appendField(const QByteArray &name, const QByteArray &content)
{
  fields[name] += content;
}

void HtmlParser::appendField(const QByteArray &name, const QString &content)
{
  fields[name] += parseAmp(content.toUtf8());
}

void HtmlParser::copyField(const QByteArray &to, const QByteArray &from)
{
  fields[to] = fields[from];
}

QByteArray HtmlParser::field(const QByteArray &name) const
{
  return fields[name];
}

QByteArray HtmlParser::parseAmp(QByteArray data)
{
  return data.replace("&amp;", "&").replace("&", "&amp;");
}

QByteArray HtmlParser::parse(const char *txt) const
{
  QByteArray data(txt);

  return parse(data);
}

QByteArray HtmlParser::parse(QByteArray &data) const
{
  for (QMap<QByteArray, QByteArray>::const_iterator i=fields.begin(); i!=fields.end(); i++)
    data.replace("{" + i.key() + "}", *i);

  return data;
}

QByteArray HtmlParser::parseFile(const QString &fileName) const
{
  QFile file(fileName);

  if (file.exists())
  if (file.open(QIODevice::ReadOnly))
  {
    QByteArray data = file.readAll();
    parse(data);
    return data;
  }

  return QByteArray();
}

void HtmlParser::setPalette(const Palette &p)
{
  palette() = p;
}

HtmlParser::Palette & HtmlParser::palette(void)
{
  static Palette p;

  return p;
}

QByteArray HtmlParser::Palette::Rgb::toByteArray(void)
{
  return ("#" + ("0" + QByteArray::number(r, 16)).right(2) +
                ("0" + QByteArray::number(g, 16)).right(2) +
                ("0" + QByteArray::number(b, 16)).right(2)).toUpper();
}


} // End of namespace
