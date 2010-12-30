/***************************************************************************
 *   Copyright (C) 2010 by A.J. Admiraal                                   *
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

#include <liblxistreamgl/sglsystem.h>
#undef printLastGlError
#undef printGlInfoLog

#ifdef Q_OS_UNIX
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

namespace LXiStreamGl {


const GLuint  SGlSystem::invalidHandle = (sizeof(GLuint) == 4) ? GLuint(4294967295ul) : GLuint(65535u);
unsigned      SGlSystem::textureCacheSize = 24;
GLint         SGlSystem::maxTextureSize = 2048;
QGLWidget   * SGlSystem::context = NULL;
bool          SGlSystem::contextOwner = false;


void SGlSystem::initialize(QGLWidget *shareContext)
{
  checkThread(true);

  if ((context == NULL) && qobject_cast<QApplication *>(QCoreApplication::instance()))
  {
    contextOwner = shareContext == NULL;
    context = contextOwner ? new QGLWidget() : shareContext;
    const_cast<QGLContext *>(context->context())->makeCurrent();

    glEnable(GL_TEXTURE_2D);
    glClearColor(0, 0, 0, 0);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

    // Ensure textureing works.
    GLuint dummy1 = 0, dummy2 = 0;
    glGenTextures(1, &dummy1);
    glGenTextures(1, &dummy2);
    if (dummy1 == dummy2)
      qFatal("Failed to initialize OpenGL texturing");
    glDeleteTextures(1, &dummy1);
    glDeleteTextures(1, &dummy2);
  }
  else if (shareContext)
    qFatal("SGlSystem already initialized; can not share the specified context.");
}

void SGlSystem::shutdown(void)
{
  if (context != NULL)
  {
    checkThread();

    while (!textureCache().isEmpty())
    {
      const Texture t = textureCache().dequeue();

      if (t.frameBuffer)
        delete t.frameBuffer;
      else
        glDeleteTextures(1, &t.handle);

      textureMem() -= quint64(t.width * t.height) * quint64(sizeof(quint32));
    }

    if (contextOwner)
      delete context;

    contextOwner = false;
    context = NULL;

    printLastGlError(__FILE__, __LINE__);
  }
}

bool SGlSystem::canOffloadProcessing(void)
{
  if (context)
    return QGLFramebufferObject::hasOpenGLFramebufferObjects();
  else
    return false;
}

/*! Throws a qFatal() if the calling thread is not the correct thread for
    handling OpenGl calls. Invoke this method to check this threading
    requirement before calling OpenGl methods.
 */
void SGlSystem::checkThread(bool firstTime)
{
  static QThread *openGlThread = NULL;

  if (!firstTime && (openGlThread == NULL))
    qFatal("SGlSystem: Trying to execute OpenGL calls before SGlSystem::initialize() is invoked.");

  if (openGlThread == NULL)
    openGlThread = QThread::currentThread();
  else if (openGlThread != QThread::currentThread())
    qFatal("SGlSystem: Trying to execute OpenGL calls from the wrong thread, please check your threading design.");
}

/*! Checks glGetError() for errors and throws a qWarning() if there is one.

    \returns False if no errors found, true otherwise.
 */
bool SGlSystem::printLastGlError(const char *file, int line)
{
  const GLenum error = glGetError();

  if (__builtin_expect(error != GL_NO_ERROR, false))
  {
    qWarning() << file << ":" << line << "- OpenGL error :"
               << (const char *)gluErrorString(error);

    return true;
  }

  return false;
}

void SGlSystem::printGlInfoLog(GLuint program, const char *file, int line)
{
#ifdef ENABLE_GLSL
  int len = 0;

  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

  if (len > 0)
  {
    int used = 0;
    char *infoLog = new char[len];

    glGetProgramInfoLog(program, len, &used, infoLog);
    if (used > 0)
      qWarning() << file << ":" << line << "- GLSL compiler results :"
                 << infoLog;

    delete [] infoLog;
  }
#endif
}

void SGlSystem::activateContext(void)
{
  if (context)
    const_cast<QGLContext *>(context->context())->makeCurrent();
}

/*! Rounds the specified size up to a size that matches 2^n and is >= 64, the
    maximum size returned is 2048. This is needed for optimal performance on all
    systems.
 */
unsigned SGlSystem::toTextureSize(unsigned s)
{
  if      ((s <= 64)   || (maxTextureSize < 128))  return 64;
  else if ((s <= 128)  || (maxTextureSize < 256))  return 128;
  else if ((s <= 256)  || (maxTextureSize < 512))  return 256;
  else if ((s <= 512)  || (maxTextureSize < 1024)) return 512;
  else if ((s <= 1024) || (maxTextureSize < 2048)) return 1024;
  else if ((s <= 2048) || (maxTextureSize < 4096)) return 2048;
  else if ((s <= 4096) || (maxTextureSize < 8192)) return 4096;
  else                                             return 8192;
}

const void * SGlSystem::emptyPixels(void)
{
  static void * d = NULL;

  if (d == NULL)
  {
    const size_t blockSize = size_t(maxTextureSize) * size_t(maxTextureSize) * sizeof(quint32);

#ifdef Q_OS_UNIX
    const size_t pageSize = sysconf(_SC_PAGESIZE);
    const QByteArray name = "/LXiStreamGl_SGlSystem_dummyData_" +
                            QByteArray::number(getpid());

    int fd1 = shm_open(name + "_A", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    int fd2 = shm_open(name + "_B", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);

    if ((fd1 >= 0) && (fd2 >= 0))
    {
      ftruncate(fd1, blockSize);
      ftruncate(fd2, pageSize);

      d = mmap(NULL, blockSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
      if (d != MAP_FAILED)
      {
        bool ok = true;
        for (size_t i=0; i<blockSize; i+=pageSize)
          ok &= (mmap(((quint8 *)d) + i, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd2, 0) != MAP_FAILED);

        shm_unlink("dummy1");
        memset(d, 0, pageSize); // Fully transparent

        if (ok)
          return d;
      }
      else
        shm_unlink("dummy1");

      // Failed
      munmap(d, blockSize);
      shm_unlink("dummy2");
    }
#endif

    // Fallback code
    d = new quint8[blockSize];
    memset(d, 0, blockSize); // Fully transparent
  }

  return d;
}

/*! Creates and fills an empty texture of the specified width and height. The
    width and height are rounded to the nearest texture size with
    toTextureSize(). This method will recycle textures, therefore they are not
    guaranteed to be black.

    \note Only use glTexSubImage2D() to upload data to these textures.
 */
SGlSystem::Texture SGlSystem::createTexture(unsigned width, unsigned height, bool fbo, const void * fromData)
{
  checkThread();
  
  const unsigned tw = toTextureSize(width), th = toTextureSize(height);

  Texture tex;

  // Find a texture in the cache.
  for (QQueue<Texture>::Iterator i=textureCache().begin(); i!=textureCache().end(); i++)
  if ((i->width == tw) && (i->height == th) && (bool(i->frameBuffer) == fbo))
  {
    tex = *i;
    textureCache().erase(i);

    glBindTexture(GL_TEXTURE_2D, tex.handle);

    if (fromData)
    {
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                      tex.width,
                      tex.height,
                      GL_RGBA,
                      GL_UNSIGNED_BYTE,
                      fromData);
    }
    
    break;
  }
  
  // If none found, create a new one
  if (tex.handle == invalidHandle)
  {
    if (fbo && QGLFramebufferObject::hasOpenGLFramebufferObjects())
    {
      const_cast<QGLContext *>(context->context())->makeCurrent();

      tex.frameBuffer = new QGLFramebufferObject(tw, th);
      tex.handle = tex.frameBuffer->texture();

      glBindTexture(GL_TEXTURE_2D, tex.handle);
    }
    else
    {
      glGenTextures(1, &tex.handle);
      glBindTexture(GL_TEXTURE_2D, tex.handle);
      glTexImage2D(GL_TEXTURE_2D, 0,
                   GL_RGBA,
                   tw,
                   th,
                   0,
                   GL_RGBA,
                   GL_UNSIGNED_BYTE,
                   fromData ? fromData : emptyPixels());
    }

    tex.width = tw;
    tex.height = th;
    tex.wf = float(width) / float(tex.width);
    tex.hf = float(height) / float(tex.height);

    textureMem() += quint64(tex.width * tex.height) * quint64(sizeof(quint32));
  }
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  printLastGlError(__FILE__, __LINE__);

  return tex;
}

void SGlSystem::deleteTexture(const Texture &tex)
{
  checkThread();
  
  textureCache().enqueue(tex);
  
  while (textureCache().count() > int(textureCacheSize))
  {
    const Texture t = textureCache().dequeue();
    
    if (t.frameBuffer)
      delete t.frameBuffer;
    else
      glDeleteTextures(1, &t.handle);

    textureMem() -= quint64(t.width * t.height) * quint64(sizeof(quint32));
  }

  printLastGlError(__FILE__, __LINE__);
}

const SGlSystem::Texture & SGlSystem::blackTexture(void)
{
  static Texture tex;

  if (__builtin_expect(tex.handle == invalidHandle, false))
  {
    const unsigned s = toTextureSize(1), s2 = s * s;
    QRgb data[s2];
    for (unsigned i=0; i<s2; i++)
      data[i] = 0xFF000000;

    tex = createTexture(s, s, false, data);

    glBindTexture(GL_TEXTURE_2D, tex.handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  return tex;
}

const SGlSystem::Texture & SGlSystem::whiteTexture(void)
{
  static Texture tex;

  if (__builtin_expect(tex.handle == invalidHandle, false))
  {
    const unsigned s = toTextureSize(1), s2 = s * s;
    QRgb data[s2];
    for (unsigned i=0; i<s2; i++)
      data[i] = 0xFFFFFFFF;

    tex = createTexture(s, s, false, data);

    glBindTexture(GL_TEXTURE_2D, tex.handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  return tex;
}

QQueue<SGlSystem::Texture> & SGlSystem::textureCache(void)
{
  static QQueue<Texture> l;

  return l;
}

quint64 & SGlSystem::textureMem(void)
{
  static quint64 m = 0;

  return m;
}


} // End of namespace
