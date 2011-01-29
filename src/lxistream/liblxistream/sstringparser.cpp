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

#include "sstringparser.h"

namespace LXiStream {

/*! Returns true if the string is a valid UTF8 string, otherwise returns false.
 */
bool SStringParser::isUtf8(const QByteArray &text)
{
  for (int i=0; i<text.count()-4; )
  {
    const unsigned char * const b = reinterpret_cast<const unsigned char *>(text.data() + i);

    if (b[0] <= 0x7E) // ASCII
    {
      i += 1;
      continue;
    }
    else if ((0xC2 <= b[0] && b[0] <= 0xDF) && (0x80 <= b[1] && b[1] <= 0xBF)) // Non-overlong 2-byte
    {
      i += 2;
      continue;
    }
    else if ((b[0] == 0xE0 &&
              (0xA0 <= b[1] && b[1] <= 0xBF) &&
              (0x80 <= b[2] && b[2] <= 0xBF)) || // excluding overlongs
             (((0xE1 <= b[0] && b[0] <= 0xEC) || b[0] == 0xEE || b[0] == 0xEF) &&
              (0x80 <= b[1] && b[1] <= 0xBF) &&
              (0x80 <= b[2] && b[2] <= 0xBF)) || // straight 3-byte
             (b[0] == 0xED &&
              (0x80 <= b[1] && b[1] <= 0x9F) &&
              (0x80 <= b[2] && b[2] <= 0xBF)) // excluding surrogates
            )
    {
      i += 3;
      continue;
    }
    else if ((b[0] == 0xF0 &&
              (0x90 <= b[1] && b[1] <= 0xBF) &&
              (0x80 <= b[2] && b[2] <= 0xBF) &&
              (0x80 <= b[3] && b[3] <= 0xBF)) || // planes 1-3
             ((0xF1 <= b[0] && b[0] <= 0xF3) &&
              (0x80 <= b[1] && b[1] <= 0xBF) &&
              (0x80 <= b[2] && b[2] <= 0xBF) &&
              (0x80 <= b[3] && b[3] <= 0xBF)) || // planes 4-15
             (b[0] == 0xF4 &&
              (0x80 <= b[1] && b[1] <= 0x8F) &&
              (0x80 <= b[2] && b[2] <= 0xBF) &&
              (0x80 <= b[3] && b[3] <= 0xBF)) // plane 16
            )
    {
      i += 4;
      continue;
    }

    return false;
  }

  return true;
}

/*! Returns a string with only printable characters; all control characters are
    removed except for the tab (\t), which is replaced by a space. For example
    "This\tis a test\n" becomes "This is a test".
 */
QString SStringParser::removeControl(const QString &str)
{
  QString input(str);
  QString printable = "";

  input.replace('\t', ' ');

  for (QString::ConstIterator i=input.begin(); i!=input.end(); i++)
  if (i->isPrint())
    printable += *i;

  return printable;
}

/*! Overload provided for convenience.
 */
QStringList SStringParser::removeControl(const QStringList &list)
{
  QStringList result;
  foreach (const QString &s, list)
    result += removeControl(s);

  return result;
}

/*! Returns the provided string as a basic latin string, all non basic latin
    characters are replaced by their basic latin equivalent or ignored. For
    example "dæmon" becomes "daemon" and "heiß" becomes "heiss".
 */
QString SStringParser::toBasicLatin(const QString &s)
{
  QString result = "";

  for (QString::ConstIterator i=s.begin(); i!=s.end(); i++)
    result += toBasicLatin(*i);

  return result;
}

/*! Overload provided for convenience.
 */
QString SStringParser::toBasicLatin(QChar c)
{
  const char ascii = c.toAscii();
  if ((ascii > 0) && (ascii < 127))
    return QString(ascii);

  // Unicode characters 00C0 to 0x01AF
  static const char * const uni00C0[] =
    // 0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
    { "A",  "A",  "A",  "A",  "A",  "A", "AE",  "C",  "E",  "E",  "E",  "E",  "I",  "I",  "I",  "I",
      "D",  "N",  "O",  "O",  "O",  "O",  "O",  "x",  "O",  "U",  "U",  "U",  "U",  "Y", "TH", "ss",
      "a",  "a",  "a",  "a",  "a",  "a", "ae",  "c",  "e",  "e",  "e",  "e",  "i",  "i",  "i",  "i",
      "d",  "n",  "o",  "o",  "o",  "o",  "o",  "/",  "o",  "u",  "u",  "u",  "u",  "y", "ty",  "y",
      "A",  "a",  "A",  "a",  "A",  "a",  "C",  "c",  "C",  "c",  "C",  "c",  "C",  "c",  "D",  "d",
      "D",  "d",  "E",  "e",  "E",  "e",  "E",  "e",  "E",  "e",  "E",  "e",  "G",  "g",  "G",  "g",
      "G",  "g",  "G",  "g",  "H",  "h",  "H",  "h",  "I",  "i",  "I",  "i",  "I",  "i",  "I",  "i",
      "I",  "i", "IJ", "ij",  "J",  "j",  "K",  "k",  "K",  "L",  "l",  "L",  "l",  "L",  "l",  "L",
      "l",  "L",  "l",  "N",  "n",  "N",  "n",  "N",  "n",  "n",  "N",  "n",  "O",  "o",  "O",  "o",
      "O",  "o", "OE", "oe",  "R",  "r",  "R",  "r",  "R",  "r",  "S",  "s",  "S",  "s",  "S",  "s",
      "S",  "s",  "T",  "t",  "T",  "t",  "T",  "t",  "U",  "u",  "U",  "u",  "U",  "u",  "U",  "u",
      "U",  "u",  "U",  "u",  "W",  "w",  "Y",  "y",  "Y",  "Z",  "z",  "Z",  "z",  "Z",  "z",  "s" };

  if ((c.unicode() >= 0x00C0) && (c.unicode() < 0x01B0))
    return QString(uni00C0[c.unicode() - 0x00C0]);

  return QString("");
}

/*! Overload provided for convenience.
 */
QStringList SStringParser::toBasicLatin(const QStringList &list)
{
  QStringList result;
  foreach (const QString &s, list)
    result += toBasicLatin(s);

  return result;
}

/*! Returns the provided string with only basic latin letters and numbers,
    all other characters are replaced by their basic latin equivalent or
    spaces and then QString::simplified() is applied on that string. For example
    "# This ... is a test!" becomes "This is a test".

    \sa toBasicLatin()
 */
QString SStringParser::toCleanName(const QString &name)
{
  QString clean = "";
  clean.reserve(name.length());

  for (QString::ConstIterator i=name.begin(); i!=name.end(); i++)
  if (i->isLetterOrNumber())
    clean += toBasicLatin(*i);
  else if (!clean.isEmpty() && (clean[clean.length()-1] != ' '))
    clean += ' ';

  return clean.trimmed();
}

/*! Overload provided for convenience.
 */
QStringList SStringParser::toCleanName(const QStringList &list)
{
  QStringList result;
  foreach (const QString &s, list)
    result += toCleanName(s);

  return result;
}

/*! Returns the provided string with only basic latin letters and numbers
    remaining in upper case, all other characters are replaced by their basic
    latin equivalent. For example: "This is ... a test!" becomes "THISISATEST".

    \sa toBasicLatin()
 */
QString SStringParser::toRawName(const QString &name)
{
  QString raw;
  raw.reserve(name.length());

  for (QString::ConstIterator i=name.begin(); i!=name.end(); i++)
  if (i->isLetterOrNumber())
    raw += toBasicLatin(i->toUpper());
    
  return raw;
}

/*! Overload provided for convenience.
 */
QStringList SStringParser::toRawName(const QStringList &list)
{
  QStringList result;
  foreach (const QString &s, list)
    result += toRawName(s);

  return result;
}

/*! Returns the provided string with only basic latin letters, numbers and path
    separators remaining in upper case, all other characters are replaced by
    their basic latin equivalent. For example: "/home/test-user/??/test/" becomes
    "HOME/TESTUSER/TEST".

    \sa toRawName()
 */
QString SStringParser::toRawPath(const QString &path)
{
  QStringList raw = toRawName(path.split('/', QString::SkipEmptyParts));
  for (QStringList::Iterator i=raw.begin(); i!=raw.end(); )
  if (i->isEmpty())
    i = raw.erase(i);
  else
    i++;

  return raw.join("/");
}

/*! Looks for the biggest possible matching string in a and b.
 */
QString SStringParser::findMatch(const QString &a, const QString &b)
{
  QString match = QString::null;

  for (int i=(-b.length() + 1); i<a.length(); i++)
  {
    const int jbegin = qMax(-i, 0), jend = qMin((a.length() - i), b.length());
    QString m = "";

    for (int j=jbegin; j<jend; j++)
    if (a[i+j] == b[j])
      m += b[j];
    else
    {
      if (m.length() > match.length())
        match = m;

      m = "";
    }

    if (m.length() > match.length())
      match = m;
  }

  return match;
}

/*! Looks for the biggest possible matching string of b in a, returns a
    real value from 0.0 to 1.0 indicating the location and amount of b matched.

    \note This operation is case sensitive, it may be useful to first pass the
          two arguments through toRawName() or toCleanName().
 */
qreal SStringParser::computeMatch(const QString &a, const QString &b)
{
  if (!a.isEmpty() && !b.isEmpty())
  {
    const QString match = findMatch(a, b);

    return (qreal(match.length()) / qreal(b.length())) *
           (1.0 - (qreal(a.indexOf(match)) / qreal(a.length())));
  }
  else
    return 0.0;
}

/*! Looks for the biggest possible matching strings of b in a, returns a
    real value from 0.0 to 1.0 indicating the location and amount of b matched.

    \note This operation is case sensitive, it may be useful to first pass the
          two arguments through toRawName() or toCleanName().
 */
qreal SStringParser::computeMatch(const QString &a, const QStringList &b)
{
  if (!a.isEmpty() && !b.isEmpty())
  {
    qreal result = 1.0;
    foreach (const QString &s, b)
      result *= computeMatch(a, s);

    return result;
  }
  else
    return 0.0;
}

/*! Returns computeMatch(a, b) * computeMatch(b, a).

    \note This operation is case sensitive, it may be useful to first pass the
          two arguments through toRawName() or toCleanName().
    \sa computeMatch()
 */
qreal SStringParser::computeBidirMatch(const QString &a, const QString &b)
{
  return computeMatch(a, b) * computeMatch(b, a);
}

unsigned SStringParser::numWords(const QString &txt)
{
  return toCleanName(txt).count(' ') + 1;
}

/*! Returns the ISO 639-2 language code of the text, or an empty string if it
    can't be determined. The language is determined by analyzing the letter
    frequency.
 */
const char * SStringParser::languageOf(const QString &text)
{
  // Count the number of characters.
  QMap<QChar, int> count;
  foreach (const QChar c, text.toLower())
  if (c.isLetter())
  {
    QMap<QChar, int>::Iterator i = count.find(c);
    if (i != count.end())
      (*i)++;
    else
      count.insert(c, 1);
  }

  // Sort the characters by frequency.
  QMultiMap<int, QChar> freq;
  for (QMap<QChar, int>::ConstIterator i = count.begin(); i != count.end(); i++)
    freq.insert(-(i.value()), i.key());

  // And determine the language.
  static const struct { const char * const l; const wchar_t * const f; } frequencies[] =
    { { "cze", L"oeantivlsrdkupímcházyjbřêéĉžýŝũgfúňwďóxťq" },
      { "dan", L"enadtrslgiohmkvufbpøjycéxqwèzüàóêç" },
      { "dut", L"enatirodslghvkmubpwjczfxyëéóq" },
      { "eng", L"etaoinsrhldcumfpgwybvkxjqz" },
      { "epo", L"aieonlsrtkjudmpvgfbcĝĉŭzŝhĵĥwyxq" },
      { "fin", L"enatrsildokgmvfuphäcböjyxzwq" },
      { "fre", L"esaitnrulodcmpévqfbghjàxèyêzçôùâûîœwkïëüæñ" },
      { "ger", L"enisratdhulcgmobwfkzvüpäßjöyqx" },
      { "grc", L"αοιετσνηυρπκμλωδγχθφβξζψ" },
      { "hun", L"eatlnskomzrigáéydbvhjfupöócíúüxwq" },
      { "ice", L"anriestudhlgmkfhvoáthídjóbyæúöpéýcxwzq" },
      { "ita", L"eaionlrtscdupmvghfbqzòàùìéèóykwxjô" },
      { "nor", L"erntsilakodgmvfupbhøjyæcwzxq" },
      { "pol", L"iaeoznscrwyłdkmtpujlgębąhżśóćńfźvqx" },
      { "por", L"aeosrinmdutlcphvqgfbãzçjáéxóõêôàyíèú" },
      { "rus", L"oeaинтсвлркдмпуëягбзчйхжшюцщeф" },
      { "slo", L"aoesnitrvlkdmcupzyhjgfbqwx" },
      { "spa", L"eaosrnidlctumpbgyívqóhfzjéáñxúüwk" },
      { "swe", L"eantrslidomgkvähfupåöbcjyxwzéq" },
      { "tur", L"aeinrlıdkmuytsboüşzgçhğvcöpfjwxq" },
      { NULL, NULL } };

  QMap<int, float> score;
  for (int i=0; frequencies[i].l; i++)
  {
    int n = 1;
    for (QMultiMap<int, QChar>::ConstIterator j = freq.begin(); j != freq.end(); j++, n++)
    {
      float d = 25.0f / float(n);
      for (int k=0; frequencies[i].f[k]; k++)
      if (QChar(frequencies[i].f[k]) == *j)
      {
        d = float(qAbs(k - n)) / float(n);
        break;
      }

      QMap<int, float>::Iterator k = score.find(i);
      if (k != score.end())
        (*k) += d;
      else
        score.insert(i, d);
    }
  }

  int best = -1;
  float bestValue = 10000000.0f;
  for (QMap<int, float>::ConstIterator i = score.begin(); i != score.end(); i++)
  if (i.value() < bestValue)
  {
    best = i.key();
    bestValue = i.value();
  }

  if (best >= 0)
    return frequencies[best].l;
  else
    return "";
}

} // End of namespaces
