/******************************************************************************
 *   Copyright (C) 2014  A.J. Admiraal                                        *
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

#ifndef LXIMEDIACENTER_BACKEND_H
#define LXIMEDIACENTER_BACKEND_H

#include <cstdint>
#include "pupnp/connection_manager.h"
#include "pupnp/content_directory.h"
#include "pupnp/mediareceiver_registrar.h"
#include "pupnp/rootdevice.h"
#include "pupnp/upnp.h"
#include "settings.h"

namespace lximediacenter {

class messageloop;

class backend
{
public:
  explicit backend(class messageloop &);
  ~backend();

  bool initialize();

private:
  int http_request(const pupnp::upnp::request &, std::string &, std::shared_ptr<std::istream> &);

private:
  class messageloop &messageloop;
  class settings settings;

  class pupnp::upnp upnp;
  class pupnp::rootdevice rootdevice;
  class pupnp::connection_manager connection_manager;
  class pupnp::content_directory content_directory;
  class pupnp::mediareceiver_registrar mediareceiver_registrar;
//  static const int              upnpRepublishTimout;
//  bool                          upnpRepublishRequired;
//  QTimer                        upnpRepublishTimer;
};

} // End of namespace

#endif
