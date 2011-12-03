#ifndef LXSTREAM_SVIDEOVIEW_H
#define LXSTREAM_SVIDEOVIEW_H

#include <QtCore>
#include <QtGui>
#include <LXiCore>
#include <LXiStream>
#include "export.h"

namespace LXiStreamGui {

class SVideoViewSink;

/*! \brief The SVideoView class provides a widget for rendering video.

    This class renders the video stream provided to the node returned by node()
    to the widget. The video rendering may be accellerated depending on the
    underlying hardware.
 */
class LXISTREAMGUI_PUBLIC SVideoView : public QWidget
{
Q_OBJECT
friend class SVideoViewSink;
public:
                                SVideoView(QWidget *);
  virtual                       ~SVideoView();

  SVideoViewSink              * createSink(SGraph *);

  bool                          slow(void) const;
  void                          setSlow(bool s);

  void                          addClone(SVideoView *);
  void                          removeClone(SVideoView *);

protected:
  bool                          start(STimer *);
  void                          stop(void);
  void                          input(const SVideoBuffer &);

  virtual void                  paintEvent(QPaintEvent *);
  virtual void                  timerEvent(QTimerEvent *);

  void                          blitVideo(void);

private:
  void                          setSource(SVideoView *);

private:
  struct Private;
  Private               * const p;
};

class LXISTREAMGUI_PUBLIC SVideoViewSink : public SInterfaces::SinkNode
{
Q_OBJECT
public:
                                SVideoViewSink(SGraph *, SVideoView *);
  virtual                       ~SVideoViewSink();

public: // From SInterfaces::SinkNode
  virtual bool                  start(STimer *);
  virtual void                  stop(void);

public slots:
  void                          input(const SVideoBuffer &);

private:
  SVideoView            * const parent;
};

} // End of namespace

#endif
