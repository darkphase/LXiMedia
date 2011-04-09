#ifndef LXSTREAM_SDISPLAY_H
#define LXSTREAM_SDISPLAY_H

#include <QtCore>
#include <LXiCore>
#include "export.h"

namespace LXiStreamGui {

class LXISTREAMGUI_PUBLIC SDisplay : public QObject
{
Q_OBJECT
public:
  struct Mode
  {
    QSize                       size;
    qreal                       rate;
  };

public:
                                SDisplay(void);
  virtual                       ~SDisplay();

  void                          blockScreenSaver(bool);

  QList<Mode>                   allModes(void) const;
  Mode                          mode(void) const;
  bool                          setMode(const Mode &);

  bool                          setSize(const QSize &);
  inline bool                   setSize(int w, int h) { return setSize(QSize(w, h)); }

  QList<qreal>                  allRates(void) const;
  bool                          setRate(qreal);

protected:
  virtual void                  timerEvent(QTimerEvent *);

private:
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
