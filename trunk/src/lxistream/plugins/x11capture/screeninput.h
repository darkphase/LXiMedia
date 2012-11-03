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

#ifndef SCREENINPUT_H
#define SCREENINPUT_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/shm.h>
#include <QtCore>
#include <LXiStreamDevice>

namespace LXiStreamDevice {
namespace X11Capture {

class ScreenInput : public SInterfaces::VideoInput
{
Q_OBJECT
public:
  struct Image
  {
    ::xcb_image_t               img;
    ::xcb_shm_segment_info_t    shminfo;
  };

  class Memory : public SBuffer::Memory
  {
  public:
                                Memory(int, Image *);
    virtual                     ~Memory();

  private:
    const int                   inputId;
    Image               * const image;
  };

public:
  static QList<SFactory::Scheme> listDevices(void);

public:
                                ScreenInput(const QString &, QObject *);
  virtual                       ~ScreenInput();

public: // From SInterfaces::VideoInput
  virtual void                  setFormat(const SVideoFormat &);
  virtual SVideoFormat          format(void);
  virtual void                  setMaxBuffers(int);
  virtual int                   maxBuffers(void) const;

  virtual bool                  start(void);
  virtual void                  stop(void);
  virtual bool                  process(void);

private:
  static void                   deleteImage(::xcb_connection_t *, Image *);

private:
  static QMap<QString, QRect>   screens;
  static const int              numImages;
  static QMutex                 imagesMutex;
  static QAtomicInt             inputIdCounter;
  static QMap< int, QStack<Image *> > images;

  const quint32                 inputId;
  ::xcb_connection_t          * connection;
  ::xcb_window_t                window;
  QRect                         screenRect;

  STimer                        timer;
  STime                         lastImage;
  SVideoFormat                  videoFormat;
};

} } // End of namespaces

#endif
