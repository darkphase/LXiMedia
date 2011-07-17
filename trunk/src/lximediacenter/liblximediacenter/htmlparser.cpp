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
#include "globalsettings.h"

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
  GlobalSettings settings;

  fields.clear();
  fields["_PRODUCT"]  = qApp->applicationName().toUtf8();
  fields["_HOSTNAME"] = (settings.value("DeviceName", settings.defaultDeviceName())).toString().toUtf8();

  fields["_PALETTE_WINDOW"]      = palette().window.toByteArray();
  fields["_PALETTE_WINDOWTEXT"]  = palette().windowText.toByteArray();
  fields["_PALETTE_BASE"]        = palette().base.toByteArray();
  fields["_PALETTE_ALTBASE"]     = palette().altBase.toByteArray();
  fields["_PALETTE_TEXT"]        = palette().text.toByteArray();
  fields["_PALETTE_BUTTON"]      = palette().button.toByteArray();
  fields["_PALETTE_BUTTONTEXT"]  = palette().buttonText.toByteArray();

  Palette::Rgb windowTextLight = palette().windowText;
  windowTextLight.r = (int(windowTextLight.r) + int(palette().window.r)) / 2;
  windowTextLight.g = (int(windowTextLight.g) + int(palette().window.g)) / 2;
  windowTextLight.b = (int(windowTextLight.b) + int(palette().window.b)) / 2;
  fields["_PALETTE_WINDOWTEXT_LIGHT"] = windowTextLight.toByteArray();

  Palette::Rgb textLight = palette().text;
  textLight.r = (int(textLight.r) + int(palette().base.r)) / 2;
  textLight.g = (int(textLight.g) + int(palette().base.g)) / 2;
  textLight.b = (int(textLight.b) + int(palette().base.b)) / 2;
  fields["_PALETTE_TEXT_LIGHT"] = textLight.toByteArray();

  Palette::Rgb buttonTextLight = palette().buttonText;
  buttonTextLight.r = (int(buttonTextLight.r) + int(palette().button.r)) / 2;
  buttonTextLight.g = (int(buttonTextLight.g) + int(palette().button.g)) / 2;
  buttonTextLight.b = (int(buttonTextLight.b) + int(palette().button.b)) / 2;
  fields["_PALETTE_BUTTONTEXT_LIGHT"] = buttonTextLight.toByteArray();

  Palette::Rgb buttonRed = palette().button;
  buttonRed.r = qMin(int(buttonRed.r) * 2, 255);
  fields["_PALETTE_BUTTON_RED"] = buttonRed.toByteArray();

  Palette::Rgb buttonGreen = palette().button;
  buttonGreen.g = qMin(int(buttonGreen.g) * 2, 255);
  fields["_PALETTE_BUTTON_GREEN"] = buttonGreen.toByteArray();

  Palette::Rgb buttonBlue = palette().button;
  buttonBlue.b = qMin(int(buttonBlue.b) * 2, 255);
  fields["_PALETTE_BUTTON_BLUE"] = buttonBlue.toByteArray();
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
