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
  static const int pixelSizes[] = { 16, 24, 32, 40, 48, 64, 80, 96 };

  QApplication app(argc, argv);

  QFile file("subtitlefont.bin");
  if (file.open(QFile::WriteOnly))
  {
    QByteArray data;
    data.reserve(67108864);
    {
      QBuffer buffer(&data);
      buffer.open(QBuffer::WriteOnly);

      for (unsigned p=0; p<(sizeof(pixelSizes)/sizeof(pixelSizes[0])); p++)
      {
        std::cout << "Generating font: " << pixelSizes[p] << " pt" << std::endl;

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

        for (qint32 i=0; i<256; i++)
        for (qint32 m=0; m<2; m++)
        {
          const QSize size(fms[m]->boundingRect(QChar(i)).width() + shadowSize * 8,
                           fms[m]->height() + shadowSize * 2);

          {
            const quint32 advance = fms[m]->width(QChar(i));
            const quint32 width = size.width();
            const quint32 height = size.height();

            buffer.write(reinterpret_cast<const char *>(&i), sizeof(i));
            buffer.write(reinterpret_cast<const char *>(&m), sizeof(m));
            buffer.write(reinterpret_cast<const char *>(&advance), sizeof(advance));
            buffer.write(reinterpret_cast<const char *>(&width), sizeof(width));
            buffer.write(reinterpret_cast<const char *>(&height), sizeof(height));
          }

          if ((size.width() > 0) && (size.height() > 0))
          {
            QImage c(size, QImage::Format_RGB32);
            QPainter p;
            p.begin(&c);
              p.fillRect(c.rect(), Qt::black);
              p.setFont(*(fonts[m]));
              p.setPen(Qt::white);
              p.drawText(shadowSize * 4, fms[m]->ascent() + shadowSize + 1, QChar(i));
            p.end();

            for (int y=0; y<c.height(); y++)
            for (int x=0; x<c.width(); x++)
            {
              quint8 b;
              int v = qGreen(c.pixel(x, y));
              if (v == 0)
              {
                for (int r=1; r<=int(shadowSize); r++)
                {
                  v += (x >= r)          && (y >= r)           ? qGreen(c.pixel(x-r, y-r)) : 0;
                  v += (x >= r)          && (y < c.height()-r) ? qGreen(c.pixel(x-r, y+r)) : 0;
                  v += (x < c.width()-r) && (y >= r)           ? qGreen(c.pixel(x+r, y-r)) : 0;
                  v += (x < c.width()-r) && (y < c.height()-r) ? qGreen(c.pixel(x+r, y+r)) : 0;
                  v += (x >= r)                                ? qGreen(c.pixel(x-r, y  )) : 0;
                  v += (x < c.width()-r)                       ? qGreen(c.pixel(x+r, y  )) : 0;
                  v +=                      (y >= r)           ? qGreen(c.pixel(x  , y-r)) : 0;
                  v +=                      (y < c.height()-r) ? qGreen(c.pixel(x  , y+r)) : 0;
                }

                b = quint8(qBound(0, int(v / (shadowSize * 4)), 127));
              }
              else
                b = quint8(qBound(128, 128 + int(v / 2), 255));

              buffer.write(reinterpret_cast<const char *>(&b), sizeof(b));
            }
          }
        }
      }
    }

    std::cout << "Compressing font data from: " << data.size() << std::endl;
    const QByteArray compr = qCompress(data, 9);
    std::cout << "Compressed file size: " << compr.size() << std::endl;
    file.write(compr);
  }

  return 0;
}
