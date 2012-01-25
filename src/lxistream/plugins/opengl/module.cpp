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

#include "module.h"
#include <QtGui/QApplication>
#include <LXiStreamGl>

#ifdef Q_OS_UNIX
#include "deinterlace.h"
#endif

namespace LXiStream {
namespace OpenGlBackend {


void Module::registerClasses(void)
{
  if ((SSystem::initializeFlags() & SSystem::Initialize_UseOpenGL) == SSystem::Initialize_UseOpenGL)
  {
    if (QCoreApplication::instance() == NULL)
      qFatal("Please create a QApplication object before loading the OpenGlBackend");

    if (qobject_cast<QApplication *>(QCoreApplication::instance()) == NULL)
      return; // Non-gui application

    SGlSystem::initialize(NULL);

    if (!QGLFormat::hasOpenGL())
      return;

#ifdef Q_OS_UNIX
    if (SGlSystem::canOffloadProcessing())
    {
      DeinterlaceBlend::registerClass<DeinterlaceBlend>(SFactory::Scheme(0, "blend"));
      DeinterlaceBob::registerClass<DeinterlaceBob>(SFactory::Scheme(-1, "bob"));
    }
#endif
  }
}

void Module::unload(void)
{
}


} } // End of namespaces
