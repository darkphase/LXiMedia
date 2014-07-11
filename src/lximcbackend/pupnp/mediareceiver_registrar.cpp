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

#include "mediareceiver_registrar.h"

namespace lximediacenter {
namespace pupnp {

const char mediareceiver_registrar::service_id[]   = "urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar";
const char mediareceiver_registrar::service_type[] = "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1";

mediareceiver_registrar::mediareceiver_registrar(class messageloop &messageloop, class rootdevice &rootdevice)
  : messageloop(messageloop),
    rootdevice(rootdevice),
    authorization_granted_updateid(1),
    authorization_denied_updateid(1),
    validation_succeeded_updateid(1),
    validation_revoked_updateid(1)
{
  rootdevice.service_register(service_id, *this);
}

mediareceiver_registrar::~mediareceiver_registrar()
{
  rootdevice.service_unregister(service_id);
}

const char * mediareceiver_registrar::get_service_type(void)
{
  return service_type;
}

void mediareceiver_registrar::initialize(void)
{
}

void mediareceiver_registrar::close(void)
{
}

void mediareceiver_registrar::write_service_description(rootdevice::service_description &desc) const
{
  {
    static const char * const argname[] = { "DeviceID"            , "Result"            };
    static const char * const argdir[]  = { "in"                  , "out"               };
    static const char * const argvar[]  = { "A_ARG_TYPE_DeviceID" , "A_ARG_TYPE_Result" };
    desc.add_action("IsAuthorized", argname, argdir, argvar);
  }
  {
    static const char * const argname[] = { "DeviceID"            , "Result"            };
    static const char * const argdir[]  = { "in"                  , "out"               };
    static const char * const argvar[]  = { "A_ARG_TYPE_DeviceID" , "A_ARG_TYPE_Result" };
    desc.add_action("IsValidated", argname, argdir, argvar);
  }
  {
    static const char * const argname[] = { "RegistrationReqMsg"            , "RegistrationRespMsg"           };
    static const char * const argdir[]  = { "in"                            , "out"                           };
    static const char * const argvar[]  = { "A_ARG_TYPE_RegistrationReqMsg" , "A_ARG_TYPE_RegistrationRespMsg"};
    desc.add_action("RegisterDevice", argname, argdir, argvar);
  }

  desc.add_statevariable("A_ARG_TYPE_DeviceID"           , "string"    , false );
  desc.add_statevariable("A_ARG_TYPE_Result"             , "int"       , false );
  desc.add_statevariable("A_ARG_TYPE_RegistrationReqMsg" , "bin.base64", false );
  desc.add_statevariable("A_ARG_TYPE_RegistrationRespMsg", "bin.base64", false );
  desc.add_statevariable("AuthorizationGrantedUpdateID"  , "ui4"       , true  );
  desc.add_statevariable("AuthorizationDeniedUpdateID"   , "ui4"       , true  );
  desc.add_statevariable("ValidationSucceededUpdateID"   , "ui4"       , true  );
  desc.add_statevariable("ValidationRevokedUpdateID"     , "ui4"       , true  );
}

void mediareceiver_registrar::write_eventable_statevariables(rootdevice::eventable_propertyset &propset) const
{
  propset.add_property("AuthorizationGrantedUpdateID", std::to_string(authorization_granted_updateid));
  propset.add_property("AuthorizationDeniedUpdateID" , std::to_string(authorization_denied_updateid ));
  propset.add_property("ValidationSucceededUpdateID" , std::to_string(validation_succeeded_updateid ));
  propset.add_property("ValidationRevokedUpdateID"   , std::to_string(validation_revoked_updateid   ));
}

void mediareceiver_registrar::handle_action(const upnp::request &, action_is_authorized &action)
{
  action.set_response(1);
}

void mediareceiver_registrar::handle_action(const upnp::request &, action_is_validated &action)
{
  action.set_response(1);
}

void mediareceiver_registrar::handle_action(const upnp::request &, action_register_device &action)
{
  action.set_response("DUMMY");
}

} // End of namespace
} // End of namespace
