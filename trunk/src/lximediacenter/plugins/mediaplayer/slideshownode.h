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

#ifndef SLIDESHOWSOURCE_H
#define SLIDESHOWSOURCE_H

#include <QtCore>
#include <LXiStream>

namespace LXiMediaCenter {

class SlideShowNode : public QObject,
                      public SInterfaces::SourceNode
{
Q_OBJECT
public:
                                SlideShowNode(SGraph *parent, const QStringList &pictures);
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
  void                          loadImage(const int &);
  void                          computeVideoBuffer(void);
  void                          sendFlush(void);
  SVideoBuffer                  blackBuffer(void) const;

public:
  static const int              frameRate = 25;
  static const int              slideFrameCount = 190;

private:
  const QStringList             pictures;

  SSize                         outSize;
  SAudioBuffer                  audioBuffer;
  SVideoBuffer                  videoBuffer;
  STime                         time;
  int                           currentPicture;
  SVideoBuffer                  current;
  STime                         nextTime;
  SVideoBuffer                  next;
  int                           fade;

  QFuture<SVideoBuffer>         loadFuture;
};

} // End of namespace

#endif
