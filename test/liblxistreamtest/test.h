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

#define __TEST_H

#include <QtCore>
#include <QtTest>
#include <LXiStream>
#include <LXiStreamGl>
#include <LXiStreamGui>

class LXiStreamTest : public QObject
{
Q_OBJECT
private slots:
  void                          initTestCase(void);
  void                          cleanupTestCase(void);

private slots: // SDebug
  void                          SDebug_Log(void);
  void                          SDebug_MutexLocker(void);

private slots: // SStringParser
  void                          StringParser_CleanName(void);
  void                          StringParser_RawName(void);
  void                          StringParser_FindMatch(void);
  void                          StringParser_ComputeMatch(void);

private slots: // Engine
  void                          Engine_BufferData(void);
  void                          Engine_BufferClone(void);
  void                          Engine_BufferEnlarge(void);
  void                          Engine_BufferExternal(void);
  void                          Engine_BufferExternalConst(void);
  void                          Engine_BufferExternalRelease(void);
  void                          Engine_BufferExternalReleaseConst(void);
  void                          Engine_AudioCodec(void);
  void                          Engine_VideoCodec(void);
  void                          Engine_CodecUnion(void);
  void                          Engine_Time(void);

private:
  static void                   Engine_BufferExternalRelease_Callback(void *, int);
  static int                    Engine_BufferExternalRelease_ReleaseID;
  static void                   Engine_fillBuffer(uchar *, size_t);

//private slots: // CommonBackend
  void                          Common_MediaFileInfoImageName(void);
  void                          Common_MediaFileInfoAudioName(void);

  void                          Common_AudioResamplerStereoMono(void);
  void                          Common_AudioResamplerMonoStereo(void);
  void                          Common_AudioResamplerHalfRate(void);
  void                          Common_AudioResamplerDoubleRate(void);

  void                          Common_ChannelMatrixInterface(void);

  void                          Common_HttpLoopback(void);
  void                          Common_PsFileLoopback(void);
  void                          Common_TsUdpLoopback(void);

//private slots: // FFTWBackend
  void                          FFTWBackend_Load(void);
  void                          FFTWBackend_FingerPrint_fromImage(void);

private:
  void                          Common_FingerPrint(const char *, const char *);

#ifdef Q_OS_UNIX
private slots: // OpenGlBackend
  void                          OpenGlBackend_Load(void);
  void                          OpenGlBackend_TextureLoopback(void);
#endif

private:
  void                          Common_Loopback(LXiStream::STerminal *);

#ifdef ENABLE_FFMPEG
private slots: // FFMpegBackend
  void                          FFMpegBackend_Load(void);
  void                          FFMpegBackend_MediaFileInfoImageDeep(void);
  void                          FFMpegBackend_MediaFileInfoAudioDeep(void);
  void                          FFMpegBackend_StreamMuxAudioFile(void);
  void                          FFMpegBackend_AudioEncodeDecode(void);
  void                          FFMpegBackend_VideoEncodeDecode(void);
  void                          FFMpegBackend_DTSFraming(void);

  // Depends on FFMPEG
  void                          FFTWBackend_FingerPrint_fromAudio(void);

private:
  void                          FFMpegBackend_VideoEncodeDecode(const char *);
#endif

#ifdef ENABLE_ALSA
//private:// slots: // AlsaBackend
  void                          AlsaBackend_Load(void);
  void                          AlsaBackend_AlsaInputOutput(void);
#endif

#ifdef ENABLE_V4L
//private:// slots: // V4lBackend
  void                          V4lBackend_Load(void);
  void                          V4lBackend_V4l1Input(void);
  void                          V4lBackend_V4l2Input(void);

private:
  void                          V4lBackend_input(LXiStream::STerminal *);
#endif

#ifdef ENABLE_LINUXDVB
//private:// slots: // LinuxDvbBackend
  void                          LinuxDvbBackend_Load(void);
#endif

#ifdef ENABLE_FFMPEG
//private slots: // Graph
  void                          Graph_ManualGraph(void);
  void                          Graph_ManagedGraph(void);
  void                          Graph_ManagedGraphSingleShot(void);

private:
  unsigned                      Graph_NumBuffers;
#endif

//private slots: // Sandbox
  void                          Sandbox_Initialize(void);
  void                          Sandbox_Probe(void);
  void                          Sandbox_ProbeAsync(void);
  void                          Sandbox_Crash(void);
  void                          Sandbox_Buffer(void);
  void                          Sandbox_Shutdown(void);

//private slots: // Performance
  void                          Performance_AudioResamplerResample(void);
  void                          Performance_AudioMixerMix(void);
  void                          Performance_DeinterlaceMix(void);
#ifdef ENABLE_FFMPEG
  void                          Performance_VideoEncoderConvert(void);
#endif
  void                          Performance_BGRtoRGB(void);
  void                          Performance_YUVtoRGB(void);

private:
  void                          Performance_AudioResamplerResample(unsigned, unsigned, const char *);

private:
  inline bool                   runSilent(void)                                 { return QCoreApplication::arguments().contains("-silent"); }
};


class TestNode : public SNode
{
public:
  inline explicit               TestNode(QObject *parent) : SNode(Behavior_None, 0, SCodecList(), parent), count(0) { }

  inline unsigned               buffersProcessed(void)                          { return count; }

public: // From SNode
  inline virtual bool           prepare(const SCodecList &)                     { return true; }
  inline virtual bool           unprepare(void)                                 { return true; }

  inline virtual Result         processBuffer(const SBuffer &buffer, SBufferList &output)
  {
    if (!buffer.isNull())
    {
      count++;
      output << buffer;
    }

    return Result_Active;
  }

private:
  unsigned                      count;
};
