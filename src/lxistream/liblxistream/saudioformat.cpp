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

#include "saudioformat.h"

namespace LXiStream {

SAudioFormat::SAudioFormat(void)
{
  d.format = Format_Invalid;
  d.channels = Channel_None;
  d.sampleRate = 0;
}

SAudioFormat::SAudioFormat(Format format, Channels channels, unsigned sampleRate)
{
  setFormat(format, channels, sampleRate);
}

bool SAudioFormat::operator==(const SAudioFormat &comp) const
{
  return (d.format == comp.d.format) &&
         (d.channels == comp.d.channels) && (d.sampleRate == comp.d.sampleRate);
}

/*! Sets this codec to the specified audio format.
 */
void SAudioFormat::setFormat(Format format, Channels channels, unsigned sampleRate)
{
  d.format = format;
  d.channels = channels;
  d.sampleRate = sampleRate;
}

/*! Returns the number of active channels for the specified configuration.
 */
unsigned SAudioFormat::numChannels(Channels c)
{
  unsigned result = 0;
  for (int i=0; i<32; i++)
  if (c & (quint32(1) << i))
    result++;

  return result;
}

/*! Guesses the most applicable channel configuration for the specified number
    of channels.
 */
SAudioFormat::Channels SAudioFormat::guessChannels(unsigned numChannels)
{
  switch (numChannels)
  {
    default:
    case 0:     return Channel_None;
    case 1:     return Channels_Mono;
    case 2:     return Channels_Stereo;
    case 3:     return Channels_Surround_3_0;
    case 4:     return Channels_Quadraphonic;
    case 5:     return Channels_Surround_5_0;
    case 6:     return Channels_Surround_5_1;
    case 7:     return Channels_Surround_6_1;
    case 8:     return Channels_Surround_7_1;
    case 12:    return Channels_Surround_10_2;
    case 24:    return Channels_Surround_22_2;
  }
}

/*! Returns the channel position in a specified channel setup. e.g. The right
    channel is at position 1 (counting from 0) in a stereo setup and at position
    2 in a 5.1 surround setup. Returns -1 if the specified channel is not
    present in the specified channel setup.
 */
int SAudioFormat::channelPos(Channels channelSetup, Channel channel)
{
  int offset = -1;
  for (int i=0, n=0; (i<32) && (offset<0); i++)
  if (channelSetup & (quint32(1) << i))
  {
    if (channel == (quint32(1) << i))
      offset = n;
    else
      n++;
  }

  return offset;
}

/*! Returns the size of one sample in bytes of the format, or 0 if not
    applicable. For example for Format_PCM_S16 this method returns 2.
 */
unsigned SAudioFormat::sampleSize(Format format)
{
  switch (Format(format))
  {
  case Format_PCM_S8:
  case Format_PCM_U8:         return 1;
  case Format_PCM_S16LE:
  case Format_PCM_S16BE:
  case Format_PCM_U16LE:
  case Format_PCM_U16BE:      return 2;
  case Format_PCM_S24LE:
  case Format_PCM_S24BE:
  case Format_PCM_U24LE:
  case Format_PCM_U24BE:      return 3;
  case Format_PCM_S32LE:
  case Format_PCM_S32BE:
  case Format_PCM_U32LE:
  case Format_PCM_U32BE:
  case Format_PCM_F32BE:
  case Format_PCM_F32LE:      return 4;
  case Format_PCM_F64BE:
  case Format_PCM_F64LE:      return 8;
  default:                    return 0;
  }
}

/*! Returns the name of the format, e.g. "PCM16le"
 */
const char * SAudioFormat::formatName(Format format)
{
  switch (format)
  {
  case Format_PCM_S16LE:              return "PCM/S16LE";
  case Format_PCM_S16BE:              return "PCM/S16BE";
  case Format_PCM_U16LE:              return "PCM/U16LE";
  case Format_PCM_U16BE:              return "PCM/U16BE";
  case Format_PCM_S8:                 return "PCM/S8";
  case Format_PCM_U8:                 return "PCM/U8";
  case Format_PCM_MULAW:              return "PCM/MULAW";
  case Format_PCM_ALAW:               return "PCM/ALAW";
  case Format_PCM_S32LE:              return "PCM/S32LE";
  case Format_PCM_S32BE:              return "PCM/S32BE";
  case Format_PCM_U32LE:              return "PCM/U32LE";
  case Format_PCM_U32BE:              return "PCM/U32BE";
  case Format_PCM_S24LE:              return "PCM/S24LE";
  case Format_PCM_S24BE:              return "PCM/S24BE";
  case Format_PCM_U24LE:              return "PCM/U24LE";
  case Format_PCM_U24BE:              return "PCM/U24BE";
  case Format_PCM_S24DAUD:            return "PCM/S24DAUD";
  case Format_PCM_ZORK:               return "PCM/ZORK";
  case Format_PCM_S16LE_PLANAR:       return "PCM/S16LEP";
  case Format_PCM_DVD:                return "PCM/DVD";
  case Format_PCM_F32BE:              return "PCM/F32BE";
  case Format_PCM_F32LE:              return "PCM/F32LE";
  case Format_PCM_F64BE:              return "PCM/F64BE";
  case Format_PCM_F64LE:              return "PCM/F64LE";
  default:                            return "";
  }
}

const char * SAudioFormat::channelName(Channel channel)
{
  switch (channel)
  {
  case Channel_LeftFront:                 return "Left (L)";
  case Channel_CenterLeft:                return "Center Left (CL)";
  case Channel_Center:                    return "Center (C)";
  case Channel_CenterRight:               return "Center Right (CL)";
  case Channel_RightFront:                return "Right (R)";
  case Channel_TopLeftFront:              return "";
  case Channel_TopCenter:                 return "";
  case Channel_TopRightFront:             return "";
  case Channel_BottomLeftFront:           return "";
  case Channel_BottomCenter:              return "";
  case Channel_BottomRightFront:          return "";
  case Channel_LeftSide:                  return "Left Surround (LS)";
  case Channel_RightSide:                 return "Right Surround (RS)";
  case Channel_TopLeftSide:               return "";
  case Channel_TopRightSide:              return "";
  case Channel_BottomLeftSide:            return "";
  case Channel_BottomRightSide:           return "";
  case Channel_LeftBack:                  return "Left Back (LB)";
  case Channel_Back:                      return "Back Surround (BS)";
  case Channel_RightBack:                 return "Right Back (RB)";
  case Channel_TopLeftBack:               return "";
  case Channel_TopRightBack:              return "";
  case Channel_BottomLeftBack:            return "";
  case Channel_BottomRightBack:           return "";
  case Channel_LowFrequencyEffectsLeft:   return "Low Frequency Effects (LFE)";
  case Channel_LowFrequencyEffects:       return "Left Low Frequency Effects (LLFE)";
  case Channel_LowFrequencyEffectsRight:  return "Right Low Frequency Effects (RLFE)";
  default:                                return "";
  };
}

QString SAudioFormat::channelNames(Channels channels)
{
  QString result = "";

  for (unsigned i=0; i<32; i++)
  if ((channels & (quint32(1) << i)) != 0)
  {
    QString name = channelName(Channel(quint32(1) << i));
    if (name.length() > 0)
      result += ", " + name;
  }

  if (result.length() > 0)
    return result.mid(2);
  else
    return "";
}

const char * SAudioFormat::channelSetupName(Channels channels)
{
  switch (channels)
  {
  case Channels_Mono:                     return "Mono";
  case Channels_Stereo:                   return "Stereo";
  case Channels_Quadraphonic:             return "Quadraphonic";
  case Channels_Surround_3_0:             return "3.0 Surround";
  case Channels_Surround_4_0:             return "4.0 Surround";
  case Channels_Surround_5_1:             return "5.1 Surround";
  case Channels_Surround_6_1:             return "6.1 Surround";
  case Channels_Surround_7_1:             return "7.1 Surround";
  case Channels_Surround_10_2:            return "10.2 Surround";
  case Channels_Surround_22_2:            return "22.2 Surround";
  default:                                return "";
  };
}

} // End of namespace
