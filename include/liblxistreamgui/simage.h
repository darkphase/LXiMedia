#ifndef LXSTREAMGUI_SIMAGE_H
#define LXSTREAMGUI_SIMAGE_H

#include <QtCore>
#include <QtGui>
#include <LXiCore>
#include <LXiStream>
#include "export.h"

namespace LXiStreamGui {

class LXISTREAMGUI_PUBLIC SImage : public QImage
{
public:
  inline                        SImage(void) : QImage()                         { d.aspectRatio = 1.0f; }
  inline                        SImage(const QSize &size, Format format) : QImage(size, format) { d.aspectRatio = 1.0f; d.originalSize = size; }
  inline                        SImage(const SSize &size, Format format) : QImage(size.size(), format) { d.aspectRatio = size.aspectRatio(); d.originalSize = size.size(); }
  inline                        SImage(int width, int height, Format format) : QImage(width, height, format) { d.aspectRatio = 1.0f; d.originalSize = QSize(width, height); }
  explicit                      SImage(const QUrl &filePath, const QSize &maxsize = QSize(), const char *format = NULL);
  inline                        SImage(const QImage &img) : QImage(img)         { d.aspectRatio = 1.0f; }
  inline                        SImage(const SImage &img) : QImage(img)         { d.aspectRatio = img.d.aspectRatio; d.originalSize = img.d.originalSize; }
                                SImage(const LXiStream::SVideoBuffer &, bool = false);

  LXiStream::SVideoBuffer       toVideoBuffer(SInterval frameRate = SInterval()) const;
  inline                        operator LXiStream::SVideoBuffer() const        { return toVideoBuffer(); }

  inline SSize                  size(void) const                                { return SSize(QImage::size(), d.aspectRatio); }
  inline QSize                  originalSize(void) const                        { return d.originalSize; }

  static SImage                 fromData(const char *, int, const QSize &maxsize = QSize(), const char *format = NULL);
  inline static SImage          fromData(const QByteArray &data, const QSize &maxsize = QSize(), const char *format = NULL) { return fromData(data.data(), data.size(), maxsize, format); }
  static SImage                 fromData(QIODevice *, const QSize &maxsize = QSize(), const char *format = NULL);
  static SImage                 fromFile(const QUrl &filePath, const QSize &maxsize = QSize(), const char *format = NULL);

  static const QSet<QString>  & rawImageSuffixes(void);
  static QString                rawImageDescription(const QString &suffix);

private:
  static SImage                 handleRawFile(QIODevice *, QSize, const QString &suffix);
  static SImage                 handleFile(QImageReader &, QSize, void * = NULL);

private:
  struct
  {
    float                       aspectRatio;
    QSize                       originalSize;
  }                             d;
};

} // End of namespace

#endif
