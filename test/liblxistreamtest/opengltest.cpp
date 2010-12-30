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

#include "opengltest.h"
#include <QtGui>
#include <QtTest>
#include <LXiStream>
#include <LXiStreamGl>
#include <LXiStreamGui>
#include "lxistream/plugins/opengl/module.h"


/*! Loads the OpenGlBackend plugin.

    \note This is required before any of the other tests depending on the
          OpenGlBackend can run.
 */
void OpenGLTest::initTestCase(void)
{
  // We only want to initialize common and gui here, not probe for plugins.
  QVERIFY(SSystem::initialize(SSystem::Initialize_Devices |
                              SSystem::Initialize_LogToConsole, 0));

  SGlSystem::initialize(NULL);

  QVERIFY(SSystem::loadModule(new OpenGlBackend::Module()));
}

void OpenGLTest::cleanupTestCase(void)
{
  SGlSystem::shutdown();
  SSystem::shutdown();
}

/*! Uploads an image to the GPU and downloads it again..

    \note The test image "ImageTest.jpeg" is public domain. It is a detail of
          "Pieter Bruegel the Elder - Storm at Sea".
 */
void OpenGLTest::TextureLoopback(void)
{
  SGlSystem::activateContext();

  const QImage image(":/ImageTest.jpeg");
  const STextureBuffer textureBuffer = image;
  QCOMPARE(textureBuffer.codec().size().width(), qint16(570));
  QCOMPARE(textureBuffer.codec().size().height(), qint16(717));
  QVERIFY(qFuzzyCompare(textureBuffer.codec().size().aspectRatio(), 1.0f));
  QCOMPARE(textureBuffer.texture().width, 1024u);
  QCOMPARE(textureBuffer.texture().height, 1024u);

  const SImageBuffer outputBuffer = textureBuffer;
  QCOMPARE(outputBuffer.codec().size().width(), qint16(570));
  QCOMPARE(outputBuffer.codec().size().height(), qint16(717));
  QVERIFY(qFuzzyCompare(outputBuffer.codec().size().aspectRatio(), 1.0f));

  const QImage outputImage = outputBuffer.toImage(); // Converts BGR to RGB

  // Compare both images (allows small rounding differences)
  bool fail = false;
  QImage deltaImage(outputImage.width(), outputImage.height(), QImage::Format_RGB32);
  for (int y=0; y<outputImage.height(); y++)
  {
    const QRgb * const c1 = reinterpret_cast<const QRgb *>(image.scanLine(y));
    const QRgb * const c2 = reinterpret_cast<const QRgb *>(outputImage.scanLine(y));
    QRgb * const di = reinterpret_cast<QRgb *>(deltaImage.scanLine(y));

    for (int x=0; x<outputImage.width(); x++)
    {
      const int delta = qAbs(qRed(c1[x]) - qRed(c2[x])) +
                        qAbs(qGreen(c1[x]) - qGreen(c2[x])) +
                        qAbs(qBlue(c1[x]) - qBlue(c2[x]));

      if (delta >= 32)
        fail = true;

      di[x] = qRgb(delta, delta, delta);
    }
  }

  if (fail)
  {
    image.save(QDir::tempPath() + "/OpenGlBackend_TextureLoopback_in.png");
    outputImage.save(QDir::tempPath() + "/OpenGlBackend_TextureLoopback_out.png");
    deltaImage.save(QDir::tempPath() + "/OpenGlBackend_TextureLoopback_delta.png");
    QFAIL(("OpenGL texture transfer failed; results saved to: \"" +
           QDir::tempPath() + "/OpenGlBackend_TextureLoopback_*.png\"").toAscii());
  }
}
