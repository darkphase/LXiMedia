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

#ifndef LXICORE_SSTRINGPARSER_H
#define LXICORE_SSTRINGPARSER_H

#include <QtCore>
#include "export.h"

namespace LXiCore {

/*! This class provides a simple template engine and several basic string
    operations. The template engine will replace fields between curly brackets
    by variables, for example:
    \code
      SStringParser p;
      p.setField("VAR", "world");
      qDebug() << p.parse("hello {VAR}"); // -> "hello world"
    \endcode
 */
class LXICORE_PUBLIC SStringParser
{
private:
  struct LetterFrequency { const char * const lang; const wchar_t * const freq; };
  struct Iso639LangCode { const char * const code, * const name; };

public:
  explicit                      SStringParser(bool enableEscapeXml = true);
                                ~SStringParser();

                                SStringParser(const SStringParser &);
  SStringParser               & operator=(const SStringParser &);

  static void                   setStaticField(const char *, const char *);
  static void                   setStaticField(const char *, const QByteArray &);
  static void                   setStaticField(const char *, const QString &);
  static void                   setStaticField(const char *, const QUrl &);
  static void                   clearStaticField(const char *);

  void                          clear(void);

  void                          setField(const char *, const char *);
  void                          setField(const char *, const QByteArray &);
  void                          setField(const char *, const QString &);
  void                          setField(const char *, const QUrl &);
  void                          appendField(const char *, const char *);
  void                          appendField(const char *, const QByteArray &);
  void                          appendField(const char *, const QString &);
  void                          appendField(const char *, const QUrl &);
  void                          clearField(const char *);

  void                          copyField(const char *, const char *);
  QByteArray                    field(const char *) const;
  QByteArray                    parse(const QByteArray &) const;

public:
  /*! Returns true if the string is a valid UTF8 string, otherwise returns false.
   */
  static bool                   isUtf8(const QByteArray &);

  /*! Ensures a string is properly escaped for use in XML.
   */
  static QByteArray             escapeXml(const QByteArray &);

  /*! Ensures a string is properly escaped for use in XML.
   */
  static QByteArray             escapeXml(const QString &);

  /*! Returns a string with only printable characters; all control characters are
      removed except for the tab (\t), which is replaced by a space. For example
      "This\tis a test\n" becomes "This is a test".
   */
  static QString                removeControl(const QString &);

  /*! Overload provided for convenience.
   */
  static QStringList            removeControl(const QStringList &);

  /*! Returns the provided string as a basic latin string, all non basic latin
      characters are replaced by their basic latin equivalent or ignored. For
      example "dæmon" becomes "daemon" and "heiß" becomes "heiss".
   */
  static QString                toBasicLatin(const QString &);

  /*! Overload provided for convenience.
   */
  static QString                toBasicLatin(QChar);

  /*! Overload provided for convenience.
   */
  static QStringList            toBasicLatin(const QStringList &);

  /*! Returns the provided string with only basic latin letters and numbers,
      all other characters are replaced by their basic latin equivalent or
      spaces and then QString::simplified() is applied on that string. For example
      "# This ... is a test!" becomes "This is a test".

      \sa toBasicLatin()
   */
  static QString                toCleanName(const QString &);

  /*! Overload provided for convenience.
   */
  static QStringList            toCleanName(const QStringList &);

  /*! Returns the provided string with only basic latin letters and numbers
      remaining in upper case, all other characters are replaced by their basic
      latin equivalent. For example: "This is ... a test!" becomes "THISISATEST".

      \sa toBasicLatin()
   */
  static QString                toRawName(const QString &);

  /*! Overload provided for convenience.
   */
  static QStringList            toRawName(const QStringList &);

  /*! Returns the provided string with only basic latin letters, numbers and path
      separators remaining in upper case, all other characters are replaced by
      their basic latin equivalent. For example: "/home/test-user/??/test/" becomes
      "HOME/TESTUSER/TEST".

      \sa toRawName()
   */
  static QString                toRawPath(const QString &);

  /*! Compares two strings using the "Aplhanum Algorithm".
      \returns <0 if a<b, >0 if a>b, or 0 if a==b.
   */
  static int                    alphaNumCompare(const QString &a, const QString &b);

  /*! Looks for the biggest possible matching string in a and b.
   */
  static QString                findMatch(const QString &, const QString &);

  /*! Looks for the biggest possible matching string of b in a, returns a
      real value from 0.0 to 1.0 indicating the location and amount of b matched.

      \note This operation is case sensitive, it may be useful to first pass the
            two arguments through toRawName() or toCleanName().
   */
  static qreal                  computeMatch(const QString &, const QString &);

  /*! Looks for the biggest possible matching strings of b in a, returns a
      real value from 0.0 to 1.0 indicating the location and amount of b matched.

      \note This operation is case sensitive, it may be useful to first pass the
            two arguments through toRawName() or toCleanName().
   */
  static qreal                  computeMatch(const QString &, const QStringList &);

  /*! Returns computeMatch(a, b) * computeMatch(b, a).

      \note This operation is case sensitive, it may be useful to first pass the
            two arguments through toRawName() or toCleanName().
      \sa computeMatch()
   */
  static qreal                  computeBidirMatch(const QString &, const QString &);

  static unsigned               numWords(const QString &);

  /*! Returns the ISO 639-2 language code of the text, or an empty string if it
      can't be determined. The language is determined by analyzing the letter
      frequency.
   */
  static const char           * languageOf(const QByteArray &);

  /*! Returns the ISO 639-2 language code of the text, or an empty string if it
      can't be determined. The language is determined by analyzing the letter
      frequency.
   */
  static const char           * languageOf(const QString &);

  /*! Returns the translated language name for the ISO 639-1 or ISO 639-2 language
      code.
   */
  static QString                iso639Language(const QString &);

  /*! Returns the translated language name for the ISO 639-1 or ISO 639-2 language
      code.
   */
  static QString                iso639Language(const char *);

  /*! Returns the default codepage for the ISO 639-1 or ISO 639-2 language code.
   */
  static const char           * codepageFor(const QString &);

  /*! Returns the default codepage for the ISO 639-1 or ISO 639-2 language code.
   */
  static const char           * codepageFor(const char *);

  /*! Returns a map with a translated language name for each of the  ISO 639-2
      language codes.
   */
  static QMap<QByteArray, QString> allIso639Languages(void);

private:
  static const LetterFrequency * letterFrequencies(void);
  static const Iso639LangCode * iso639_1Codes(void);
  static const Iso639LangCode * iso639_2Codes(void);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespaces

#endif
