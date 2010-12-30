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

#ifndef __FILEPROBER_H
#define __FILEPROBER_H

#include <QtCore>
#include <LXiStream>
#include <QtGui/QImage>

namespace LXiStream {
namespace FFTWBackend {


class FileProber : public SProber
{
Q_OBJECT
public:
                                FileProber(QObject *);
  virtual                       ~FileProber();

  virtual void                  fingerprint(FingerPrint &, const QString &) const;

private:
  static void                   fromImageFile(FingerPrint &fp, const QString &);
  static void                   fromAudioFile(FingerPrint &fp, const QString &);
};


} } // End of namespaces

#endif
