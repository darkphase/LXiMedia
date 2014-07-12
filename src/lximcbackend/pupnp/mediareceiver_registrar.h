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

#ifndef PUPNP_MEDIARECEIVER_REGISTRAR_H
#define PUPNP_MEDIARECEIVER_REGISTRAR_H

#include "rootdevice.h"
#include <cstdint>

namespace pupnp {

class mediareceiver_registrar : public rootdevice::service
{
public:
  struct action_is_authorized
  {
    virtual std::string get_deviceid() const = 0;
    virtual void set_response(int) = 0;
  };

  struct action_is_validated
  {
    virtual std::string get_deviceid() const = 0;
    virtual void set_response(int) = 0;
  };

  struct action_register_device
  {
    virtual std::string get_registration_req_msg() const = 0;
    virtual void set_response(const std::string &) = 0;
  };

public:
  static const char service_id[];
  static const char service_type[];

  explicit mediareceiver_registrar(class messageloop &, class rootdevice &);
  virtual ~mediareceiver_registrar();

  void handle_action(const upnp::request &, action_is_authorized &);
  void handle_action(const upnp::request &, action_is_validated &);
  void handle_action(const upnp::request &, action_register_device &);

private: // From rootdevice::service
  virtual const char * get_service_type(void) override final;

  virtual void initialize(void) override final;
  virtual void close(void) override final;

  virtual void write_service_description(rootdevice::service_description &) const override final;
  virtual void write_eventable_statevariables(rootdevice::eventable_propertyset &) const override final;

private:
  class messageloop &messageloop;
  class rootdevice &rootdevice;

  uint32_t authorization_granted_updateid;
  uint32_t authorization_denied_updateid;
  uint32_t validation_succeeded_updateid;
  uint32_t validation_revoked_updateid;
};

} // End of namespace

#endif
