#ifndef LXSTREAM_SGLVIDEOVIEW_H
#define LXSTREAM_SGLVIDEOVIEW_H

#include <QtOpenGL>
#include <LXiStream>
#include <liblxistreamgl/sglsystem.h>
#include <liblxistreamgl/stexturebuffer.h>

namespace LXiStreamGl {


class SGlVideoView : public QGLWidget
{
Q_OBJECT
public:
                                SGlVideoView(QWidget *);
  virtual                       ~SGlVideoView();

public slots:
  void                          input(const SVideoBuffer &);

protected:
  virtual void                  initializeGL(void);
  virtual void                  resizeGL(int, int);
  virtual void                  paintGL(void);

private:
  STextureBuffer                lastBuffer;
  float                         overScan;
};


} // End of namespace

#endif
