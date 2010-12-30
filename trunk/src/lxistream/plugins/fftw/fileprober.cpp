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

#include "fileprober.h"

#include <fftw3.h>
#include <liblxistreamgui/simagebuffer.h>
#include "module.h"

namespace LXiStream {
namespace FFTWBackend {


FileProber::FileProber(QObject *parent)
           :SProber(parent)
{
}

FileProber::~FileProber()
{
}

void FileProber::fingerprint(FingerPrint &fp, const QString &path) const
{
  if (path.startsWith("file:"))
  {
    const QFileInfo info(path.mid(5));
    const QString suffix = info.suffix().toLower();

    if (SMediaFile::imageSuffixes().contains(suffix))
      fromImageFile(fp, path.mid(5));
    else if (SMediaFile::audioSuffixes().contains(suffix))
      fromAudioFile(fp, path.mid(5));
  }
}

void FileProber::fromImageFile(FingerPrint &fp, const QString &imageFile)
{
  const LXiStreamGui::SImageBuffer buffer = SMediaFile(imageFile).image(true);
  if (!buffer.isNull())
  {
    const QImage image = buffer.toImage();
    if (!image.isNull())
    {
      static const unsigned numPoints = fp.numBins * 2;

      const QImage im = image.convertToFormat(QImage::Format_RGB32).scaled(numPoints, numPoints, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

      Module::fftwMutex().lock();
      fftwf_complex * const tBuf = reinterpret_cast<fftwf_complex *>(fftwf_malloc(numPoints * numPoints * sizeof(fftwf_complex)));
      fftwf_complex * const fBuf = reinterpret_cast<fftwf_complex *>(fftwf_malloc(numPoints * numPoints * sizeof(fftwf_complex)));
      const fftwf_plan fftPlan = fftwf_plan_dft_2d(numPoints, numPoints, tBuf, fBuf, FFTW_FORWARD, FFTW_ESTIMATE);
      Module::fftwMutex().unlock();

      // Get center of image
      const unsigned xo = (im.width() / 2) - (numPoints / 2);
      const unsigned yo = (im.height() / 2) - (numPoints / 2);

      for (unsigned y=0; y<numPoints; y++)
      for (unsigned x=0; x<numPoints; x++)
      {
        const unsigned i = y * numPoints + x;
        tBuf[i][0] = float(qGray(im.pixel(xo + x, yo + y))) / 255.0f;
        tBuf[i][1] = 0.0f;
      }

      // Go to frequency domain
      fftwf_execute(fftPlan);

      // Gather data
      const unsigned blockSize = unsigned(sqrtf(float((numPoints * (numPoints / 2)) / fp.numBins)));

      unsigned bin = 0;
      float bins[fp.numBins];
      for (unsigned i=0; i<fp.numBins; i++)
        bins[i] = 0.0f;

      for (unsigned y=0; y<numPoints/2; y+=blockSize)
      for (unsigned x=0; x<numPoints; x+=blockSize)
      {
        float sum = 0.0f;
        for (unsigned by=0; by<blockSize; by++)
        for (unsigned bx=0; bx<blockSize; bx++)
          sum += tBuf[(y+by)*numPoints+(x+by)][0];

        bins[bin++] = sum;
      }

      // Normalize (average at 0.2)
      float avg = 0.0f;
      for (unsigned i=0; i<fp.numBins; i++)
        avg += bins[i];

      const float max = (avg / float(fp.numBins)) * 5.0f;
      if (max > 0.0f)
      for (unsigned i=0; i<fp.numBins; i++)
        fp.bins[i] = qMin(1.0f, float(bins[i] / max));

      Module::fftwMutex().lock();
      fftwf_free(tBuf);
      fftwf_free(fBuf);
      fftwf_destroy_plan(fftPlan);
      Module::fftwMutex().unlock();
    }
  }
}

void FileProber::fromAudioFile(FingerPrint &fp, const QString &audioFile)
{
  static const unsigned numPoints = 512;
  static const unsigned shiftPoints = 32;
  static const unsigned sampleRate = 8192;
  static const unsigned numBands = 8;
  static const unsigned introLength = 64; // seconds
  static const unsigned sampleLength = 64; // seconds
  static const unsigned numSamplesPerBand = (sampleRate / shiftPoints) * sampleLength;
  static const unsigned numBinsPerBand = fp.numBins / numBands;

  STerminals::File * const file = SSystem::createTerminal<STerminals::File>(NULL, audioFile, false);
  if (file)
  {
    SNode * const fileSource = file->openStream(file->inputStream(0));
    SNodes::Audio::Decoder * const audioDecoder = SSystem::createNode<SNodes::Audio::Decoder>(file, false);
    SNodes::Audio::Resampler * const audioResampler = SSystem::createNode<SNodes::Audio::Resampler>(file, false);
    if (fileSource && audioDecoder && audioResampler)
    {
      audioResampler->setSampleRate(sampleRate);
      audioResampler->setChannels(SAudioCodec::Channel_Mono);

      if (fileSource->prepare() &&
          audioDecoder->prepare() &&
          audioResampler->prepare())
      {
        unsigned numBandSamples = 0;
        float bands[numBands][numSamplesPerBand];
        for (unsigned i=0; i<numBands; i++)
        for (unsigned j=0; j<numSamplesPerBand; j++)
          bands[i][j] = 0.0f;

        // Fill the bands
        {
          Module::fftwMutex().lock();
          fftwf_complex * const tBuf = reinterpret_cast<fftwf_complex *>(fftwf_malloc(numPoints * sizeof(fftwf_complex)));
          fftwf_complex * const fBuf = reinterpret_cast<fftwf_complex *>(fftwf_malloc(numPoints * sizeof(fftwf_complex)));
          const fftwf_plan fftPlan = fftwf_plan_dft_1d(numPoints, tBuf, fBuf, FFTW_FORWARD, FFTW_ESTIMATE);
          Module::fftwMutex().unlock();

          qint16 buffer[131072];
          unsigned bufferSize = 0;
          bool skipSilence = true;
          int skipIntro = introLength * sampleRate;

          while (numBandSamples < numSamplesPerBand)
          {
            SBufferList inputBuffers;
            if (fileSource->processBuffer(SBuffer(), inputBuffers) != SNode::Result_Active)
              break; // End of file

            SBufferList audioBuffers;
            foreach (const SBuffer &b1, inputBuffers)
            if (b1.typeId() == SAudioBuffer::baseTypeId)
            {
              SBufferList decoded;
              if (audioDecoder->processBuffer(b1, decoded) == SNode::Result_Active)
              foreach (const SBuffer &b2, decoded)
                audioResampler->processBuffer(b2, audioBuffers);
            }

            const SAudioBuffer sourceBuffer = audioBuffers; // Concatenate all samples.
            if (sourceBuffer.isNull())
              continue; // Read more data

            const unsigned numSamples = sourceBuffer.numSamples();
            const qint16 * const samples = reinterpret_cast<const qint16 *>(sourceBuffer.bits());
            if (numSamples == 0)
              break;

            if (skipSilence)
            {
              float avgVol = 0;
              for (unsigned i=0; i<numSamples; i++)
                avgVol += float(qAbs(samples[i]));

              if ((avgVol / float(numSamples * 32768)) < 0.05f)
                continue;
              else
                skipSilence = false;
            }

            if (skipIntro > 0)
            {
              skipIntro -= numSamples;
              continue;
            }

            memcpy(buffer + bufferSize, samples, numSamples * sizeof(qint16));
            bufferSize += numSamples;

            while ((bufferSize >= numPoints) && (numBandSamples < numSamplesPerBand))
            {
              for (unsigned i=0; i<numPoints; i++)
              {
                tBuf[i][0] = float(buffer[i]);
                tBuf[i][1] = 0.0f;
              }

              fftwf_execute(fftPlan);

              static const unsigned ratio = (numPoints / 4) / numBands;
              for (unsigned i=0; i<numPoints/4; i++)
                bands[i/ratio][numBandSamples] += qAbs(fBuf[i+(numPoints/4)][0]);

              for (unsigned i=0; i<numPoints/4; i++)
                bands[i/ratio][numBandSamples] += qAbs(fBuf[numPoints-1-(i+(numPoints/4))][0]);

              // We shift by shiftPoints to make the samples overlap
              bufferSize -= shiftPoints;
              memmove(buffer, buffer + shiftPoints, bufferSize * sizeof(qint16));
              numBandSamples++;
            }
          }

          Module::fftwMutex().lock();
          fftwf_free(tBuf);
          fftwf_free(fBuf);
          fftwf_destroy_plan(fftPlan);
          Module::fftwMutex().unlock();
        }

        // Only if all samples could have been extracted
        if (numBandSamples == numSamplesPerBand)
        {
          // Determine the rythm in these bands
          Module::fftwMutex().lock();
          fftwf_complex * const tBuf = reinterpret_cast<fftwf_complex *>(fftwf_malloc(numSamplesPerBand * sizeof(fftwf_complex)));
          fftwf_complex * const fBuf = reinterpret_cast<fftwf_complex *>(fftwf_malloc(numSamplesPerBand * sizeof(fftwf_complex)));
          const fftwf_plan fftPlan = fftwf_plan_dft_1d(numSamplesPerBand, tBuf, fBuf, FFTW_FORWARD, FFTW_ESTIMATE);
          Module::fftwMutex().unlock();

          float bins[fp.numBins];
          for (unsigned i=0; i<fp.numBins; i++)
            bins[i] = 0.0f;

          for (unsigned i=0; i<numBands; i++)
          {
            for (unsigned j=0; j<numSamplesPerBand; j++)
            {
              tBuf[j][0] = qAbs(bands[i][j]);
              tBuf[j][1] = 0.0f;
            }

            fftwf_execute(fftPlan);

            for (unsigned j=0; j<numBinsPerBand; j++)
              bins[(i*(numBinsPerBand))+j] += qAbs(fBuf[j+1][0]); // +1 to skip DC
          }

          Module::fftwMutex().lock();
          fftwf_free(tBuf);
          fftwf_free(fBuf);
          fftwf_destroy_plan(fftPlan);
          Module::fftwMutex().unlock();

          // Normalize (average at 0.2)
          float avg = 0.0f;
          for (unsigned i=0; i<fp.numBins; i++)
            avg += bins[i];

          const float max = (avg / float(fp.numBins)) * 5.0f;
          if (max > 0.0)
          for (unsigned i=0; i<fp.numBins; i++)
            fp.bins[i] = qMin(1.0f, float(bins[i] / max));
        }
      }
    }

    delete file;
  }
}


} } // End of namespaces
