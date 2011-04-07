#ifndef LXSTREAM_SVIDEOVIEW_H
#define LXSTREAM_SVIDEOVIEW_H

#include <QtCore>
#include <QtGui>
#include <LXiCore>
#include <LXiStream>

namespace LXiStreamGui {

/*! \brief The SVideoView class provides a widget for rendering video.

    This class renders the video stream provided to the node returned by node()
    to the widget. The video rendering may be accellerated depending on the
    underlying hardware.
 */
class S_DSO_PUBLIC SVideoView : public QWidget,
                                public SGraph::SinkNode
{
Q_OBJECT
public:
                                SVideoView(QWidget *);
  virtual                       ~SVideoView();

  bool                          slow(void) const;
  void                          setSlow(bool s);

  void                          addClone(SVideoView *);
  void                          removeClone(SVideoView *);

public: // From SInterfaces::SinkNode
  virtual bool                  start(STimer *);
  virtual void                  stop(void);

public slots:
  void                          input(const SVideoBuffer &);

protected:
  virtual void                  paintEvent(QPaintEvent *);
  virtual void                  timerEvent(QTimerEvent *);

  void                          blitVideo(void);

private:
  void                          setSource(SVideoView *);

private:
  struct Private;
  Private               * const p;
};

} // End of namespace

#endif
