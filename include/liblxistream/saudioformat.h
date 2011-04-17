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

#ifndef LXSTREAM_SAUDIOFORMAT_H
#define LXSTREAM_SAUDIOFORMAT_H

#include <QtCore>
#include <LXiCore>
#include "export.h"

namespace LXiStream {

class LXISTREAM_PUBLIC SAudioFormat
{
public:
  /*! Specifies the format in which data is stored.
   */
  enum Format
  {
    Format_Invalid = 0,

    Format_PCM_S16LE = 0x1000, Format_PCM_S16BE, Format_PCM_U16LE,
    Format_PCM_U16BE, Format_PCM_S8, Format_PCM_U8, Format_PCM_MULAW,
    Format_PCM_ALAW, Format_PCM_S32LE, Format_PCM_S32BE, Format_PCM_U32LE,
    Format_PCM_U32BE, Format_PCM_S24LE, Format_PCM_S24BE, Format_PCM_U24LE,
    Format_PCM_U24BE, Format_PCM_S24DAUD, Format_PCM_ZORK,
    Format_PCM_S16LE_PLANAR, Format_PCM_DVD, Format_PCM_F32BE, Format_PCM_F32LE,
    Format_PCM_F64BE, Format_PCM_F64LE,

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    Format_PCM_S16 = Format_PCM_S16LE, Format_PCM_U16 = Format_PCM_U16LE,
    Format_PCM_S32 = Format_PCM_S32LE, Format_PCM_U32 = Format_PCM_U32LE,
    Format_PCM_F32 = Format_PCM_F32LE, Format_PCM_F64 = Format_PCM_F64LE
#else
    Format_PCM_S16 = Format_PCM_S16BE, Format_PCM_U16 = Format_PCM_U16BE,
    Format_PCM_S32 = Format_PCM_S32BE, Format_PCM_U32 = Format_PCM_U32BE,
    Format_PCM_F32 = Format_PCM_F32BE, Format_PCM_F64 = Format_PCM_F64BE
#endif
  };

  /*! Specifies surround sound channels.

      \note These values are guaranteed to fit in an unsigned 32-bit integer.
   */
  enum Channel
  {
    Channel_None                      = 0x00000000,

    // Front speakers
    Channel_LeftFront                 = 0x00000001,
    Channel_CenterLeft                = 0x00000002,
    Channel_Center                    = 0x00000004,
    Channel_CenterRight               = 0x00000008,
    Channel_RightFront                = 0x00000010,

    Channel_TopLeftFront              = 0x00000020,
    Channel_TopCenter                 = 0x00000040,
    Channel_TopRightFront             = 0x00000080,

    Channel_BottomLeftFront           = 0x00000100,
    Channel_BottomCenter              = 0x00000200,
    Channel_BottomRightFront          = 0x00000400,

    // Side speakers
    Channel_LeftSide                  = 0x00001000,
    Channel_RightSide                 = 0x00002000,

    Channel_TopLeftSide               = 0x00004000,
    Channel_TopRightSide              = 0x00008000,

    Channel_BottomLeftSide            = 0x00010000,
    Channel_BottomRightSide           = 0x00020000,

    // Back speakers
    Channel_LeftBack                  = 0x00100000,
    Channel_Back                      = 0x00200000,
    Channel_RightBack                 = 0x00400000,

    Channel_TopLeftBack               = 0x00800000,
    Channel_TopRightBack              = 0x01000000,

    Channel_BottomLeftBack            = 0x02000000,
    Channel_BottomRightBack           = 0x04000000,

    // Misc speakers
    Channel_LowFrequencyEffectsLeft   = 0x10000000,
    Channel_LowFrequencyEffects       = 0x20000000,
    Channel_LowFrequencyEffectsRight  = 0x40000000,

    // Channel configurations
    Channel_Mono                      = Channel_Center,
    Channel_Stereo                    = Channel_LeftFront | Channel_RightFront,
    Channel_Quadraphonic              = Channel_LeftFront | Channel_RightFront |
                                        Channel_LeftBack | Channel_RightBack,
    Channel_Surround_3_0              = Channel_LeftFront | Channel_RightFront |
                                        Channel_Back,
    Channel_Surround_4_0              = Channel_LeftFront | Channel_Center | Channel_RightFront |
                                        Channel_Back,
    Channel_Surround_5_0              = Channel_LeftFront | Channel_Center | Channel_RightFront |
                                        Channel_LeftBack | Channel_RightBack,
    Channel_Surround_5_1              = Channel_Surround_5_0 |
                                        Channel_LowFrequencyEffects,
    Channel_Surround_6_0              = Channel_LeftFront | Channel_Center | Channel_RightFront |
                                        Channel_LeftBack | Channel_RightBack |
                                        Channel_Back,
    Channel_Surround_6_1              = Channel_Surround_6_0 |
                                        Channel_LowFrequencyEffects,
    Channel_Surround_7_1              = Channel_LeftFront | Channel_Center | Channel_RightFront |
                                        Channel_LeftSide | Channel_RightSide |
                                        Channel_LeftBack | Channel_RightBack |
                                        Channel_LowFrequencyEffects,
    Channel_Surround_7_1_Wide         = Channel_LeftFront | Channel_CenterLeft | Channel_Center | Channel_CenterRight | Channel_RightFront |
                                        Channel_LeftSide | Channel_RightSide |
                                        Channel_LowFrequencyEffects,
    Channel_Surround_10_2             = Channel_LeftFront | Channel_CenterLeft | Channel_Center | Channel_CenterRight | Channel_RightFront |
                                        Channel_TopLeftBack | Channel_LeftSide | Channel_RightSide | Channel_TopRightBack |
                                        Channel_Back |
                                        Channel_LowFrequencyEffectsLeft | Channel_LowFrequencyEffectsRight |
                                        Channel_TopLeftFront | Channel_TopRightFront,
    Channel_Surround_22_2             = Channel_LeftFront | Channel_CenterLeft | Channel_Center | Channel_CenterRight | Channel_RightFront |
                                        Channel_TopLeftFront | Channel_TopCenter | Channel_TopRightFront |
                                        Channel_BottomLeftFront | Channel_BottomCenter | Channel_BottomRightFront |
                                        Channel_LeftSide | Channel_RightSide |
                                        Channel_TopLeftSide | Channel_TopRightSide |
                                        Channel_LeftBack | Channel_RightBack |
                                        Channel_TopLeftBack | Channel_TopRightBack |
                                        Channel_LowFrequencyEffectsLeft | Channel_LowFrequencyEffectsRight
  };
  Q_DECLARE_FLAGS(Channels, Channel)

public:
                                SAudioFormat(void);
                                SAudioFormat(Format format, Channels = Channel_None, unsigned sampleRate = 0);

  inline                        operator Format() const                         { return format(); }
  inline                        operator const char *() const                   { return formatName(); }

  bool                          operator==(const SAudioFormat &other) const;
  inline bool                   operator!=(const SAudioFormat &other) const     { return !operator==(other); }
  bool                          operator==(Format other) const                  { return d.format == other; }
  inline bool                   operator!=(Format other) const                  { return !operator==(other); }

  inline bool                   isNull(void) const                              { return d.format == Format_Invalid; }
  inline Format                 format(void) const                              { return d.format; }
  void                          setFormat(Format format, Channels = Channel_None, unsigned sampleRate = 0);

  inline Channels               channelSetup(void) const                        { return d.channels; }
  inline void                   setChannelSetup(Channels c)                     { d.channels = c; }
  inline int                    sampleRate(void) const                          { return d.sampleRate; }
  inline void                   setSampleRate(int s)                            { d.sampleRate = s; }

  inline const char           * formatName(void) const                          { return formatName(format()); }
  inline int                    numChannels(void) const                         { return numChannels(channelSetup()); }
  inline int                    sampleSize(void) const                          { return sampleSize(format()); }

  _lxi_pure static int          numChannels(Channels c);
  _lxi_pure static Channels     guessChannels(unsigned numChannels);
  _lxi_pure static int          sampleSize(Format);
  _lxi_pure static const char * formatName(Format);
  _lxi_pure static const char * channelName(Channel);
  _lxi_pure static QString      channelNames(Channels);
  _lxi_pure static const char * channelSetupName(Channels);

private:
  struct
  {
    Format                      format;
    Channels                    channels;
    int                         sampleRate;
  }                             d;
};

} // End of namespace

Q_DECLARE_OPERATORS_FOR_FLAGS(LXiStream::SAudioFormat::Channels)

#endif
