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

#ifndef MEDIAPLAYER_MODULE_H
#define MEDIAPLAYER_MODULE_H

#include <QtCore>
#include <LXiMediaCenter>

#ifndef QT_PLUGIN
#define QT_PLUGIN
#endif

namespace LXiMediaCenter {
namespace MediaPlayerBackend {

class MediaDatabase;

class Module : public SModule
{
Q_OBJECT
public:
  virtual bool                  registerClasses(void);
  virtual void                  unload(void);
  virtual QByteArray            about(void);
  virtual QByteArray            licenses(void);

public:
  static const char             pluginName[];

private:
  static const char             moviesName[],       moviesIcon[];
  static const char             tvShowsName[],      tvShowsIcon[];
  static const char             clipsName[],        clipsIcon[];
  static const char             homeVideosName[],   homeVideosIcon[];
  static const char             photosName[],       photosIcon[];
  static const char             musicName[],        musicIcon[];
};

} } // End of namespaces

#endif
