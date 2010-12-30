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

#include <iostream>
#include <QtCore>
#include <QtGui>

int main(int argc, char *argv[])
{
  static const int pixelSizes[] = { 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96 };

  std::cout << "/* This file has been automatically generated." << std::endl
            << std::endl
                ////////////////////////////////////////////////////////////////////////////////
            << "This file contains pre-rendered \"DejaVu Sans\" characters, see" << std::endl
            << "http://dejavu-fonts.org/ for more information on this font." << std::endl
            << std::endl
                ////////////////////////////////////////////////////////////////////////////////
            << "Copyright (c) 2003 by Bitstream, Inc. All Rights Reserved. Bitstream Vera is a" << std::endl
            << "trademark of Bitstream, Inc. Permission is hereby granted, free of charge, to" << std::endl
            << "any person obtaining a copy of the fonts accompanying this license (\"Fonts\") and" << std::endl
            << "associated documentation files (the \"Font Software\"), to reproduce and" << std::endl
            << "distribute the Font Software, including without limitation the rights to use," << std::endl
            << "copy, merge, publish, distribute, and/or sell copies of the Font Software, and" << std::endl
            << "to permit persons to whom the Font Software is furnished to do so, subject to" << std::endl
            << "the following conditions:" << std::endl
            << "The above copyright and trademark notices and this permission notice shall be" << std::endl
            << "included in all copies of one or more of the Font Software typefaces. The Font" << std::endl
            << "Software may be modified, altered, or added to, and in particular the designs of" << std::endl
            << "glyphs or characters in the Fonts may be modified and additional glyphs or" << std::endl
            << "characters may be added to the Fonts, only if the fonts are renamed to names not" << std::endl
            << "containing either the words \"Bitstream\" or the word \"Vera\"." << std::endl
            << "This License becomes null and void to the extent applicable to Fonts or Font" << std::endl
            << "Software that has been modified and is distributed under the \"Bitstream Vera\"" << std::endl
            << "names. The Font Software may be sold as part of a larger software package but no" << std::endl
            << "copy of one or more of the Font Software typefaces may be sold by itself." << std::endl
            << std::endl
                ////////////////////////////////////////////////////////////////////////////////
            << "THE FONT SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR" << std::endl
            << "IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF MERCHANTABILITY, FITNESS" << std::endl
            << "FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF COPYRIGHT, PATENT, TRADEMARK, OR" << std::endl
            << "OTHER RIGHT. IN NO EVENT SHALL BITSTREAM OR THE GNOME FOUNDATION BE LIABLE FOR" << std::endl
            << "ANY CLAIM, DAMAGES OR OTHER LIABILITY, INCLUDING ANY GENERAL, SPECIAL, INDIRECT," << std::endl
            << "INCIDENTAL, OR CONSEQUENTIAL DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR" << std::endl
            << "OTHERWISE, ARISING FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR" << std::endl
            << "FROM OTHER DEALINGS IN THE FONT SOFTWARE." << std::endl
            << std::endl
                ////////////////////////////////////////////////////////////////////////////////
            << "Except as contained in this notice, the names of Gnome, the Gnome Foundation," << std::endl
            << "and Bitstream Inc., shall not be used in advertising or otherwise to promote the" << std::endl
            << "sale, use or other dealings in this Font Software without prior written" << std::endl
            << "authorization from the Gnome Foundation or Bitstream Inc., respectively. For" << std::endl
            << "further information, contact: fonts at gnome dot org." << std::endl
            << std::endl
                ////////////////////////////////////////////////////////////////////////////////
            << "Copyright (c) 2006 by Tavmjong Bah. All Rights Reserved. Permission is hereby" << std::endl
            << "granted, free of charge, to any person obtaining a copy of the fonts" << std::endl
            << "accompanying this license (\"Fonts\") and associated documentation files (the" << std::endl
            << "\"Font Software\"), to reproduce and distribute the modifications to the" << std::endl
            << "Bitstream Vera Font Software, including without limitation the rights to use," << std::endl
            << "copy, merge, publish, distribute, and/or sell copies of the Font Software, and" << std::endl
            << "to permit persons to whom the Font Software is furnished to do so, subject to" << std::endl
            << "the following conditions:" << std::endl
            << "The above copyright and trademark notices and this permission notice shall be" << std::endl
            << "included in all copies of one or more of the Font Software typefaces. The Font" << std::endl
            << "Software may be modified, altered, or added to, and in particular the designs of" << std::endl
            << "glyphs or characters in the Fonts may be modified and additional glyphs or" << std::endl
            << "characters may be added to the Fonts, only if the fonts are renamed to names not" << std::endl
            << "containing either the words \"Tavmjong Bah\" or the word \"Arev\"." << std::endl
            << "This License becomes null and void to the extent applicable to Fonts or Font" << std::endl
            << "Software that has been modified and is distributed under the \"Tavmjong Bah Arev\"" << std::endl
            << "names. The Font Software may be sold as part of a larger software package but no" << std::endl
            << "copy of one or more of the Font Software typefaces may be sold by itself." << std::endl
            << std::endl
                ////////////////////////////////////////////////////////////////////////////////
            << "THE FONT SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR" << std::endl
            << "IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF MERCHANTABILITY, FITNESS" << std::endl
            << "FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF COPYRIGHT, PATENT, TRADEMARK, OR" << std::endl
            << "OTHER RIGHT. IN NO EVENT SHALL TAVMJONG BAH BE LIABLE FOR ANY CLAIM, DAMAGES OR" << std::endl
            << "OTHER LIABILITY, INCLUDING ANY GENERAL, SPECIAL, INDIRECT, INCIDENTAL, OR" << std::endl
            << "CONSEQUENTIAL DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE," << std::endl
            << "ARISING FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR FROM OTHER" << std::endl
            << "DEALINGS IN THE FONT SOFTWARE." << std::endl
            << std::endl
                ////////////////////////////////////////////////////////////////////////////////
            << "Except as contained in this notice, the name of Tavmjong Bah shall not be used" << std::endl
            << "in advertising or otherwise to promote the sale, use or other dealings in this" << std::endl
            << "Font Software without prior written authorization from Tavmjong Bah. For further" << std::endl
            << "information, contact: tavmjong @ free . fr." << std::endl
            << "*/" << std::endl
            << std::endl;

  QApplication app(argc, argv);
  QByteArray data;

  for (unsigned p=0; p<(sizeof(pixelSizes)/sizeof(pixelSizes[0])); p++)
  {
    const unsigned shadowSize = qMax(1, pixelSizes[p] / 16);
    QFont font("DejaVu Sans");
    font.setPixelSize(pixelSizes[p]);
    font.setStretch(75);
    font.setWeight(QFont::DemiBold);
    const QFontMetrics fm(font);
    QFont fonti(font);
    fonti.setItalic(true);
    const QFontMetrics fmi(fonti);

    const QFont * const fonts[2] = { &font, &fonti };
    const QFontMetrics * const fms[2] = { &fm, &fmi };

    for (unsigned i=0; i<256; i++)
    for (unsigned m=0; m<2; m++)
    {
      const QSize size(fms[m]->boundingRect(QChar(i)).width() + shadowSize * 2,
                       fms[m]->height() + shadowSize * 2);

      QByteArray line = QByteArray::number(i) + ";" +
                        QByteArray::number(m) + ";" +
                        QByteArray::number(fms[m]->width(QChar(i))) + ";" +
                        QByteArray::number(size.width()) + ";" +
                        QByteArray::number(size.height()) + ";";

      if ((size.width() > 0) && (size.height() > 0))
      {
        QImage c(size, QImage::Format_RGB32);
        QPainter p;
        p.begin(&c);
          p.fillRect(c.rect(), Qt::black);
          p.setFont(*(fonts[m]));
          p.setPen(Qt::white);
          p.drawText(shadowSize, fms[m]->ascent() + shadowSize + 1, QChar(i));
        p.end();

        for (int y=0; y<c.height(); y++)
        for (int x=0; x<c.width(); x++)
        {
          int v = qGreen(c.pixel(x, y)) / 28;
          if (v == 0)
          {
            v = -1;
            for (int r=1; (r<=int(shadowSize)) && (v<0); r++)
            if (((x >= r)          && (y >= r)           && (qGreen(c.pixel(x-r, y-r)) > 63)) ||
                ((x >= r)          && (y < c.height()-r) && (qGreen(c.pixel(x-r, y+r)) > 63)) ||
                ((x < c.width()-r) && (y >= r)           && (qGreen(c.pixel(x+r, y-r)) > 63)) ||
                ((x < c.width()-r) && (y < c.height()-r) && (qGreen(c.pixel(x+r, y+r)) > 63)) ||
                ((x >= r)                                && (qGreen(c.pixel(x-r, y  )) > 63)) ||
                ((x < c.width()-r)                       && (qGreen(c.pixel(x+r, y  )) > 63)) ||
                (                     (y >= r)           && (qGreen(c.pixel(x  , y-r)) > 63)) ||
                (                     (y < c.height()-r) && (qGreen(c.pixel(x  , y+r)) > 63)))
            {
              v = 0;
            }
          }

          if (v >= 0)
            line += QByteArray::number(v);
          else
            line += ' ';
        }
      }

      data += line + ":";
    }
  }

  data = qCompress(data, 9);
  for (int i=0, l=0; i<data.length(); i++)
  {
    const int d = int((unsigned char)(data[i]));
    int n;
    if (d >= 100)
      n = 4;
    else if (d >= 10)
      n = 3;
    else
      n = 2;

    if (l + n > 80)
    {
      std::cout << std::endl << d << ',';
      l = n;
    }
    else if (i < data.length() - 1)
    {
      std::cout << d << ',';
      l += n;
    }
    else
      std::cout << d;
  }

  return 0;
}
