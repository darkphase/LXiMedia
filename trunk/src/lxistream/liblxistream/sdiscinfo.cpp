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

#include "sdiscinfo.h"
#include "sinterfaces.h"
#include "sstringparser.h"

namespace LXiStream {

QDomNode SDiscInfo::toXml(QDomDocument &doc) const
{
  QDomElement discinfo = createElement(doc, "discinfo");
  if (pi.isProbed)         discinfo.setAttribute("probed", trueFalse(pi.isProbed));
  if (pi.isReadable)       discinfo.setAttribute("readable", trueFalse(pi.isReadable));

  discinfo.setAttribute("format", SStringParser::removeControl(pi.format));

  unsigned id = 0;
  foreach (const SMediaInfo &title, titles())
  {
    QDomElement elm = createElement(doc, "title");
    elm.setAttribute("id", id++);
    elm.appendChild(title.toXml(doc));
    discinfo.appendChild(elm);
  }

  return discinfo;
}

void SDiscInfo::fromXml(const QDomNode &elm)
{
  QDomElement discinfo = findElement(elm, "discinfo");

  pi.isProbed = trueFalse(discinfo.attribute("probed"));
  pi.isReadable = trueFalse(discinfo.attribute("readable"));
  pi.format = discinfo.attribute("format");

  QMultiMap<unsigned, SMediaInfo> titles;
  for (QDomElement elm = discinfo.firstChildElement("title");
       !elm.isNull();
       elm = elm.nextSiblingElement("title"))
  {
    SMediaInfo title;
    title.fromXml(elm);
    titles.insert(elm.attribute("id").toUInt(), title);
  }

  pi.titles.clear();
  foreach (const SMediaInfo &info, titles)
    pi.titles += info.pi;
}

SMediaInfoList SDiscInfo::titles(void) const
{
  QList<SMediaInfo> result;
  foreach (const SInterfaces::FileFormatProber::ProbeInfo &i, pi.titles)
    result += SMediaInfo(i);

  return result;
}

void SDiscInfo::probe(void)
{
  if (!pi.isProbed)
  {
    foreach (SInterfaces::DiscFormatProber *prober, SInterfaces::DiscFormatProber::create(NULL))
    {
      prober->probePath(pi, path);

      delete prober;
    }

    pi.isProbed = true;
  }
}

} // End of namespace
