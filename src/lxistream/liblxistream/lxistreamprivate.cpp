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

#include "lxistreamprivate.h"
#include "saudiobuffer.h"
#include "saudiocodec.h"
#include "sdatacodec.h"
#include "sencodedaudiobuffer.h"
#include "sencodeddatabuffer.h"
#include "sencodedvideobuffer.h"
#include "smediainfo.h"
#include "ssubpicturebuffer.h"
#include "ssubtitlebuffer.h"
#include "svideobuffer.h"
#include "svideocodec.h"

#include "common/module.h"

class LXiStreamInit : public SApplication::Initializer
{
public:
  virtual void                  startup(void);
  virtual void                  shutdown(void);

private:
  static LXiStreamInit          self;
};

LXiStreamInit LXiStreamInit::self;

void LXiStreamInit::startup(void)
{
  static bool firsttime = true;
  if (firsttime)
  {
    firsttime = false;

    // Register metatypes.
    qRegisterMetaType<SEncodedAudioBuffer>("SEncodedAudioBuffer");
    qRegisterMetaType<SAudioBuffer>("SAudioBuffer");
    qRegisterMetaType<SEncodedVideoBuffer>("SEncodedVideoBuffer");
    qRegisterMetaType<SVideoBuffer>("SVideoBuffer");
    qRegisterMetaType<SEncodedDataBuffer>("SEncodedDataBuffer");
    qRegisterMetaType<SSubpictureBuffer>("SSubpictureBuffer");
    qRegisterMetaType<SSubtitleBuffer>("SSubtitleBuffer");

    qRegisterMetaType<SAudioCodec>("SAudioCodec");
    qRegisterMetaType<SVideoCodec>("SVideoCodec");
    qRegisterMetaType<SDataCodec>("SDataCodec");

    qRegisterMetaType<SInterval>("SInterval");
    qRegisterMetaType<STime>("STime");
    qRegisterMetaType<SMediaInfo>("SMediaInfo");
  }

  sApp->addModuleFilter("lxistream");

  // Always load the Common module.
  Common::Module * const module = new Common::Module();
  if (!sApp->loadModule(module))
    delete module;
}

void LXiStreamInit::shutdown(void)
{
}
