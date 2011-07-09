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

#include "ffmpegcommon.h"
#include <cstring>

namespace LXiStream {
namespace FFMpegBackend {

int   FFMpegCommon::logLevel = AV_LOG_QUIET;
bool  FFMpegCommon::logDisabled = false;

void FFMpegCommon::init(bool verbose)
{
  static bool initialized = false;

  if (SBuffer::numPaddingBytes < FF_INPUT_BUFFER_PADDING_SIZE)
    qFatal("The number of padding bytes (%d) is not sufficient for FFMpeg (>= %d)", SBuffer::numPaddingBytes, FF_INPUT_BUFFER_PADDING_SIZE);

  if (verbose)
    logLevel = AV_LOG_VERBOSE;
  else
    logLevel = AV_LOG_QUIET;

  if (!initialized)
  {
    initialized = true;

    ::av_log_set_callback(&FFMpegCommon::log);
    ::av_lockmgr_register(&FFMpegCommon::lock);
    ::av_register_all();
  }
}

void FFMpegCommon::disableLog(bool disabled)
{
  logDisabled = disabled;
}

::CodecID FFMpegCommon::toFFMpegCodecID(const QByteArray &codec)
{
  ////////////////////////////////////////////////////////////////////////////
  // Audio codecs
  if (codec == "MP2")                   return CODEC_ID_MP2;
  if (codec == "MP3")                   return CODEC_ID_MP3;
  if (codec == "AAC")                   return CODEC_ID_AAC;
  if (codec == "AC3")                   return CODEC_ID_AC3;
  if (codec == "DTS")                   return CODEC_ID_DTS;
  if (codec == "VORBIS")                return CODEC_ID_VORBIS;
  if (codec == "DVAUDIO")               return CODEC_ID_DVAUDIO;
  if (codec == "WMAV1")                 return CODEC_ID_WMAV1;
  if (codec == "WMAV2")                 return CODEC_ID_WMAV2;
  if (codec == "MACE3")                 return CODEC_ID_MACE3;
  if (codec == "MACE6")                 return CODEC_ID_MACE6;
  if (codec == "VMDAUDIO")              return CODEC_ID_VMDAUDIO;
  if (codec == "SONIC")                 return CODEC_ID_SONIC;
  if (codec == "SONIC_LS")              return CODEC_ID_SONIC_LS;
  if (codec == "FLAC")                  return CODEC_ID_FLAC;
  if (codec == "MP3ADU")                return CODEC_ID_MP3ADU;
  if (codec == "MP3ON4")                return CODEC_ID_MP3ON4;
  if (codec == "SHORTEN")               return CODEC_ID_SHORTEN;
  if (codec == "ALAC")                  return CODEC_ID_ALAC;
  if (codec == "WESTWOOD_SND1")         return CODEC_ID_WESTWOOD_SND1;
  if (codec == "GSM")                   return CODEC_ID_GSM;
  if (codec == "QDM2")                  return CODEC_ID_QDM2;
  if (codec == "COOK")                  return CODEC_ID_COOK;
  if (codec == "TRUESPEECH")            return CODEC_ID_TRUESPEECH;
  if (codec == "TTA")                   return CODEC_ID_TTA;
  if (codec == "SMACKAUDIO")            return CODEC_ID_SMACKAUDIO;
  if (codec == "QCELP")                 return CODEC_ID_QCELP;
  if (codec == "WAVPACK")               return CODEC_ID_WAVPACK;
  if (codec == "DSICINAUDIO")           return CODEC_ID_DSICINAUDIO;
  if (codec == "IMC")                   return CODEC_ID_IMC;
  if (codec == "MUSEPACK7")             return CODEC_ID_MUSEPACK7;
  if (codec == "MLP")                   return CODEC_ID_MLP;
  if (codec == "GSM_MS")                return CODEC_ID_GSM_MS;
  if (codec == "ATRAC3")                return CODEC_ID_ATRAC3;
  if (codec == "VOXWARE")               return CODEC_ID_VOXWARE;
  if (codec == "APE")                   return CODEC_ID_APE;
  if (codec == "NELLYMOSER")            return CODEC_ID_NELLYMOSER;
  if (codec == "MUSEPACK8")             return CODEC_ID_MUSEPACK8;
  if (codec == "SPEEX")                 return CODEC_ID_SPEEX;
  if (codec == "WMAVOICE")              return CODEC_ID_WMAVOICE;
  if (codec == "WMAPRO")                return CODEC_ID_WMAPRO;
  if (codec == "WMALOSSLESS")           return CODEC_ID_WMALOSSLESS;
  if (codec == "ATRAC3P")               return CODEC_ID_ATRAC3P;
  if (codec == "EAC3")                  return CODEC_ID_EAC3;
  if (codec == "SIPR")                  return CODEC_ID_SIPR;
  if (codec == "MP1")                   return CODEC_ID_MP1;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  if (codec == "TWINVQ")                return CODEC_ID_TWINVQ;
  if (codec == "TRUEHD")                return CODEC_ID_TRUEHD;
  if (codec == "MP4ALS")                return CODEC_ID_MP4ALS;
  if (codec == "ATRAC1")                return CODEC_ID_ATRAC1;
  if (codec == "BINKAUDIO_RDFT")        return CODEC_ID_BINKAUDIO_RDFT;
  if (codec == "BINKAUDIO_DCT")         return CODEC_ID_BINKAUDIO_DCT;
#endif

  ////////////////////////////////////////////////////////////////////////////
  // Video codecs
  if (codec == "MPEG1")                 return CODEC_ID_MPEG1VIDEO;
  if (codec == "MPEG2")                 return CODEC_ID_MPEG2VIDEO;
  if (codec == "H261")                  return CODEC_ID_H261;
  if (codec == "H263")                  return CODEC_ID_H263;
  if (codec == "RV10")                  return CODEC_ID_RV10;
  if (codec == "RV20")                  return CODEC_ID_RV20;
  if (codec == "MJPEG")                 return CODEC_ID_MJPEG;
  if (codec == "MJPEGB")                return CODEC_ID_MJPEGB;
  if (codec == "LJPEG")                 return CODEC_ID_LJPEG;
  if (codec == "SP5X")                  return CODEC_ID_SP5X;
  if (codec == "JPEGLS")                return CODEC_ID_JPEGLS;
  if (codec == "MPEG4")                 return CODEC_ID_MPEG4;
  if (codec == "RAWVIDEO")              return CODEC_ID_RAWVIDEO;
  if (codec == "MSMPEG4V1")             return CODEC_ID_MSMPEG4V1;
  if (codec == "MSMPEG4V2")             return CODEC_ID_MSMPEG4V2;
  if (codec == "MSMPEG4V3")             return CODEC_ID_MSMPEG4V3;
  if (codec == "WMV1")                  return CODEC_ID_WMV1;
  if (codec == "WMV2")                  return CODEC_ID_WMV2;
  if (codec == "H263P")                 return CODEC_ID_H263P;
  if (codec == "H263I")                 return CODEC_ID_H263I;
  if (codec == "FLV1")                  return CODEC_ID_FLV1;
  if (codec == "SVQ1")                  return CODEC_ID_SVQ1;
  if (codec == "SVQ3")                  return CODEC_ID_SVQ3;
  if (codec == "DVVIDEO")               return CODEC_ID_DVVIDEO;
  if (codec == "HUFFYUV")               return CODEC_ID_HUFFYUV;
  if (codec == "CYUV")                  return CODEC_ID_CYUV;
  if (codec == "H264")                  return CODEC_ID_H264;
  if (codec == "INDEO3")                return CODEC_ID_INDEO3;
  if (codec == "VP3")                   return CODEC_ID_VP3;
  if (codec == "THEORA")                return CODEC_ID_THEORA;
  if (codec == "ASV1")                  return CODEC_ID_ASV1;
  if (codec == "ASV2")                  return CODEC_ID_ASV2;
  if (codec == "FFV1")                  return CODEC_ID_FFV1;
  if (codec == "4XM")                   return CODEC_ID_4XM;
  if (codec == "VCR1")                  return CODEC_ID_VCR1;
  if (codec == "CLJR")                  return CODEC_ID_CLJR;
  if (codec == "MDEC")                  return CODEC_ID_MDEC;
  if (codec == "ROQ")                   return CODEC_ID_ROQ;
  if (codec == "INTERPLAY_VIDEO")       return CODEC_ID_INTERPLAY_VIDEO;
  if (codec == "XAN_WC3")               return CODEC_ID_XAN_WC3;
  if (codec == "XAN_WC4")               return CODEC_ID_XAN_WC4;
  if (codec == "RPZA")                  return CODEC_ID_RPZA;
  if (codec == "CINEPAK")               return CODEC_ID_CINEPAK;
  if (codec == "WS_VQA")                return CODEC_ID_WS_VQA;
  if (codec == "MSRLE")                 return CODEC_ID_MSRLE;
  if (codec == "MSVIDEO1")              return CODEC_ID_MSVIDEO1;
  if (codec == "IDCIN")                 return CODEC_ID_IDCIN;
  if (codec == "8BPS")                  return CODEC_ID_8BPS;
  if (codec == "SMC")                   return CODEC_ID_SMC;
  if (codec == "FLIC")                  return CODEC_ID_FLIC;
  if (codec == "TRUEMOTION1")           return CODEC_ID_TRUEMOTION1;
  if (codec == "VMDVIDEO")              return CODEC_ID_VMDVIDEO;
  if (codec == "MSZH")                  return CODEC_ID_MSZH;
  if (codec == "ZLIB")                  return CODEC_ID_ZLIB;
  if (codec == "QTRLE")                 return CODEC_ID_QTRLE;
  if (codec == "SNOW")                  return CODEC_ID_SNOW;
  if (codec == "TSCC")                  return CODEC_ID_TSCC;
  if (codec == "ULTI")                  return CODEC_ID_ULTI;
  if (codec == "QDRAW")                 return CODEC_ID_QDRAW;
  if (codec == "VIXL")                  return CODEC_ID_VIXL;
  if (codec == "QPEG")                  return CODEC_ID_QPEG;
  if (codec == "XVID")                  return CODEC_ID_XVID;
  if (codec == "PNG")                   return CODEC_ID_PNG;
  if (codec == "PPM")                   return CODEC_ID_PPM;
  if (codec == "PBM")                   return CODEC_ID_PBM;
  if (codec == "PGM")                   return CODEC_ID_PGM;
  if (codec == "PGMYUV")                return CODEC_ID_PGMYUV;
  if (codec == "PAM")                   return CODEC_ID_PAM;
  if (codec == "FFVHUFF")               return CODEC_ID_FFVHUFF;
  if (codec == "RV30")                  return CODEC_ID_RV30;
  if (codec == "RV40")                  return CODEC_ID_RV40;
  if (codec == "VC1")                   return CODEC_ID_VC1;
  if (codec == "WMV3")                  return CODEC_ID_WMV3;
  if (codec == "LOCO")                  return CODEC_ID_LOCO;
  if (codec == "WNV1")                  return CODEC_ID_WNV1;
  if (codec == "AASC")                  return CODEC_ID_AASC;
  if (codec == "INDEO2")                return CODEC_ID_INDEO2;
  if (codec == "FRAPS")                 return CODEC_ID_FRAPS;
  if (codec == "TRUEMOTION2")           return CODEC_ID_TRUEMOTION2;
  if (codec == "BMP")                   return CODEC_ID_BMP;
  if (codec == "CSCD")                  return CODEC_ID_CSCD;
  if (codec == "MMVIDEO")               return CODEC_ID_MMVIDEO;
  if (codec == "ZMBV")                  return CODEC_ID_ZMBV;
  if (codec == "AVS")                   return CODEC_ID_AVS;
  if (codec == "SMACKVIDEO")            return CODEC_ID_SMACKVIDEO;
  if (codec == "NUV")                   return CODEC_ID_NUV;
  if (codec == "KMVC")                  return CODEC_ID_KMVC;
  if (codec == "FLASHSV")               return CODEC_ID_FLASHSV;
  if (codec == "CAVS")                  return CODEC_ID_CAVS;
  if (codec == "JPEG2000")              return CODEC_ID_JPEG2000;
  if (codec == "VMNC")                  return CODEC_ID_VMNC;
  if (codec == "VP5")                   return CODEC_ID_VP5;
  if (codec == "VP6")                   return CODEC_ID_VP6;
  if (codec == "VP6F")                  return CODEC_ID_VP6F;
  if (codec == "TARGA")                 return CODEC_ID_TARGA;
  if (codec == "DSICINVIDEO")           return CODEC_ID_DSICINVIDEO;
  if (codec == "TIERTEXSEQVIDEO")       return CODEC_ID_TIERTEXSEQVIDEO;
  if (codec == "TIFF")                  return CODEC_ID_TIFF;
  if (codec == "GIF")                   return CODEC_ID_GIF;
  if (codec == "FFH264")                return CODEC_ID_FFH264;
  if (codec == "DXA")                   return CODEC_ID_DXA;
  if (codec == "DNXHD")                 return CODEC_ID_DNXHD;
  if (codec == "THP")                   return CODEC_ID_THP;
  if (codec == "SGI")                   return CODEC_ID_SGI;
  if (codec == "C93")                   return CODEC_ID_C93;
  if (codec == "BETHSOFTVID")           return CODEC_ID_BETHSOFTVID;
  if (codec == "PTX")                   return CODEC_ID_PTX;
  if (codec == "TXD")                   return CODEC_ID_TXD;
  if (codec == "VP6A")                  return CODEC_ID_VP6A;
  if (codec == "AMV")                   return CODEC_ID_AMV;
  if (codec == "VB")                    return CODEC_ID_VB;
  if (codec == "PCX")                   return CODEC_ID_PCX;
  if (codec == "SUNRAST")               return CODEC_ID_SUNRAST;
  if (codec == "INDEO4")                return CODEC_ID_INDEO4;
  if (codec == "INDEO5")                return CODEC_ID_INDEO5;
  if (codec == "MIMIC")                 return CODEC_ID_MIMIC;
  if (codec == "RL2")                   return CODEC_ID_RL2;
  if (codec == "8SVX_EXP")              return CODEC_ID_8SVX_EXP;
  if (codec == "8SVX_FIB")              return CODEC_ID_8SVX_FIB;
  if (codec == "ESCAPE124")             return CODEC_ID_ESCAPE124;
  if (codec == "DIRAC")                 return CODEC_ID_DIRAC;
  if (codec == "BFI")                   return CODEC_ID_BFI;
  if (codec == "CMV")                   return CODEC_ID_CMV;
  if (codec == "MOTIONPIXELS")          return CODEC_ID_MOTIONPIXELS;
  if (codec == "TGV")                   return CODEC_ID_TGV;
  if (codec == "TGQ")                   return CODEC_ID_TGQ;
  if (codec == "TQI")                   return CODEC_ID_TQI;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  if (codec == "AURA")                  return CODEC_ID_AURA;
  if (codec == "AURA2")                 return CODEC_ID_AURA2;
  if (codec == "V210X")                 return CODEC_ID_V210X;
  if (codec == "TMV")                   return CODEC_ID_TMV;
  if (codec == "V210")                  return CODEC_ID_V210;
  if (codec == "DPX")                   return CODEC_ID_DPX;
  if (codec == "MAD")                   return CODEC_ID_MAD;
  if (codec == "FRWU")                  return CODEC_ID_FRWU;
  if (codec == "FLASHSV2")              return CODEC_ID_FLASHSV2;
  if (codec == "CDGRAPHICS")            return CODEC_ID_CDGRAPHICS;
  if (codec == "R210")                  return CODEC_ID_R210;
  if (codec == "ANM")                   return CODEC_ID_ANM;
  if (codec == "BINKVIDEO")             return CODEC_ID_BINKVIDEO;
  if (codec == "IFF_ILBM")              return CODEC_ID_IFF_ILBM;
  if (codec == "IFF_BYTERUN1")          return CODEC_ID_IFF_BYTERUN1;
  if (codec == "KGV1")                  return CODEC_ID_KGV1;
  if (codec == "YOP")                   return CODEC_ID_YOP;
  if (codec == "VP8")                   return CODEC_ID_VP8;
#endif

  ////////////////////////////////////////////////////////////////////////////
  // Audio formats
  if (codec == "PCM/S16LE")             return CODEC_ID_PCM_S16LE;
  if (codec == "PCM/S16BE")             return CODEC_ID_PCM_S16BE;
  if (codec == "PCM/U16LE")             return CODEC_ID_PCM_U16LE;
  if (codec == "PCM/U16BE")             return CODEC_ID_PCM_U16BE;
  if (codec == "PCM/S8")                return CODEC_ID_PCM_S8;             
  if (codec == "PCM/U8")                return CODEC_ID_PCM_U8;             
  if (codec == "PCM/MULAW")             return CODEC_ID_PCM_MULAW;
  if (codec == "PCM/ALAW")              return CODEC_ID_PCM_ALAW;
  if (codec == "PCM/S32LE")             return CODEC_ID_PCM_S32LE;
  if (codec == "PCM/S32BE")             return CODEC_ID_PCM_S32BE;
  if (codec == "PCM/U32LE")             return CODEC_ID_PCM_U32LE;
  if (codec == "PCM/U32BE")             return CODEC_ID_PCM_U32BE;
  if (codec == "PCM/S24LE")             return CODEC_ID_PCM_S24LE;
  if (codec == "PCM/S24BE")             return CODEC_ID_PCM_S24BE;
  if (codec == "PCM/U24LE")             return CODEC_ID_PCM_U24LE;
  if (codec == "PCM/U24BE")             return CODEC_ID_PCM_U24BE;
  if (codec == "PCM/S24DAUD")           return CODEC_ID_PCM_S24DAUD;
  if (codec == "PCM/ZORK")              return CODEC_ID_PCM_ZORK;
  if (codec == "PCM/S16LEP")            return CODEC_ID_PCM_S16LE_PLANAR;
  if (codec == "PCM/DVD")               return CODEC_ID_PCM_DVD;
  if (codec == "PCM/F32BE")             return CODEC_ID_PCM_F32BE;
  if (codec == "PCM/F32LE")             return CODEC_ID_PCM_F32LE;
  if (codec == "PCM/F64BE")             return CODEC_ID_PCM_F64BE;
  if (codec == "PCM/F64LE")             return CODEC_ID_PCM_F64LE;

  //////////////////////////////////////////////////////////////////////////////
  // Subtitle codecs
  if (codec == "SUB/DVD")               return CODEC_ID_DVD_SUBTITLE;
  if (codec == "SUB/DVB")               return CODEC_ID_DVB_SUBTITLE;
  if (codec == "SUB/RAWUTF8")           return CODEC_ID_TEXT;
  if (codec == "SUB/XSUB")              return CODEC_ID_XSUB;
  if (codec == "SUB/SSA")               return CODEC_ID_SSA;
  if (codec == "SUB/MOV_TEXT")          return CODEC_ID_MOV_TEXT;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  if (codec == "SUB/HDMV_PGS")          return CODEC_ID_HDMV_PGS_SUBTITLE;
  if (codec == "SUB/DVB_TELETEXT")      return CODEC_ID_DVB_TELETEXT;
#endif

  else                                  return CODEC_ID_NONE;
}

const char * FFMpegCommon::fromFFMpegCodecID(::CodecID id)
{
  switch (id)
  {
  default:
  case CODEC_ID_NONE:               return NULL;

  //////////////////////////////////////////////////////////////////////////////
  // Audio codecs
  case CODEC_ID_MP2:                return "MP2";
  case CODEC_ID_MP3:                return "MP3";
  case CODEC_ID_AAC:                return "AAC";
  case CODEC_ID_AC3:                return "AC3";
  case CODEC_ID_DTS:                return "DTS";
  case CODEC_ID_VORBIS:             return "VORBIS";
  case CODEC_ID_DVAUDIO:            return "DVAUDIO";
  case CODEC_ID_WMAV1:              return "WMAV1";
  case CODEC_ID_WMAV2:              return "WMAV2";
  case CODEC_ID_MACE3:              return "MACE3";
  case CODEC_ID_MACE6:              return "MACE6";
  case CODEC_ID_VMDAUDIO:           return "VMDAUDIO";
  case CODEC_ID_SONIC:              return "SONIC";
  case CODEC_ID_SONIC_LS:           return "SONIC_LS";
  case CODEC_ID_FLAC:               return "FLAC";
  case CODEC_ID_MP3ADU:             return "MP3ADU";
  case CODEC_ID_MP3ON4:             return "MP3ON4";
  case CODEC_ID_SHORTEN:            return "SHORTEN";
  case CODEC_ID_ALAC:               return "ALAC";
  case CODEC_ID_WESTWOOD_SND1:      return "WESTWOOD_SND1";
  case CODEC_ID_GSM:                return "GSM";
  case CODEC_ID_QDM2:               return "QDM2";
  case CODEC_ID_COOK:               return "COOK";
  case CODEC_ID_TRUESPEECH:         return "TRUESPEECH";
  case CODEC_ID_TTA:                return "TTA";
  case CODEC_ID_SMACKAUDIO:         return "SMACKAUDIO";
  case CODEC_ID_QCELP:              return "QCELP";
  case CODEC_ID_WAVPACK:            return "WAVPACK";
  case CODEC_ID_DSICINAUDIO:        return "DSICINAUDIO";
  case CODEC_ID_IMC:                return "IMC";
  case CODEC_ID_MUSEPACK7:          return "MUSEPACK7";
  case CODEC_ID_MLP:                return "MLP";
  case CODEC_ID_GSM_MS:             return "GSM_MS";
  case CODEC_ID_ATRAC3:             return "ATRAC3";
  case CODEC_ID_VOXWARE:            return "VOXWARE";
  case CODEC_ID_APE:                return "APE";
  case CODEC_ID_NELLYMOSER:         return "NELLYMOSER";
  case CODEC_ID_MUSEPACK8:          return "MUSEPACK8";
  case CODEC_ID_SPEEX:              return "SPEEX";
  case CODEC_ID_WMAVOICE:           return "WMAVOICE";
  case CODEC_ID_WMAPRO:             return "WMAPRO";
  case CODEC_ID_WMALOSSLESS:        return "WMALOSSLESS";
  case CODEC_ID_ATRAC3P:            return "ATRAC3P";
  case CODEC_ID_EAC3:               return "EAC3";
  case CODEC_ID_SIPR:               return "SIPR";
  case CODEC_ID_MP1:                return "MP1";
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  case CODEC_ID_TWINVQ:             return "TWINVQ";
  case CODEC_ID_TRUEHD:             return "TRUEHD";
  case CODEC_ID_MP4ALS:             return "MP4ALS";
  case CODEC_ID_ATRAC1:             return "ATRAC1";
  case CODEC_ID_BINKAUDIO_RDFT:     return "BINKAUDIO_RDFT";
  case CODEC_ID_BINKAUDIO_DCT:      return "BINKAUDIO_DCT";
#endif

  //////////////////////////////////////////////////////////////////////////////
  // Video codecs
  case CODEC_ID_MPEG1VIDEO:         return "MPEG1";
  case CODEC_ID_MPEG2VIDEO:         return "MPEG2";
  case CODEC_ID_H261:               return "H261";
  case CODEC_ID_H263:               return "H263";
  case CODEC_ID_RV10:               return "RV10";
  case CODEC_ID_RV20:               return "RV20";
  case CODEC_ID_MJPEG:              return "MJPEG";
  case CODEC_ID_MJPEGB:             return "MJPEGB";
  case CODEC_ID_LJPEG:              return "LJPEG";
  case CODEC_ID_SP5X:               return "SP5X";
  case CODEC_ID_JPEGLS:             return "JPEGLS";
  case CODEC_ID_MPEG4:              return "MPEG4";
  case CODEC_ID_RAWVIDEO:           return "RAWVIDEO";
  case CODEC_ID_MSMPEG4V1:          return "MSMPEG4V1";
  case CODEC_ID_MSMPEG4V2:          return "MSMPEG4V2";
  case CODEC_ID_MSMPEG4V3:          return "MSMPEG4V3";
  case CODEC_ID_WMV1:               return "WMV1";
  case CODEC_ID_WMV2:               return "WMV2";
  case CODEC_ID_H263P:              return "H263P";
  case CODEC_ID_H263I:              return "H263I";
  case CODEC_ID_FLV1:               return "FLV1";
  case CODEC_ID_SVQ1:               return "SVQ1";
  case CODEC_ID_SVQ3:               return "SVQ3";
  case CODEC_ID_DVVIDEO:            return "DVVIDEO";
  case CODEC_ID_HUFFYUV:            return "HUFFYUV";
  case CODEC_ID_CYUV:               return "CYUV";
  case CODEC_ID_H264:               return "H264";
  case CODEC_ID_INDEO3:             return "INDEO3";
  case CODEC_ID_VP3:                return "VP3";
  case CODEC_ID_THEORA:             return "THEORA";
  case CODEC_ID_ASV1:               return "ASV1";
  case CODEC_ID_ASV2:               return "ASV2";
  case CODEC_ID_FFV1:               return "FFV1";
  case CODEC_ID_4XM:                return "4XM";
  case CODEC_ID_VCR1:               return "VCR1";
  case CODEC_ID_CLJR:               return "CLJR";
  case CODEC_ID_MDEC:               return "MDEC";
  case CODEC_ID_ROQ:                return "ROQ";
  case CODEC_ID_INTERPLAY_VIDEO:    return "INTERPLAY_VIDEO";
  case CODEC_ID_XAN_WC3:            return "XAN_WC3";
  case CODEC_ID_XAN_WC4:            return "XAN_WC4";
  case CODEC_ID_RPZA:               return "RPZA";
  case CODEC_ID_CINEPAK:            return "CINEPAK";
  case CODEC_ID_WS_VQA:             return "WS_VQA";
  case CODEC_ID_MSRLE:              return "MSRLE";
  case CODEC_ID_MSVIDEO1:           return "MSVIDEO1";
  case CODEC_ID_IDCIN:              return "IDCIN";
  case CODEC_ID_8BPS:               return "8BPS";
  case CODEC_ID_SMC:                return "SMC";
  case CODEC_ID_FLIC:               return "FLIC";
  case CODEC_ID_TRUEMOTION1:        return "TRUEMOTION1";
  case CODEC_ID_VMDVIDEO:           return "VMDVIDEO";
  case CODEC_ID_MSZH:               return "MSZH";
  case CODEC_ID_ZLIB:               return "ZLIB";
  case CODEC_ID_QTRLE:              return "QTRLE";
  case CODEC_ID_SNOW:               return "SNOW";
  case CODEC_ID_TSCC:               return "TSCC";
  case CODEC_ID_ULTI:               return "ULTI";
  case CODEC_ID_QDRAW:              return "QDRAW";
  case CODEC_ID_VIXL:               return "VIXL";
  case CODEC_ID_QPEG:               return "QPEG";
  case CODEC_ID_XVID:               return "XVID";
  case CODEC_ID_PNG:                return "PNG";
  case CODEC_ID_PPM:                return "PPM";
  case CODEC_ID_PBM:                return "PBM";
  case CODEC_ID_PGM:                return "PGM";
  case CODEC_ID_PGMYUV:             return "PGMYUV";
  case CODEC_ID_PAM:                return "PAM";
  case CODEC_ID_FFVHUFF:            return "FFVHUFF";
  case CODEC_ID_RV30:               return "RV30";
  case CODEC_ID_RV40:               return "RV40";
  case CODEC_ID_VC1:                return "VC1";
  case CODEC_ID_WMV3:               return "WMV3";
  case CODEC_ID_LOCO:               return "LOCO";
  case CODEC_ID_WNV1:               return "WNV1";
  case CODEC_ID_AASC:               return "AASC";
  case CODEC_ID_INDEO2:             return "INDEO2";
  case CODEC_ID_FRAPS:              return "FRAPS";
  case CODEC_ID_TRUEMOTION2:        return "TRUEMOTION2";
  case CODEC_ID_BMP:                return "BMP";
  case CODEC_ID_CSCD:               return "CSCD";
  case CODEC_ID_MMVIDEO:            return "MMVIDEO";
  case CODEC_ID_ZMBV:               return "ZMBV";
  case CODEC_ID_AVS:                return "AVS";
  case CODEC_ID_SMACKVIDEO:         return "SMACKVIDEO";
  case CODEC_ID_NUV:                return "NUV";
  case CODEC_ID_KMVC:               return "KMVC";
  case CODEC_ID_FLASHSV:            return "FLASHSV";
  case CODEC_ID_CAVS:               return "CAVS";
  case CODEC_ID_JPEG2000:           return "JPEG2000";
  case CODEC_ID_VMNC:               return "VMNC";
  case CODEC_ID_VP5:                return "VP5";
  case CODEC_ID_VP6:                return "VP6";
  case CODEC_ID_VP6F:               return "VP6F";
  case CODEC_ID_TARGA:              return "TARGA";
  case CODEC_ID_DSICINVIDEO:        return "DSICINVIDEO";
  case CODEC_ID_TIERTEXSEQVIDEO:    return "TIERTEXSEQVIDEO";
  case CODEC_ID_TIFF:               return "TIFF";
  case CODEC_ID_GIF:                return "GIF";
  case CODEC_ID_FFH264:             return "FFH264";
  case CODEC_ID_DXA:                return "DXA";
  case CODEC_ID_DNXHD:              return "DNXHD";
  case CODEC_ID_THP:                return "THP";
  case CODEC_ID_SGI:                return "SGI";
  case CODEC_ID_C93:                return "C93";
  case CODEC_ID_BETHSOFTVID:        return "BETHSOFTVID";
  case CODEC_ID_PTX:                return "PTX";
  case CODEC_ID_TXD:                return "TXD";
  case CODEC_ID_VP6A:               return "VP6A";
  case CODEC_ID_AMV:                return "AMV";
  case CODEC_ID_VB:                 return "VB";
  case CODEC_ID_PCX:                return "PCX";
  case CODEC_ID_SUNRAST:            return "SUNRAST";
  case CODEC_ID_INDEO4:             return "INDEO4";
  case CODEC_ID_INDEO5:             return "INDEO5";
  case CODEC_ID_MIMIC:              return "MIMIC";
  case CODEC_ID_RL2:                return "RL2";
  case CODEC_ID_8SVX_EXP:           return "8SVX_EXP";
  case CODEC_ID_8SVX_FIB:           return "8SVX_FIB";
  case CODEC_ID_ESCAPE124:          return "ESCAPE124";
  case CODEC_ID_DIRAC:              return "DIRAC";
  case CODEC_ID_BFI:                return "BFI";
  case CODEC_ID_CMV:                return "CMV";
  case CODEC_ID_MOTIONPIXELS:       return "MOTIONPIXELS";
  case CODEC_ID_TGV:                return "TGV";
  case CODEC_ID_TGQ:                return "TGQ";
  case CODEC_ID_TQI:                return "TQI";
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  case CODEC_ID_AURA:               return "AURA";
  case CODEC_ID_AURA2:              return "AURA2";
  case CODEC_ID_V210X:              return "V210X";
  case CODEC_ID_TMV:                return "TMV";
  case CODEC_ID_V210:               return "V210";
  case CODEC_ID_DPX:                return "DPX";
  case CODEC_ID_MAD:                return "MAD";
  case CODEC_ID_FRWU:               return "FRWU";
  case CODEC_ID_FLASHSV2:           return "FLASHSV2";
  case CODEC_ID_CDGRAPHICS:         return "CDGRAPHICS";
  case CODEC_ID_R210:               return "R210";
  case CODEC_ID_ANM:                return "ANM";
  case CODEC_ID_BINKVIDEO:          return "BINKVIDEO";
  case CODEC_ID_IFF_ILBM:           return "IFF_ILBM";
  case CODEC_ID_IFF_BYTERUN1:       return "IFF_BYTERUN1";
  case CODEC_ID_KGV1:               return "KGV1";
  case CODEC_ID_YOP:                return "YOP";
  case CODEC_ID_VP8:                return "VP8";
#endif

  //////////////////////////////////////////////////////////////////////////////
  // Audio formats
  case CODEC_ID_PCM_S16LE:          return "PCM/S16LE";
  case CODEC_ID_PCM_S16BE:          return "PCM/S16BE";
  case CODEC_ID_PCM_U16LE:          return "PCM/U16LE";
  case CODEC_ID_PCM_U16BE:          return "PCM/U16BE";
  case CODEC_ID_PCM_S8:             return "PCM/S8";
  case CODEC_ID_PCM_U8:             return "PCM/U8";
  case CODEC_ID_PCM_MULAW:          return "PCM/MULAW";
  case CODEC_ID_PCM_ALAW:           return "PCM/ALAW";
  case CODEC_ID_PCM_S32LE:          return "PCM/S32LE";
  case CODEC_ID_PCM_S32BE:          return "PCM/S32BE";
  case CODEC_ID_PCM_U32LE:          return "PCM/U32LE";
  case CODEC_ID_PCM_U32BE:          return "PCM/U32BE";
  case CODEC_ID_PCM_S24LE:          return "PCM/S24LE";
  case CODEC_ID_PCM_S24BE:          return "PCM/S24BE";
  case CODEC_ID_PCM_U24LE:          return "PCM/U24LE";
  case CODEC_ID_PCM_U24BE:          return "PCM/U24BE";
  case CODEC_ID_PCM_S24DAUD:        return "PCM/S24DAUD";
  case CODEC_ID_PCM_ZORK:           return "PCM/ZORK";
  case CODEC_ID_PCM_S16LE_PLANAR:   return "PCM/S16LEP";
  case CODEC_ID_PCM_DVD:            return "PCM/DVD";
  case CODEC_ID_PCM_F32BE:          return "PCM/F32BE";
  case CODEC_ID_PCM_F32LE:          return "PCM/F32LE";
  case CODEC_ID_PCM_F64BE:          return "PCM/F64BE";
  case CODEC_ID_PCM_F64LE:          return "PCM/F64LE";

  //////////////////////////////////////////////////////////////////////////////
  // Subtitle codecs
  case CODEC_ID_DVD_SUBTITLE:       return "SUB/DVD";
  case CODEC_ID_DVB_SUBTITLE:       return "SUB/DVB";
  case CODEC_ID_TEXT:               return "SUB/RAWUTF8";
  case CODEC_ID_XSUB:               return "SUB/XSUB";
  case CODEC_ID_SSA:                return "SUB/SSA";
  case CODEC_ID_MOV_TEXT:           return "SUB/MOV_TEXT";
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  case CODEC_ID_HDMV_PGS_SUBTITLE:  return "SUB/HDMV_PGS";
  case CODEC_ID_DVB_TELETEXT:       return "SUB/DVB_TELETEXT";
#endif
  }
}

::PixelFormat FFMpegCommon::toFFMpegPixelFormat(SVideoFormat::Format format)
{
  switch (format)
  {
  case SVideoFormat::Format_RGB555:       return PIX_FMT_RGB555;
  case SVideoFormat::Format_BGR555:       return PIX_FMT_BGR555;
  case SVideoFormat::Format_RGB565:       return PIX_FMT_RGB565;
  case SVideoFormat::Format_BGR565:       return PIX_FMT_BGR565;
  case SVideoFormat::Format_RGB24:        return PIX_FMT_RGB24;
  case SVideoFormat::Format_BGR24:        return PIX_FMT_BGR24;
  case SVideoFormat::Format_RGB32:        return PIX_FMT_RGB32;
  case SVideoFormat::Format_BGR32:        return PIX_FMT_BGR32;
  case SVideoFormat::Format_GRAY8:        return PIX_FMT_GRAY8;
  case SVideoFormat::Format_GRAY16BE:     return PIX_FMT_GRAY16BE;
  case SVideoFormat::Format_GRAY16LE:     return PIX_FMT_GRAY16LE;
  case SVideoFormat::Format_YUYV422:      return PIX_FMT_YUYV422;
  case SVideoFormat::Format_UYVY422:      return PIX_FMT_UYVY422;
  case SVideoFormat::Format_YUV410P:      return PIX_FMT_YUV410P;
  case SVideoFormat::Format_YUV411P:      return PIX_FMT_YUV411P;
  case SVideoFormat::Format_YUV420P:      return PIX_FMT_YUV420P;
  case SVideoFormat::Format_YUV422P:      return PIX_FMT_YUV422P;
  case SVideoFormat::Format_YUV444P:      return PIX_FMT_YUV444P;
  default:                                return PIX_FMT_NONE;
  }
}

SVideoFormat::Format FFMpegCommon::fromFFMpegPixelFormat(::PixelFormat pf)
{
  switch (pf)
  {
  default:
  case PIX_FMT_NONE:         return SVideoFormat::Format_Invalid;

  case PIX_FMT_RGB555:       return SVideoFormat::Format_RGB555;
  case PIX_FMT_BGR555:       return SVideoFormat::Format_BGR555;
  case PIX_FMT_RGB565:       return SVideoFormat::Format_RGB565;
  case PIX_FMT_BGR565:       return SVideoFormat::Format_BGR565;
  case PIX_FMT_RGB24:        return SVideoFormat::Format_RGB24;
  case PIX_FMT_BGR24:        return SVideoFormat::Format_BGR24;
  case PIX_FMT_RGB32:        return SVideoFormat::Format_RGB32;
  case PIX_FMT_BGR32:        return SVideoFormat::Format_BGR32;
  case PIX_FMT_GRAY8:        return SVideoFormat::Format_GRAY8;
  case PIX_FMT_GRAY16BE:     return SVideoFormat::Format_GRAY16BE;
  case PIX_FMT_GRAY16LE:     return SVideoFormat::Format_GRAY16LE;
  case PIX_FMT_YUYV422:      return SVideoFormat::Format_YUYV422;
  case PIX_FMT_UYVY422:      return SVideoFormat::Format_UYVY422;
  case PIX_FMT_YUV410P:      return SVideoFormat::Format_YUV410P;
  case PIX_FMT_YUV411P:      return SVideoFormat::Format_YUV411P;
  case PIX_FMT_YUV420P:
  case PIX_FMT_YUVJ420P:     return SVideoFormat::Format_YUV420P;
  case PIX_FMT_YUV422P:
  case PIX_FMT_YUVJ422P:     return SVideoFormat::Format_YUV422P;
  case PIX_FMT_YUV444P:
  case PIX_FMT_YUVJ444P:     return SVideoFormat::Format_YUV444P;
  }
}

int64_t FFMpegCommon::toFFMpegChannelLayout(SAudioFormat::Channels channels)
{
  switch (channels)
  {
  case SAudioFormat::Channels_Mono:             return CH_LAYOUT_MONO;
  case SAudioFormat::Channels_Stereo:           return CH_LAYOUT_STEREO;
  case SAudioFormat::Channels_Quadraphonic:     return CH_LAYOUT_QUAD;
  case SAudioFormat::Channels_Surround_3_0:     return CH_LAYOUT_SURROUND;
#ifdef CH_LAYOUT_4POINT0
  case SAudioFormat::Channels_Surround_4_0:     return CH_LAYOUT_4POINT0;
#endif
  case SAudioFormat::Channels_Surround_5_0:     return CH_LAYOUT_5POINT0;
  case SAudioFormat::Channels_Surround_5_1:     return CH_LAYOUT_5POINT1;
#ifdef CH_LAYOUT_5POINT0_BACK
  case SAudioFormat::Channels_Surround_6_0:     return CH_LAYOUT_5POINT0_BACK;
#endif
#ifdef CH_LAYOUT_5POINT1_BACK
  case SAudioFormat::Channels_Surround_6_1:     return CH_LAYOUT_5POINT1_BACK;
#endif
  case SAudioFormat::Channels_Surround_7_1:     return CH_LAYOUT_7POINT1;
  case SAudioFormat::Channels_Surround_7_1_Wide:return CH_LAYOUT_7POINT1_WIDE;
  default:
    {
      int64_t result = 0;

      if ((channels & SAudioFormat::Channel_LeftFront) != 0)                result |= CH_FRONT_LEFT;
      if ((channels & SAudioFormat::Channel_RightFront) != 0)               result |= CH_FRONT_RIGHT;
      if ((channels & SAudioFormat::Channel_CenterFront) != 0)              result |= CH_FRONT_CENTER;
      if ((channels & SAudioFormat::Channel_LowFrequencyEffects) != 0)      result |= CH_LOW_FREQUENCY;
      if ((channels & SAudioFormat::Channel_LeftBack) != 0)                 result |= CH_BACK_LEFT;
      if ((channels & SAudioFormat::Channel_RightBack) != 0)                result |= CH_BACK_RIGHT;
      if ((channels & SAudioFormat::Channel_CenterLeftFront) != 0)          result |= CH_FRONT_LEFT_OF_CENTER;
      if ((channels & SAudioFormat::Channel_CenterRightFront) != 0)         result |= CH_FRONT_RIGHT_OF_CENTER;
      if ((channels & SAudioFormat::Channel_CenterBack) != 0)               result |= CH_BACK_CENTER;
      if ((channels & SAudioFormat::Channel_LeftSide) != 0)                 result |= CH_SIDE_LEFT;
      if ((channels & SAudioFormat::Channel_RightSide) != 0)                result |= CH_SIDE_RIGHT;
      if ((channels & SAudioFormat::Channel_TopLeftFront) != 0)             result |= CH_TOP_FRONT_LEFT;
      if ((channels & SAudioFormat::Channel_TopCenterFront) != 0)           result |= CH_TOP_FRONT_CENTER;
      if ((channels & SAudioFormat::Channel_TopRightFront) != 0)            result |= CH_TOP_FRONT_RIGHT;
      if ((channels & SAudioFormat::Channel_TopLeftBack) != 0)              result |= CH_TOP_BACK_LEFT;
      if ((channels & SAudioFormat::Channel_TopCenterBack) != 0)            result |= CH_TOP_BACK_CENTER;
      if ((channels & SAudioFormat::Channel_TopRightBack) != 0)             result |= CH_TOP_BACK_RIGHT;

      return result;
    }
  }
}

SAudioFormat::Channels FFMpegCommon::fromFFMpegChannelLayout(int64_t layout, int channels)
{
#ifndef CH_LAYOUT_4POINT0
#define CH_LAYOUT_4POINT0 (CH_LAYOUT_SURROUND|CH_BACK_CENTER)
#endif
#ifndef CH_LAYOUT_5POINT0_BACK
#define CH_LAYOUT_5POINT0_BACK (CH_LAYOUT_SURROUND|CH_BACK_LEFT|CH_BACK_RIGHT)
#endif
#ifndef CH_LAYOUT_5POINT1_BACK
#define CH_LAYOUT_5POINT1_BACK (CH_LAYOUT_5POINT0_BACK|CH_LOW_FREQUENCY)
#endif
#ifndef CH_LAYOUT_6POINT0
#define CH_LAYOUT_6POINT0 (CH_LAYOUT_5POINT0|CH_BACK_CENTER)
#endif
#ifndef CH_LAYOUT_6POINT1
#define CH_LAYOUT_6POINT1 (CH_LAYOUT_5POINT1|CH_BACK_CENTER)
#endif

  switch (layout)
  {
  case CH_LAYOUT_MONO:              return SAudioFormat::Channels_Mono;
  case CH_LAYOUT_STEREO:            return SAudioFormat::Channels_Stereo;
  case CH_LAYOUT_QUAD:              return SAudioFormat::Channels_Quadraphonic;
  case CH_LAYOUT_SURROUND:          return SAudioFormat::Channels_Surround_3_0;
  case CH_LAYOUT_4POINT0:           return SAudioFormat::Channels_Surround_4_0;
  case CH_LAYOUT_5POINT0:           return SAudioFormat::Channels_Surround_5_0;
  case CH_LAYOUT_5POINT0_BACK:      return SAudioFormat::Channels_Surround_5_0;
  case CH_LAYOUT_5POINT1:           return SAudioFormat::Channels_Surround_5_1;
  case CH_LAYOUT_5POINT1_BACK:      return SAudioFormat::Channels_Surround_5_1;
  case CH_LAYOUT_6POINT0:           return SAudioFormat::Channels_Surround_6_0;
  case CH_LAYOUT_6POINT1:           return SAudioFormat::Channels_Surround_6_1;
  case CH_LAYOUT_7POINT1:           return SAudioFormat::Channels_Surround_7_1;
  case CH_LAYOUT_7POINT1_WIDE:      return SAudioFormat::Channels_Surround_7_1_Wide;
  case 0:                           return SAudioFormat::guessChannels(channels);
  default:
    {
      SAudioFormat::Channels result = 0;

      if ((layout & CH_FRONT_LEFT) != 0)                result |= SAudioFormat::Channel_LeftFront;
      if ((layout & CH_FRONT_RIGHT) != 0)               result |= SAudioFormat::Channel_RightFront;
      if ((layout & CH_FRONT_CENTER) != 0)              result |= SAudioFormat::Channel_CenterFront;
      if ((layout & CH_LOW_FREQUENCY) != 0)             result |= SAudioFormat::Channel_LowFrequencyEffects;
      if ((layout & CH_BACK_LEFT) != 0)                 result |= SAudioFormat::Channel_LeftBack;
      if ((layout & CH_BACK_RIGHT) != 0)                result |= SAudioFormat::Channel_RightBack;
      if ((layout & CH_FRONT_LEFT_OF_CENTER) != 0)      result |= SAudioFormat::Channel_CenterLeftFront;
      if ((layout & CH_FRONT_RIGHT_OF_CENTER) != 0)     result |= SAudioFormat::Channel_CenterRightFront;
      if ((layout & CH_BACK_CENTER) != 0)               result |= SAudioFormat::Channel_CenterBack;
      if ((layout & CH_SIDE_LEFT) != 0)                 result |= SAudioFormat::Channel_LeftSide;
      if ((layout & CH_SIDE_RIGHT) != 0)                result |= SAudioFormat::Channel_RightSide;
      if ((layout & CH_TOP_FRONT_LEFT) != 0)            result |= SAudioFormat::Channel_TopLeftFront;
      if ((layout & CH_TOP_FRONT_CENTER) != 0)          result |= SAudioFormat::Channel_TopCenterFront;
      if ((layout & CH_TOP_FRONT_RIGHT) != 0)           result |= SAudioFormat::Channel_TopRightFront;
      if ((layout & CH_TOP_BACK_LEFT) != 0)             result |= SAudioFormat::Channel_TopLeftBack;
      if ((layout & CH_TOP_BACK_CENTER) != 0)           result |= SAudioFormat::Channel_TopCenterBack;
      if ((layout & CH_TOP_BACK_RIGHT) != 0)            result |= SAudioFormat::Channel_TopRightBack;

      return result;
    }
  }
}

::AVPacket FFMpegCommon::toAVPacket(const SEncodedAudioBuffer &buffer, ::AVStream *stream)
{
  ::AVPacket packet;
  ::av_init_packet(&packet);

  packet.data = (uint8_t *)buffer.data();
  packet.size = buffer.size();

  if (stream)
  {
    packet.stream_index = stream->index;

    if (buffer.presentationTimeStamp().isValid())
      packet.pts = buffer.presentationTimeStamp().toClock(stream->time_base.num, stream->time_base.den);
    else
      packet.pts = AV_NOPTS_VALUE;

    if (buffer.decodingTimeStamp().isValid())
      packet.dts = buffer.decodingTimeStamp().toClock(stream->time_base.num, stream->time_base.den);
    else
      packet.dts = packet.pts;

    if ((packet.pts != AV_NOPTS_VALUE) && (packet.dts != AV_NOPTS_VALUE) && (packet.dts > packet.pts))
      packet.pts = packet.dts;

    stream->pts.val = packet.pts;
  }

  packet.flags |=
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
    PKT_FLAG_KEY
#else
    AV_PKT_FLAG_KEY
#endif
    ;

  return packet;
}

::AVPacket FFMpegCommon::toAVPacket(const SEncodedVideoBuffer &buffer, ::AVStream *stream)
{
  ::AVPacket packet;
  ::av_init_packet(&packet);

  packet.data = (uint8_t *)buffer.data();
  packet.size = buffer.size();

  if (stream)
  {
    packet.stream_index = stream->index;

    if (buffer.presentationTimeStamp().isValid())
      packet.pts = buffer.presentationTimeStamp().toClock(stream->time_base.num, stream->time_base.den);
    else
      packet.pts = AV_NOPTS_VALUE;

    if (buffer.decodingTimeStamp().isValid())
      packet.dts = buffer.decodingTimeStamp().toClock(stream->time_base.num, stream->time_base.den);
    else
      packet.dts = packet.pts;

    if ((packet.pts != AV_NOPTS_VALUE) && (packet.dts != AV_NOPTS_VALUE) && (packet.dts > packet.pts))
      packet.pts = packet.dts;

    stream->pts.val = packet.pts;
  }

  packet.flags |= buffer.isKeyFrame() ?
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
    PKT_FLAG_KEY
#else
    AV_PKT_FLAG_KEY
#endif
    : 0;

  return packet;
}

::AVPacket FFMpegCommon::toAVPacket(const SEncodedDataBuffer &buffer, ::AVStream *stream)
{
  ::AVPacket packet;
  ::av_init_packet(&packet);

  packet.data = (uint8_t *)buffer.data();
  packet.size = buffer.size();

  if (stream)
  {
    packet.stream_index = stream->index;

    if (buffer.presentationTimeStamp().isValid())
      packet.pts = buffer.presentationTimeStamp().toClock(stream->time_base.num, stream->time_base.den);
    else
      packet.pts = AV_NOPTS_VALUE;

    if (buffer.decodingTimeStamp().isValid())
      packet.dts = buffer.decodingTimeStamp().toClock(stream->time_base.num, stream->time_base.den);
    else
      packet.dts = packet.pts;

    if (buffer.duration().isValid())
      packet.duration = buffer.duration().toClock(stream->time_base.num, stream->time_base.den);

    if ((packet.pts != AV_NOPTS_VALUE) && (packet.dts != AV_NOPTS_VALUE) && (packet.dts > packet.pts))
      packet.pts = packet.dts;

    stream->pts.val = packet.pts;
  }

  packet.flags |=
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 72, 0)
    PKT_FLAG_KEY
#else
    AV_PKT_FLAG_KEY
#endif
    ;

  return packet;
}

#ifdef OPT_ENABLE_THREADS
int FFMpegCommon::encodeThreadCount(::CodecID codec)
{
  int limit;
  switch (codec)
  {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
  case CODEC_ID_MPEG2VIDEO:
    limit = threadLimit;
    break;
#endif

  default:
    limit = 1;
    break;
  }

  return qBound(1, QThreadPool::globalInstance()->maxThreadCount(), limit);
}

int FFMpegCommon::decodeThreadCount(::CodecID codec)
{
  int limit;
  switch (codec)
  {
  //case CODEC_ID_MPEG2VIDEO: // Buggy
  case CODEC_ID_H263:
  case CODEC_ID_H264:
  case CODEC_ID_THEORA:
  case CODEC_ID_FFH264:
    limit = threadLimit;
    break;

  default:
    limit = 1;
    break;
  }

  return qBound(1, QThreadPool::globalInstance()->maxThreadCount(), limit);
}

int FFMpegCommon::execute(::AVCodecContext *c, int (*func)(::AVCodecContext *c2, void *arg), void *arg2, int *ret, int count, int size)
{
  if ((count > 1) && (c->thread_count > 1))
  {
    QVector< QFuture<int> > future;
    future.reserve(count);

    for (int i=0; i<count; i++)
      future += QtConcurrent::run(&FFMpegCommon::executeTask, func, c, reinterpret_cast<char *>(arg2) + (size * i));

    for (int i=0; i<count; i++)
    {
      future[i].waitForFinished();
      if (ret)
        ret[i] = future[i].result();
    }
  }
  else // Single-threaded.
  {
    for (int i=0; i<count; i++)
    {
      if (ret)
        ret[i] = func(c, arg2);
      else
        func(c, arg2);
    }
  }

  return 0;
}

int FFMpegCommon::executeTask(int (*func)(::AVCodecContext *, void *), ::AVCodecContext *c, void *arg)
{
  LXI_PROFILE_FUNCTION;

  return func(c, arg);
}

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 72, 0)
int FFMpegCommon::execute2(::AVCodecContext *c, int (*func)(::AVCodecContext *c2, void *arg, int jobnr, int threadnr), void *arg2, int *ret, int count)
{
  if ((count > 1) && (c->thread_count > 1))
  {
    QVector< QFuture<int> > future;
    future.reserve(c->thread_count);

    for (int i=0; i<count; i+=c->thread_count)
    {
      for (int j=0; (j<c->thread_count) && ((i+j)<count); j++)
        future += QtConcurrent::run(&FFMpegCommon::execute2Task, func, c, arg2, i, j);

      for (int j=0; j<future.count(); j++)
      {
        future[j].waitForFinished();
        if (ret)
          ret[i+j] = future[j].result();
      }

      future.clear();
    }
  }
  else // Single-threaded.
  {
    for (int i=0; i<count; i++)
    {
      if (ret)
        ret[i] = func(c, arg2, i, 0);
      else
        func(c, arg2, i, 0);
    }
  }

  return 0;
}

int FFMpegCommon::execute2Task(int (*func)(::AVCodecContext *, void *, int, int), ::AVCodecContext *c, void *arg, int jobnr, int threadnr)
{
  LXI_PROFILE_FUNCTION;

  return func(c, arg, jobnr, threadnr);
}
#endif
#endif

void FFMpegCommon::log(void *, int level, const char *fmt, va_list vl)
{
  if ((level <= logLevel) && !logDisabled)
  {
    //const ::AVClass * const * const c = reinterpret_cast<const ::AVClass * const *>(ptr);

    char buffer[4096]; buffer[sizeof(buffer) - 1] = '\0';
    ::vsnprintf(buffer, sizeof(buffer) - 1, fmt, vl);

    // Trim trailing whitespace.
    for (int i=::strlen(buffer); i>=0; i--)
    if ((buffer[i] >= '\0') && (buffer[i] <= ' '))
      buffer[i] = '\0';
    else
      break;

    if (level >= AV_LOG_INFO)
      qDebug("FFMpeg info: %s", buffer);
    else if (level >= AV_LOG_FATAL)
      qDebug("FFMpeg fatal: %s", buffer);
#ifdef AV_LOG_PANIC
    else if (level >= AV_LOG_PANIC)
      qFatal("FFMpeg panic: %s", buffer);
#endif
    else
      qDebug("FFMpeg debug: %s", buffer);
  }
}

int FFMpegCommon::lock(void **mutex, AVLockOp op)
{
  switch (op)
  {
  case AV_LOCK_CREATE:
    *mutex = new QMutex();
    return 0;

  case AV_LOCK_OBTAIN:
    static_cast<QMutex *>(*mutex)->lock();
    return 0;

  case AV_LOCK_RELEASE:
    static_cast<QMutex *>(*mutex)->unlock();
    return 0;

  case AV_LOCK_DESTROY:
    delete static_cast<QMutex *>(*mutex);
    return 0;
  }

  return -1;
}

} } // End of namespaces
