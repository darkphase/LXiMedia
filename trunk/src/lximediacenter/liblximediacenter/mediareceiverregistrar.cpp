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

#include "mediareceiverregistrar.h"
#include "mediaserver.h"

namespace LXiMediaCenter {

const char  MediaReceiverRegistrar::serviceId[] = "urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar";

struct MediaReceiverRegistrar::Data
{
  static const QEvent::Type     emitUpdateEventType;

  quint32                       authorizationGrantedUpdateID;
  quint32                       authorizationDeniedUpdateID;
  quint32                       validationSucceededUpdateID;
  quint32                       validationRevokedUpdateID;
};

const QEvent::Type  MediaReceiverRegistrar::Data::emitUpdateEventType = QEvent::Type(QEvent::registerEventType());

MediaReceiverRegistrar::MediaReceiverRegistrar(RootDevice *parent)
  : QObject(parent),
    parent(parent),
    d(new Data())
{
  d->authorizationGrantedUpdateID = 1;
  d->authorizationDeniedUpdateID = 1;
  d->validationSucceededUpdateID = 1;
  d->validationRevokedUpdateID = 1;

  parent->registerService(serviceId, this);
}

MediaReceiverRegistrar::~MediaReceiverRegistrar()
{
  parent->unregisterService(serviceId);

  delete d;
  *const_cast<Data **>(&d) = NULL;
}

const char * MediaReceiverRegistrar::serviceType(void)
{
  return RootDevice::serviceTypeMediaReceiverRegistrar;
}

void MediaReceiverRegistrar::initialize(void)
{
}

void MediaReceiverRegistrar::close(void)
{
}

void MediaReceiverRegistrar::writeServiceDescription(RootDevice::ServiceDescription &desc) const
{
  {
    static const char * const argname[] = { "DeviceID"            , "Result"            };
    static const char * const argdir[]  = { "in"                  , "out"               };
    static const char * const argvar[]  = { "A_ARG_TYPE_DeviceID" , "A_ARG_TYPE_Result" };
    desc.addAction("IsAuthorized", argname, argdir, argvar);
  }
  {
    static const char * const argname[] = { "DeviceID"            , "Result"            };
    static const char * const argdir[]  = { "in"                  , "out"               };
    static const char * const argvar[]  = { "A_ARG_TYPE_DeviceID" , "A_ARG_TYPE_Result" };
    desc.addAction("IsValidated", argname, argdir, argvar);
  }
  {
    static const char * const argname[] = { "RegistrationReqMsg"            , "RegistrationRespMsg"           };
    static const char * const argdir[]  = { "in"                            , "out"                           };
    static const char * const argvar[]  = { "A_ARG_TYPE_RegistrationReqMsg" , "A_ARG_TYPE_RegistrationRespMsg"};
    desc.addAction("RegisterDevice", argname, argdir, argvar);
  }

  desc.addStateVariable("A_ARG_TYPE_DeviceID"           , "string"    , false );
  desc.addStateVariable("A_ARG_TYPE_Result"             , "int"       , false );
  desc.addStateVariable("A_ARG_TYPE_RegistrationReqMsg" , "bin.base64", false );
  desc.addStateVariable("A_ARG_TYPE_RegistrationRespMsg", "bin.base64", false );
  desc.addStateVariable("AuthorizationGrantedUpdateID"  , "ui4"       , true  );
  desc.addStateVariable("AuthorizationDeniedUpdateID"   , "ui4"       , true  );
  desc.addStateVariable("ValidationSucceededUpdateID"   , "ui4"       , true  );
  desc.addStateVariable("ValidationRevokedUpdateID"     , "ui4"       , true  );
}

void MediaReceiverRegistrar::writeEventableStateVariables(RootDevice::EventablePropertySet &propset) const
{
  propset.addProperty("AuthorizationGrantedUpdateID", QString::number(d->authorizationGrantedUpdateID ));
  propset.addProperty("AuthorizationDeniedUpdateID" , QString::number(d->authorizationDeniedUpdateID  ));
  propset.addProperty("ValidationSucceededUpdateID" , QString::number(d->validationSucceededUpdateID  ));
  propset.addProperty("ValidationRevokedUpdateID"   , QString::number(d->validationRevokedUpdateID    ));
}

void MediaReceiverRegistrar::customEvent(QEvent *e)
{
  if (e->type() == d->emitUpdateEventType)
    parent->emitEvent(serviceId);
  else
    QObject::customEvent(e);
}

void MediaReceiverRegistrar::handleAction(const RootDevice::HttpCallback::RequestInfo &, ActionIsAuthorized &action)
{
  action.setResponse(1);
}

void MediaReceiverRegistrar::handleAction(const RootDevice::HttpCallback::RequestInfo &, ActionIsValidated &action)
{
  action.setResponse(1);
}

void MediaReceiverRegistrar::handleAction(const RootDevice::HttpCallback::RequestInfo &, ActionRegisterDevice &action)
{
  action.setResponse("DUMMY");
}

} // End of namespace
