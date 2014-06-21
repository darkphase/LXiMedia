/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#include <QtCore>
#include <LXiStreamDevice>
#include <windows.h>

namespace LXiStreamDevice {
namespace GdiCapture {

class ScreenInput : public SInterfaces::VideoInput
{
Q_OBJECT
public:
  struct Image
  {
    ::HDC                       dc;
    ::BITMAPINFO                bitmapInfo;
    ::HBITMAP                   bitmap;
    void                      * buffer;
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
  static void                   deleteImage(Image *);

private:
  static QMap<QString, QRect>   screens;
  static const int              numImages;
  static QMutex                 imagesMutex;
  static QAtomicInt             inputIdCounter;
  static QMap< int, QStack<Image *> > images;

  const quint32                 inputId;
  ::HDC                         screenDc;
  QRect                         screenRect;

  STimer                        timer;
  STime                         lastImage;
  SVideoFormat                  videoFormat;
};

} } // End of namespaces

#endif
