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

#ifndef SLIDESHOWNODE_H
#define SLIDESHOWNODE_H

#include <QtCore>
#include <LXiStream>
#include "mediadatabase.h"

namespace LXiMediaCenter {

class SlideShowNode : public QObject,
                      public SGraph::SourceNode
{
Q_OBJECT
public:
                                SlideShowNode(SGraph *parent, const QList<MediaDatabase::File> &files, MediaDatabase *);
  virtual                       ~SlideShowNode();

  SSize                         size(void) const;
  void                          setSize(const SSize &size);

  STime                         duration(void) const;

public: // From SInterfaces::SourceNode
  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual void                  process(void);

signals:
  void                          output(const SAudioBuffer &);
  void                          output(const SVideoBuffer &);
  void                          finished(void);

private:
  void                          loadImage(const QString &);
  void                          computeVideoBuffer(const SVideoBuffer &, const SVideoBuffer &, int);
  void                          sendFlush(void);
  SVideoBuffer                  blackBuffer(void) const;

public:
  static const int              frameRate = 24;
  static const int              slideFrameCount = 180;

private:
  const QList<MediaDatabase::File> files;
  MediaDatabase          * const mediaDatabase;

  SScheduler::Dependency * const loadDependency;
  SScheduler::Dependency * const procDependency;
  SSize                         outSize;
  SAudioBuffer                  audioBuffer;
  STime                         time;
  int                           currentPicture;
  int                           currentFrame;
  SVideoBuffer                  lastBuffer, currentBuffer, nextBuffer;
  QSemaphore                    nextBufferReady;
};

} // End of namespace

#endif
