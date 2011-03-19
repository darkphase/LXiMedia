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

#include "formatprober.h"
#include "smediainfo.h"
#include "psbufferwriter.h"

namespace LXiStream {
namespace Common {


FormatProber::FormatProber(const QString &, QObject *parent)
  : SInterfaces::FormatProber(parent)
{
}

FormatProber::~FormatProber()
{
}

QList<FormatProber::Format> FormatProber::probeFormat(const QByteArray &, const QString &)
{
  QList<Format> formats;

  //if (data.startsWith(PsBufferWriter::buildHeader()))
  //  formats += Format(PsBufferWriter::formatName, 1);

  return formats;
}

void FormatProber::probeMetadata(ProbeInfo &pi, ReadCallback *callback)
{
  if (callback)
  if (!callback->path.isEmpty())
  {
    const QFileInfo info(callback->path);
    const QString suffix = info.suffix().toLower();

    if (pi.programs.isEmpty())
      pi.programs.append(ProbeInfo::Program());

    ProbeInfo::Program &program = pi.programs.first();
    if (imageSuffixes().contains(suffix))
    {
      program.imageCodec = SVideoCodec(suffix.toUpper());
      pi.fileTypeName = imageDescription(suffix);
    }
    else if (audioSuffixes().contains(suffix))
    {
      program.audioStreams = QList<AudioStreamInfo>() << AudioStreamInfo(0, NULL, SAudioCodec(suffix.toUpper()));
      pi.fileTypeName = audioDescription(suffix);

      splitFileName(info.completeBaseName(), pi.title, pi.author, pi.album, pi.track);
    }
    else if (videoSuffixes().contains(suffix))
    {
      program.audioStreams = QList<AudioStreamInfo>() << AudioStreamInfo(0, NULL, SAudioCodec(suffix.toUpper()));
      program.videoStreams = QList<VideoStreamInfo>() << VideoStreamInfo(0, NULL, SVideoCodec(suffix.toUpper()));
      pi.fileTypeName = videoDescription(suffix);

      splitFileName(info.completeBaseName(), pi.title, pi.author, pi.album, pi.track);

      QDir dir = info.absoluteDir();
      QString dirName = dir.dirName();

      if (dirName.contains("season", Qt::CaseInsensitive) &&
          (dirName.length() < 10))
      {
        pi.track = (pi.track % SMediaInfo::tvShowSeason) + (dirName.mid(7).toUInt() * SMediaInfo::tvShowSeason);

        const QString path = dir.absolutePath();

        dir = QDir(path.left(path.length() - dirName.length()));
        dirName = dir.dirName();
      }

      if (pi.track > 0)
      {
        QString dummy1, dummy2;
        unsigned dummy3 = 0;
        splitFileName(dirName, pi.album, dummy1, dummy2, dummy3);
      }
      else
        pi.title = info.completeBaseName();
    }

    if (pi.title.length() <= 3)
      pi.title = info.completeBaseName();
  }
}

void FormatProber::splitFileName(QString baseName, QString &title, QString &author, QString &album, unsigned &episode)
{
  static const QString videoChars = " &()-[]";

  if (baseName.length() > 0)
  {
    for (QString::Iterator c=baseName.begin(); c!=baseName.end(); c++)
    if (!c->isLetterOrNumber() && !videoChars.contains(*c))
      *c = ' ';

    baseName.replace("[", " [ ");
    baseName.replace("]", " ] ");

    int openBrackets = 0;
    QString *process = &author;

    // Finds sNNeNN or NNxNN
    foreach (const QString &word, baseName.split(' ', QString::SkipEmptyParts))
    {
      unsigned se = 0, ep = 0, digits = 0;

      if (word == "-")
      {
        if (openBrackets == 0)
        {
          process = &title;
          continue;
        }
      }
      else if (word == "[")
      {
        if (openBrackets++ == 0)
        {
          title = *process;
          process = &author;
          process->clear();
          continue;
        }
      }
      else if (word == "]")
      {
        if (--openBrackets == 0)
        {
          process = &title;
          continue;
        }
      }

      if (openBrackets > 0)
      {
        *process = (*process + ' ' + word).trimmed();
      }
      else
      {
        if (episode == 0)
        foreach (QChar c, word)
        {
          if (c.isNumber())
          {
            ep = (ep * 10) + unsigned(c.toAscii() - '0');
            digits++;
          }
          else if ((c.toLower() == 'e') || (c.toLower() == 'x'))
          {
            if ((ep > 0) && (se == 0))
            {
              se = ep;
              ep = 0;
            }
            else
              break; // Probably double-episode
          }
          else if (c.toLower() != 's')
          {
            se = ep = 0;
            break;
          }
        }

        // Larger than 1900 is a year, 576, 720 and 1080 are HD resolutions, and
        // 264 is a codec.
        if ((digits < 2) || (ep == 0) || (ep >= 1900) ||
            (ep == 576) || (ep == 720) || (ep == 1080) ||
            (ep == 264))
        {
          *process = (*process + ' ' + word).trimmed();
        }
        else
        {
          // Episode number NNNN
          if ((se == 0) && (ep >= SMediaInfo::tvShowSeason))
          {
            se = ep / SMediaInfo::tvShowSeason;
            ep = ep % SMediaInfo::tvShowSeason;
          }

          episode = (se * SMediaInfo::tvShowSeason) + ep;
          album = *process;
          author.clear();
          process = &title;
        }
      }
    }

    if (title.length() == 0)
    {
      title = *process;
      process->clear();
    }
  }
}

QString FormatProber::toGenre(const QString &text)
{
  bool ok;
  const int genreId = text.toInt(&ok);

  if (ok && (QString::number(genreId) == text))
  {
    switch (genreId)
    {
    case 0: return tr("Blues");           case 20: return tr("Alternative");      case 40: return tr("AlternRock");           case 60: return tr("Top 40");
    case 1: return tr("Classic Rock");    case 21: return tr("Ska");              case 41: return tr("Bass");                 case 61: return tr("Christian Rap");
    case 2: return tr("Country");         case 22: return tr("Death Metal");      case 42: return tr("Soul");                 case 62: return tr("Pop/Funk");
    case 3: return tr("Dance");           case 23: return tr("Pranks");           case 43: return tr("Punk");                 case 63: return tr("Jungle");
    case 4: return tr("Disco");           case 24: return tr("Soundtrack");       case 44: return tr("Space");                case 64: return tr("Native American");
    case 5: return tr("Funk");            case 25: return tr("Euro-Techno");      case 45: return tr("Meditative");           case 65: return tr("Cabaret");
    case 6: return tr("Grunge");          case 26: return tr("Ambient");          case 46: return tr("Instrumental Pop");     case 66: return tr("New Wave");
    case 7: return tr("Hip-Hop");         case 27: return tr("Trip-Hop");         case 47: return tr("Instrumental Rock");    case 67: return tr("Psychadelic");
    case 8: return tr("Jazz");            case 28: return tr("Vocal");            case 48: return tr("Ethnic");               case 68: return tr("Rave");
    case 9: return tr("Metal");           case 29: return tr("Jazz+Funk");        case 49: return tr("Gothic");               case 69: return tr("Showtunes");
    case 10: return tr("New Age");        case 30: return tr("Fusion");           case 50: return tr("Darkwave");             case 70: return tr("Trailer");
    case 11: return tr("Oldies");         case 31: return tr("Trance");           case 51: return tr("Techno-Industrial");    case 71: return tr("Lo-Fi");
    case 12: return tr("Other");          case 32: return tr("Classical");        case 52: return tr("Electronic");           case 72: return tr("Tribal");
    case 13: return tr("Pop");            case 33: return tr("Instrumental");     case 53: return tr("Pop-Folk");             case 73: return tr("Acid Punk");
    case 14: return tr("R&B");            case 34: return tr("Acid");             case 54: return tr("Eurodance");            case 74: return tr("Acid Jazz");
    case 15: return tr("Rap");            case 35: return tr("House");            case 55: return tr("Dream");                case 75: return tr("Polka");
    case 16: return tr("Reggae");         case 36: return tr("Game");             case 56: return tr("Southern Rock");        case 76: return tr("Retro");
    case 17: return tr("Rock");           case 37: return tr("Sound Clip");       case 57: return tr("Comedy");               case 77: return tr("Musical");
    case 18: return tr("Techno");         case 38: return tr("Gospel");           case 58: return tr("Cult");                 case 78: return tr("Rock & Roll");
    case 19: return tr("Industrial");     case 39: return tr("Noise");            case 59: return tr("Gangsta");              case 79: return tr("Hard Rock");

    case 80: return tr("Folk");           case 92: return tr("Progressive Rock"); case 104: return tr("Chamber Music");       case 116: return tr("Ballad");
    case 81: return tr("Folk-Rock");      case 93: return tr("Psychedelic Rock"); case 105: return tr("Sonata");              case 117: return tr("Poweer Ballad");
    case 82: return tr("National Folk");  case 94: return tr("Symphonic Rock");   case 106: return tr("Symphony");            case 118: return tr("Rhytmic Soul");
    case 83: return tr("Swing");          case 95: return tr("Slow Rock");        case 107: return tr("Booty Brass");         case 119: return tr("Freestyle");
    case 84: return tr("Fast Fusion");    case 96: return tr("Big Band");         case 108: return tr("Primus");              case 120: return tr("Duet");
    case 85: return tr("Bebob");          case 97: return tr("Chorus");           case 109: return tr("Porn Groove");         case 121: return tr("Punk Rock");
    case 86: return tr("Latin");          case 98: return tr("Easy Listening");   case 110: return tr("Satire");              case 122: return tr("Drum Solo");
    case 87: return tr("Revival");        case 99: return tr("Acoustic");         case 111: return tr("Slow Jam");            case 123: return tr("A Capela");
    case 88: return tr("Celtic");         case 100: return tr("Humour");          case 112: return tr("Club");                case 124: return tr("Euro-House");
    case 89: return tr("Bluegrass");      case 101: return tr("Speech");          case 113: return tr("Tango");               case 125: return tr("Dance Hall");
    case 90: return tr("Avantgarde");     case 102: return tr("Chanson");         case 114: return tr("Samba");
    case 91: return tr("Gothic Rock");    case 103: return tr("Opera");           case 115: return tr("Folklore");

    default: return text;
    }
  }
  else
    return text;
}

QString FormatProber::audioDescription(const QString &suffix)
{
  if (suffix == "dts")
    return "Digital Theater Systems surround";

  return "Audio";
}

QString FormatProber::videoDescription(const QString &suffix)
{
  if ((suffix == "3gp") || (suffix == "3g2"))
    return "Mobile video";
  else if ((suffix == "flv") || (suffix == "swf"))
    return "Flash video";
  else if ((suffix == "mpg") || (suffix == "mpeg") || (suffix == "ps") || (suffix == "vob"))
    return "MPEG Program Stream";
  else if ((suffix == "ogv") || (suffix == "oga") || (suffix == "ogx") || (suffix == "ogg") || (suffix == "spx"))
    return "Flash video";
  else if (suffix == "ts")
    return "MPEG Transport Stream";

  return "Video";
}

QString FormatProber::imageDescription(const QString &suffix)
{
  if ((suffix == "ppm") || (suffix == "png") || (suffix == "bmp"))
    return "Portable Pixmap";
  else if (suffix == "ico")
    return "Icon";
  else if ((suffix == "jpg") || (suffix == "jpe") || (suffix == "jpeg"))
    return "JPEG Compressed";
  else if ((suffix == "tif") || (suffix == "tiff"))
    return "TIFF Raw";
  else if (suffix == "svg")
    return "Vector Graphics";
  else if ((suffix == "raw"))
    return "Generic Raw";
  else if (suffix == "3fr")
    return "Hasselblad Raw";
  else if ((suffix == "arw") || (suffix == "srf") || (suffix == "sr2"))
    return "Sony Raw";
  else if (suffix == "bay")
    return "Casio Raw";
  else if ((suffix == "crw") || (suffix == "cr2"))
    return "Canon Raw";
  else if ((suffix == "cap") || (suffix == "iiq") || (suffix == "eip"))
    return "Phase One Raw";
  else if ((suffix == "dcs") || (suffix == "dcr") || (suffix == "drf") || (suffix == "k25") || (suffix == "kdc"))
    return "Kodak Raw";
  else if (suffix == "dng")
    return "Adobe Raw";
  else if (suffix == "erf")
    return "Epson Raw";
  else if (suffix == "fff")
    return "Imacon Raw";
  else if (suffix == "mef")
    return "Mamiya Raw";
  else if (suffix == "mos")
    return "Leaf Raw";
  else if (suffix == "mrw")
    return "Minolta Raw";
  else if ((suffix == "nef") || (suffix == "nrw"))
    return "Nikon Raw";
  else if (suffix == "orf")
    return "Olympus Raw";
  else if ((suffix == "ptx") || (suffix == "pef"))
    return "Pentax Raw";
  else if (suffix == "pxn")
    return "Logitech Raw";
  else if (suffix == "r3d")
    return "Red Raw";
  else if (suffix == "raf")
    return "Fuji Raw";
  else if (suffix == "rw2")
    return "Panasonic Raw";
  else if (suffix == "rw1")
    return "Leica Raw";
  else if (suffix == "rwz")
    return "Rawzor Raw";
  else if (suffix == "x3f")
    return "Signa Raw";

  return "Image";
}

const QSet<QString> & FormatProber::audioSuffixes(void)
{
  static QSet<QString> suffixes;

  if (__builtin_expect(suffixes.isEmpty(), false))
  {
    suffixes += "aac";
    suffixes += "aif";
    suffixes += "dts";
    suffixes += "iff";
    suffixes += "mid";
    suffixes += "midi";
    suffixes += "mp2";
    suffixes += "mp3";
    suffixes += "mpa";
    suffixes += "oga";
    suffixes += "ogg";
    suffixes += "ra";
    suffixes += "ram";
    suffixes += "wav";
    suffixes += "wma";
  }

  return suffixes;
}

const QSet<QString> & FormatProber::videoSuffixes(void)
{
  static QSet<QString> suffixes;

  if (__builtin_expect(suffixes.isEmpty(), false))
  {
    suffixes += "3gp";
    suffixes += "3g2";
    suffixes += "asf";
    suffixes += "asx";
    suffixes += "avi";
    suffixes += "divx";
    suffixes += "flv";
    suffixes += "m2t";
    suffixes += "m2v";
    suffixes += "m4v";
    suffixes += "mkv";
    suffixes += "mov";
    suffixes += "mp4";
    suffixes += "mpe";
    suffixes += "mpeg";
    suffixes += "mpg";
    suffixes += "mpv";
    suffixes += "mpv2";
    suffixes += "ogv";
    suffixes += "ogx";
    suffixes += "ps";
    suffixes += "qt";
    suffixes += "rm";
    suffixes += "spx";
    suffixes += "swf";
    suffixes += "ts";
    suffixes += "vob";
    suffixes += "wmv";
  }

  return suffixes;
}

const QSet<QString> & FormatProber::imageSuffixes(void)
{
  static QSet<QString> suffixes;

  if (__builtin_expect(suffixes.isEmpty(), false))
  {
    suffixes += "ai";
    suffixes += "bmp";
    suffixes += "drw";
    suffixes += "dxf";
    suffixes += "gif";
    suffixes += "ico";
    suffixes += "indd";
    suffixes += "jpe";
    suffixes += "jpg";
    suffixes += "jpeg";
    suffixes += "mng";
    suffixes += "pct";
    suffixes += "png";
    suffixes += "psd";
    suffixes += "svg";
    suffixes += "tif";
    suffixes += "tiff";

    suffixes += rawImageSuffixes();
  }

  return suffixes;
}

const QSet<QString>  & FormatProber::rawImageSuffixes(void)
{
  static QSet<QString> suffixes;

  if (__builtin_expect(suffixes.isEmpty(), false))
  {
    suffixes += "3fr";
    suffixes += "arw";
    suffixes += "srf";
    suffixes += "sr2";
    suffixes += "bay";
    suffixes += "crw";
    suffixes += "cr2";
    suffixes += "cap";
    suffixes += "tif";
    suffixes += "iiq";
    suffixes += "eip";
    suffixes += "dcs";
    suffixes += "dcr";
    suffixes += "drf";
    suffixes += "k25";
    suffixes += "kdc";
    suffixes += "dng";
    suffixes += "erf";
    suffixes += "fff";
    suffixes += "mos";
    suffixes += "mrw";
    suffixes += "nef";
    suffixes += "nrw";
    suffixes += "orf";
    suffixes += "ptx";
    suffixes += "pef";
    suffixes += "pxn";
    suffixes += "r3d";
    suffixes += "raf";
    suffixes += "raw";
    suffixes += "rw1";
    suffixes += "rw2";
    suffixes += "rwz";
    suffixes += "x3f";
  }

  return suffixes;
}


} } // End of namespaces
