#ifndef LXSTREAMGUI_SIMAGE_H
#define LXSTREAMGUI_SIMAGE_H

#include <QtCore>
#include <LXiCore>
#include <LXiStream>
#include "export.h"

namespace LXiStreamGui {

class LXISTREAMGUI_PUBLIC SImage : public QImage
{
public:
  inline                        SImage(void) : QImage(), aspectRatio(1.0f)      { }
  inline                        SImage(const QSize &size, Format format) : QImage(size, format), aspectRatio(1.0f) { }
  inline                        SImage(const SSize &size, Format format) : QImage(size.size(), format), aspectRatio(size.aspectRatio()) { }
  inline                        SImage(int width, int height, Format format) : QImage(width, height, format), aspectRatio(1.0f) { }
  explicit                      SImage(const QString &fileName, const char * = NULL);
#ifndef QT_NO_CAST_FROM_ASCII
  explicit                      SImage(const char *fileName, const char * = NULL);
#endif
  inline                        SImage(const QImage &img) : QImage(img), aspectRatio(1.0f) { }
  inline                        SImage(const SImage &img) : QImage(img), aspectRatio(img.aspectRatio) { }
                                SImage(const LXiStream::SVideoBuffer &, bool = false);

  LXiStream::SVideoBuffer       toVideoBuffer(SInterval frameRate = SInterval()) const;
  inline                        operator LXiStream::SVideoBuffer() const        { return toVideoBuffer(); }

  inline SSize                  size(void) const                                { return SSize(QImage::size(), aspectRatio); }

  static SImage                 fromData(const uchar *, int, const char * = NULL);
  static SImage                 fromData(const QByteArray &, const char * = NULL);

private:
  float                         aspectRatio;
};

} // End of namespace

#endif
