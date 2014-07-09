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
#include "connection_manager.h"
#include "content_directory.h"
#include "mediareceiver_registrar.h"
#include "rootdevice.h"
#include "upnp.h"

namespace lximediacenter {

class messageloop;

class backend
{
public:
  explicit backend(class messageloop &);
  ~backend();

  bool initialize();

private:
  int http_request(const upnp::request &, std::string &, std::shared_ptr<std::istream> &);

private:
  static const uint16_t default_port = 4280;

  class messageloop &messageloop;

  class upnp upnp;
  class rootdevice rootdevice;
  class connection_manager connection_manager;
  class content_directory content_directory;
  class mediareceiver_registrar mediareceiver_registrar;
//  static const int              upnpRepublishTimout;
//  bool                          upnpRepublishRequired;
//  QTimer                        upnpRepublishTimer;
};

} // End of namespace

#endif
