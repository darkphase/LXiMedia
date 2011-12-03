#ifndef LXSTREAM_SVUMETER_H
#define LXSTREAM_SVUMETER_H

#include <QtCore>
#include <QtGui>
#include <LXiCore>
#include <LXiStream>
#include "export.h"

namespace LXiStreamGui {

class LXISTREAMGUI_PUBLIC SVuMeter : public QFrame
{
Q_OBJECT
public:
                                SVuMeter(QWidget *);
  virtual                       ~SVuMeter();

  inline bool                   slow(void) const                                { return slowUpdate; }
  inline void                   setSlow(bool s)                                 { slowUpdate = s; }

  LXiStream::SAudioFormat       inputFormat(void) const;

public slots:
  void                          input(const SAudioBuffer &);
  
protected:
  virtual void                  paintEvent(QPaintEvent *);
  virtual void                  timerEvent(QTimerEvent *);

private:
  QVector<int>                  determinePos(const QVector<qreal> &, const QVector< QQueue<qreal> > &) const;

private:
  static const qreal            maxRms;

  mutable QMutex                mutex;
  bool                          slowUpdate;
  int                           updateTimer;
  volatile bool                 needsUpdate;
  QRect                         myRect;
  QVector<qreal>                rms;
  QVector<qreal>                lastRms;
  QVector< QQueue<qreal> >      peaks;
  QVector< QQueue<qreal> >      lastPeaks;
  LXiStream::SAudioFormat       format;
};

} // End of namespace

#endif
