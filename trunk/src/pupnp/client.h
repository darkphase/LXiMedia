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

#ifndef PUPNP_CLIENT_H
#define PUPNP_CLIENT_H

#include "upnp.h"

namespace pupnp {

class client : private upnp::child
{
public:
  struct device_description
  {
    std::string devicetype;
    std::string friendlyname;
    std::string manufacturer;
    std::string modelname;
    std::string udn;
    std::string icon_url;
    std::string presentation_url;
  };

public:
  client(class platform::messageloop_ref &, class upnp &);
  virtual ~client();

  virtual bool initialize(void);
  virtual void close(void);

  void start_search(const std::string &target, int mx = 3);

  static std::string get(const std::string &location);
  bool read_device_description(const std::string &location, struct device_description &device_description);

  std::function<void(const std::string &, const std::string &)> device_discovered;
  std::function<void(const std::string &)> device_closed;

private:
  bool enable_client(void);

private:
  class platform::messageloop_ref messageloop;
  class upnp &upnp;
  bool client_enabled;
  int client_handle;
};

} // End of namespace

#endif
