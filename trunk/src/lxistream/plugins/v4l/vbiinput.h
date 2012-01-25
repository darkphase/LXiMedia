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

#ifndef V4LBACKEND_VBIINPUT_H
#define V4LBACKEND_VBIINPUT_H

#include <QtCore>
#include <LXiStream>

namespace LXiStream {
namespace V4lBackend {


class VBIInput : public SNode
{
Q_OBJECT
Q_PROPERTY(quint32 bitErrorRate READ bitErrorRate)
Q_PROPERTY(qreal signalQuality READ signalQuality)
private:
  struct Page
  {
    int                         pgno, subno;	// the wanted page number
    int                         lang;		// language code
    int                         flags;		// misc flags (see PG_xxx below)
    int                         errors;		// number of single bit errors in page
    quint32                     lines;		// 1 bit for each line received
    char                        data[25][40];	// page contents
    int                         flof;		// page has FastText links
    struct{int pgno;int subno;} link[6];	// FastText links (FLOF)
  } __attribute__((packed));

  struct Event
  {
    int                         type;
    void                      * resource;
    int                         i1, i2, i3, i4;
    void                      * p1;
  } __attribute__((packed));

public:
                                VBIInput(const QString &, QObject *);
  virtual                       ~VBIInput();

  quint32                       bitErrorRate(void) const;
  qreal                         signalQuality(void) const;

public: // From SNode
  virtual bool                  prepare(const SCodecList &);
  virtual bool                  unprepare(void);
  virtual Result                processBuffer(const SBuffer &, SBufferList &);

protected:
  void                          timerEvent(QTimerEvent *);

private:
  static void                   vbiEvent(VBIInput *, Event *);
  
private:
  QString                       device;
  volatile bool                 running;
  volatile qreal                quality;
  volatile quint32              bitErrors;
  volatile quint32              bitErrorCounter;
  int                           bitErrorTimer;
  void                        * vbi;
  STimer                        timer;

  SBufferList                 * producedBuffers;
};


} } // End of namespaces

#endif
