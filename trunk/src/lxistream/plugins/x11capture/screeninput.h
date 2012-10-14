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

#include <xcb/xcb.h>
#include <QtCore>
#include <LXiStreamDevice>

namespace LXiStreamDevice {
namespace X11Capture {

class ScreenInput : public SInterfaces::VideoInput
{
Q_OBJECT
public:
  class Memory : public SBuffer::Memory
  {
  public:
                                Memory(::xcb_get_image_reply_t *img);
    virtual                     ~Memory();

  private:
    ::xcb_get_image_reply_t * const img;
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
  static QMap<QString, QRect>   screens;

  ::xcb_connection_t          * connnection;
  ::xcb_window_t                window;
  QRect                         screenRect;

  STimer                        timer;
  STime                         lastImage;
  SVideoFormat                  videoFormat;
};

} } // End of namespaces

#endif
