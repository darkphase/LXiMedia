/***************************************************************************
 *   Copyright (C) 2007 by A.J. Admiraal                                   *
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
  struct Iso639LangCode { const char * const code, * const name; };

public:
  explicit                      SStringParser(bool enableEscapeXml = true);
                                ~SStringParser();

                                SStringParser(const SStringParser &);
  SStringParser               & operator=(const SStringParser &);

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
  static bool                   isUtf8(const QByteArray &);

  static QByteArray             escapeXml(const QByteArray &);
  static QByteArray             escapeXml(const QString &);

  static QString                removeControl(const QString &);
  static QStringList            removeControl(const QStringList &);
  static QString                toBasicLatin(const QString &);
  static QString                toBasicLatin(QChar);
  static QStringList            toBasicLatin(const QStringList &);
  static QString                toCleanName(const QString &);
  static QStringList            toCleanName(const QStringList &);
  static QString                toRawName(const QString &);
  static QStringList            toRawName(const QStringList &);
  static QString                toRawPath(const QString &);

  static QString                findMatch(const QString &, const QString &);
  static qreal                  computeMatch(const QString &, const QString &);
  static qreal                  computeMatch(const QString &, const QStringList &);
  static qreal                  computeBidirMatch(const QString &, const QString &);

  static unsigned               numWords(const QString &);

  static const char           * languageOf(const QString &);

  /*! Returns the translated language name for the ISO 639-1 or ISO 639-2 language
      code.
   */
  static QString                iso639Language(const QString &);

  /*! Returns the translated language name for the ISO 639-1 or ISO 639-2 language
      code.
   */
  static QString                iso639Language(const char *);

  /*! Returns a map with a translated language name for each of the  ISO 639-2
      language codes.
   */
  static QMap<QByteArray, QString> allIso639Languages(void);

private:
  static const Iso639LangCode * iso639_1Codes(void);
  static const Iso639LangCode * iso639_2Codes(void);

private:
  struct Data;
  Data                  * const d;
};

} // End of namespaces

#endif
