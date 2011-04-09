#ifndef LXSTREAM_SIMAGE_H
#define LXSTREAM_SIMAGE_H

#include <QtCore>
#include <LXiCore>
#include <liblxistream/sinterval.h>
#include <liblxistream/ssize.h>
#include <liblxistream/svideobuffer.h>
#include "export.h"

namespace LXiStreamGui {

class LXISTREAMGUI_PUBLIC SImage : public QImage
{
public:
  inline                        SImage(void) : QImage()                         { }
  inline                        SImage(const QSize &size, Format format) : QImage(size, format) { }
  inline                        SImage(int width, int height, Format format) : QImage(width, height, format) { }
  explicit                      SImage(const QString &fileName, const char * = NULL);
#ifndef QT_NO_CAST_FROM_ASCII
  explicit                      SImage(const char *fileName, const char * = NULL);
#endif
  inline                        SImage(const QImage &img) : QImage(img)         { }
  inline                        SImage(const SImage &img) : QImage(img)         { }
                                SImage(const LXiStream::SVideoBuffer &, bool = false);

  LXiStream::SVideoBuffer       toVideoBuffer(float aspectRatio = 1.0f, LXiStream::SInterval frameRate = LXiStream::SInterval()) const;
  inline                        operator LXiStream::SVideoBuffer() const        { return toVideoBuffer(); }

  static SImage                 fromData(const uchar *, int, const char * = NULL);
  static SImage                 fromData(const QByteArray &, const char * = NULL);
};

} // End of namespace

#endif
