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

#ifndef LXISTREAMDEVICE_SINTERFACES_H
#define LXISTREAMDEVICE_SINTERFACES_H

#include <QtCore>
#include <LXiCore>
#include <LXiStream>
#include "export.h"

namespace LXiStreamDevice {
namespace SInterfaces {

/*! The AudioInput interface can be used to provide audio input devices.
 */
class LXISTREAMDEVICE_PUBLIC AudioInput : public QObject
{
Q_OBJECT
S_FACTORIZABLE(AudioInput)
protected:
  inline explicit               AudioInput(QObject *parent) : QObject(parent) { }

public:
  virtual void                  setFormat(const SAudioFormat &) = 0;
  virtual SAudioFormat          format(void) = 0;

  virtual bool                  start(void) = 0;
  virtual void                  stop(void) = 0;
  virtual void                  process(void) = 0;

signals:
  void                          produce(const SAudioBuffer &);
};

/*! The AudioOutput interface can be used to provide audio output devices.
 */
class LXISTREAMDEVICE_PUBLIC AudioOutput : public QObject
{
Q_OBJECT
S_FACTORIZABLE(AudioOutput)
protected:
  inline explicit               AudioOutput(QObject *parent) : QObject(parent) { }

public:
  virtual bool                  start(void) = 0;
  virtual void                  stop(void) = 0;
  virtual STime                 latency(void) const = 0;

public slots:
  virtual void                  consume(const SAudioBuffer &) = 0;
};

/*! The VideoInput interface can be used to provide video input devices.
 */
class LXISTREAMDEVICE_PUBLIC VideoInput : public QObject
{
Q_OBJECT
S_FACTORIZABLE(VideoInput)
protected:
  inline explicit               VideoInput(QObject *parent) : QObject(parent) { }

public:
  virtual void                  setFormat(const SVideoFormat &) = 0;
  virtual SVideoFormat          format(void) = 0;
  virtual void                  setMaxBuffers(int) = 0;
  virtual int                   maxBuffers(void) const = 0;

  virtual bool                  start(void) = 0;
  virtual void                  stop(void) = 0;
  virtual void                  process(void) = 0;

signals:
  void                          produce(const SVideoBuffer &);
};

} } // End of namespaces

#endif
