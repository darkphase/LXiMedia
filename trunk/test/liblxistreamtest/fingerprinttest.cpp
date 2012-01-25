/******************************************************************************
 *   Copyright (C) 2012  A.J. Admiraal                                        *
 *   code@admiraal.dds.nl                                                     *
 *                                                                            *
 *   This program is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License version 3 as        *
 *   published by the Free Software Foundation.                               *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 ******************************************************************************/

#include "test.h"
#include "lxistream/plugins/fftw/module.h"

/*! Loads the FFTWBackend plugin.

    \note This is required before any of the other tests depending on the
          FFTWBackend can run.
 */
void LXiStreamTest::FFTWBackend_Load(void)
{
  QVERIFY(SSystem::loadModule(new FFTWBackend::Module()));
}

/*! Tests the fingerprinting mechanism on an image.

    \note The following license applies to the test image "FingerPrintTest.jpeg":
          Permission is granted to copy, distribute and/or modify this document
          under the terms of the GNU Free Documentation License, Version 1.2 or
          any later version published by the Free Software Foundation; with no
          Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts.

    \note The test image "ImageTest.jpeg" is public domain. It is a detail of
          "Pieter Bruegel the Elder - Storm at Sea".
 */
void LXiStreamTest::FFTWBackend_FingerPrint_fromImage(void)
{
  static const char * const refFp =
      "<fingerprint>"
      " LjGDMyA21DNtMSYxBi+OMBowti5NK80paC9tMUsuhCyxLKQuczDrLusuEjQc"
      " PNg8/za7MGMtGSz/K4kubzK8Lesq7SthLCArtSrlL/Y2oTgJNGwtjykzKJom"
      " KCeAKtYp8CW9JgkpACmfLHtCwFB+R2JZ8GCnTcs+bi7JJ1Im9yT8Jm0xgjah"
      " OCM7IT5PQgdYoUPuWL9IXT1oOpFAikHTJBlNo1rTPt80VzOBMrgv1TAsMOkt"
      " eywyK94s2DGvL4wz9kUtNEkxvTGUOko1xS2dLyk2MC5GLOYoaC8XK9YpRCsG"
      " Lxwt4C1wK8Ar9C7eMJUvgC5sLUwnFCbcL3cuAS28OA=="
      "</fingerprint>";

  Common_FingerPrint(":/FingerPrintTest.jpeg", refFp);

  Common_FingerPrint(":/ImageTest.jpeg", NULL);

}

/*! Tests the fingerprinting mechanism on a sound.

    \note The test sound "FingerPrintTest.ogg" is public domain. It is a clip of
          Berceuse (Op. 57), by Frédéric Chopin performed by Veronica van der
          Knaap, Public Concert, Christchurch, New Zealand, 4 May 2002.

    \note The test sound "SoundTest.ogg" is public domain. It is a the chord of
          A major played on an acoustic guitar.
 */
void LXiStreamTest::FFTWBackend_FingerPrint_fromAudio(void)
{
  static const char * const refFp =
      "<fingerprint>"
      " Vb4/vtTyIv/YnncpWDSaPtR2AyXkaPD3AZ+2HsuuC19/i85CXGwT0ns+Vgyw"
      " eisFoQT4Mf8UPU7sCZQdSDCEKIltEDBvbiSkvABmBZWM6AQvAZZeaReqN9Ef"
      " lTZ4JikAX10mKO5FUYmFCdEH3GnlDe0DkkBPC6M6+xAfFzc0FQW4QBkj8CCP"
      " X5EFtR+dRAgIhgo4HRwGRCNOHksSrAebBGgxGRfjI69aoQwjE1tV5hX/AO0w"
      " 4QzAHI4WgBPqFQEfLjSNHjcRcEiFChAaPUPUABoLthx1AakX9g5aDXYB0RZs"
      " MbUcsA0mRksLcxcyQdoAHQvNHD8EjxZHDKMNRAJMFw=="
      "</fingerprint>";

  Common_FingerPrint(":/FingerPrintTest.ogg", refFp);

  Common_FingerPrint(":/SoundTest.ogg", NULL);
}

void LXiStreamTest::Common_FingerPrint(const char *file, const char *refFp)
{
  /*const SProber::FingerPrint fp1 = SMediaFile(file).fingerprint();
  SProber::FingerPrint fp2;
  fp2.fromByteArray(fp1.toByteArray());

  QCOMPARE(fp1.toByteArray(-1), fp2.toByteArray(-1));

  if (refFp)
  {
    SProber::FingerPrint fp3;
    fp3.fromByteArray(refFp);

    QVERIFY(qFuzzyIsNull(fp2.delta(fp3)));
    for (unsigned i=0; i<fp1.numBins; i++)
      QVERIFY(qFuzzyCompare(fp2.bins[i], fp3.bins[i]));
  }*/
}
