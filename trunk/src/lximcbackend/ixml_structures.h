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

#ifndef LXIMEDIACENTER_IXMLSTRUCTURES_H
#define LXIMEDIACENTER_IXMLSTRUCTURES_H

#include "rootdevice.h"
#include "connection_manager.h"

struct _IXML_Document;
struct _IXML_Element;
struct _IXML_Node;

namespace lximediacenter {
namespace ixml_structures {

class xml_structure
{
public:
  xml_structure();
  explicit xml_structure(_IXML_Document *&);
  virtual ~xml_structure();

  _IXML_Element * add_element(_IXML_Node *to, const std::string &name);
  _IXML_Element * add_element(_IXML_Node *to, const std::string &ns, const std::string &name);
  _IXML_Element * add_textelement(_IXML_Node *to, const std::string &name, const std::string &value);
  _IXML_Element * add_textelement(_IXML_Node *to, const std::string &ns, const std::string &name, const std::string &value);
  void set_attribute(_IXML_Element *, const std::string &name, const std::string &value);

  std::string get_textelement(_IXML_Node *from, const std::string &name) const;

public:
  _IXML_Document * const doc;

private:
  _IXML_Document ** const dest;
};

class device_description final : public xml_structure, public rootdevice::device_description
{
public:
  explicit device_description(const std::string &host, const std::string &baseDir);

  virtual void set_devicetype(const std::string &, const std::string &dlnaDoc) override;
  virtual void set_friendlyname(const std::string &) override;
  virtual void set_manufacturer(const std::string &manufacturer, const std::string &url) override;
  virtual void set_model(const std::string &description, const std::string &name, const std::string &url, const std::string &number) override;
  virtual void set_serialnumber(const std::string &) override;
  virtual void set_udn(const std::string &) override;
  virtual void add_icon(const std::string &url, const char *mimetype, int width, int height, int depth) override;
  virtual void set_presentation_url(const std::string &) override;

  void add_service(const std::string &service_type, const std::string &service_id, const std::string &description_file, const std::string &control_file, const std::string &event_file);

private:
  _IXML_Element * const root;
  _IXML_Element * device;
  _IXML_Element * iconlist;
  _IXML_Element * servicelist;

  const std::string host;
};

class service_description final : public xml_structure, public rootdevice::service_description
{
public:
  service_description();

  virtual void add_action(const char *name, const char * const *argname, const char * const *argdir, const char * const *argvar, int argcount) override;
  virtual void add_statevariable(const char *name, const char *type, bool sendEvents, const char * const *values, int valcount) override;

private:
  _IXML_Element * const scpd;
  _IXML_Element * actionlist;
  _IXML_Element * servicestatetable;
};

class eventable_propertyset final : public xml_structure, public rootdevice::eventable_propertyset
{
public:
  eventable_propertyset();

  virtual void add_property(const std::string &name, const std::string &value) override;

private:
  _IXML_Element * const propertySet;
};

class action_get_current_connectionids final : public xml_structure, public connection_manager::action_get_current_connectionids

{
public:
  action_get_current_connectionids(_IXML_Node *, _IXML_Document *&, const char *);

  virtual void set_response(const std::vector<int32_t> &) override;

private:
  const std::string prefix;
};

class action_get_current_connection_info final : public xml_structure, public connection_manager::action_get_current_connection_info
{
public:
  action_get_current_connection_info(_IXML_Node *, _IXML_Document *&, const char *);

  virtual int32_t get_connectionid() const override;

  virtual void set_response(const connection_manager::connection_info &info) override;

private:
  _IXML_Node * const src;
  const std::string prefix;
};

class action_get_protocol_info final : public connection_manager::action_get_protocol_info,
                              public xml_structure
{
public:
  action_get_protocol_info(_IXML_Node *, _IXML_Document *&, const char *);

  virtual void set_response(const std::string &source, const std::string &sink) override;

private:
  const std::string prefix;
};

#if 0
class action_browse final : public ContentDirectory::action_browse,
                     public xml_structure
{
public:
  explicit                      action_browse(_IXML_Node *, _IXML_Document *&, const char *);

  virtual std::string            get_object_id() const override;
  virtual BrowseFlag            get_browse_flag() const override;
  virtual std::string            get_filter() const override;
  virtual uint32_t               get_starting_index() const override;
  virtual uint32_t               get_requested_count() const override;
  virtual std::string            get_sort_criteria() const override;

  virtual void                  add_item(const ContentDirectory::BrowseItem &) override;
  virtual void                  add_container(const ContentDirectory::BrowseContainer &) override;
  virtual void                  set_response(uint32_t totalMatches, uint32_t updateID) override;

private:
  _IXML_Node            * const src;
  const std::string              prefix;
  xml_structure                  result;
  _IXML_Element         * const didl;
  uint32_t                       numberReturned;
};

class action_search final : public ContentDirectory::action_search,
                     public xml_structure
{
public:
  explicit                      action_search(_IXML_Node *, _IXML_Document *&, const char *);

  virtual std::string            getContainerID() const override;
  virtual std::string            getSearchCriteria() const override;
  virtual std::string            get_filter() const override;
  virtual uint32_t               get_starting_index() const override;
  virtual uint32_t               get_requested_count() const override;
  virtual std::string            get_sort_criteria() const override;

  virtual void                  add_item(const ContentDirectory::BrowseItem &) override;
  virtual void                  set_response(uint32_t totalMatches, uint32_t updateID) override;

private:
  _IXML_Node            * const src;
  const std::string              prefix;
  xml_structure                  result;
  _IXML_Element         * const didl;
  uint32_t                       numberReturned;
};

class action_get_search_capabilities final : public ContentDirectory::action_get_search_capabilities,
                                    public xml_structure
{
public:
  explicit                      action_get_search_capabilities(_IXML_Node *, _IXML_Document *&, const char *);

  virtual void                  set_response(const std::string &) override;

private:
//  _IXML_Node            * const src;
  const std::string              prefix;
};

class action_get_sort_capabilities final : public ContentDirectory::action_get_sort_capabilities,
                                  public xml_structure
{
public:
  explicit                      action_get_sort_capabilities(_IXML_Node *, _IXML_Document *&, const char *);

  virtual void                  set_response(const std::string &) override;

private:
//  _IXML_Node            * const src;
  const std::string              prefix;
};

class action_get_system_update_id final : public ContentDirectory::action_get_system_update_id,
                                public xml_structure
{
public:
  explicit                      action_get_system_update_id(_IXML_Node *, _IXML_Document *&, const char *);

  virtual void                  set_response(uint32_t) override;

private:
//  _IXML_Node            * const src;
  const std::string              prefix;
};

// Samsung GetFeatureList
class action_get_featurelist final : public ContentDirectory::action_get_featurelist,
                             public xml_structure
{
public:
  explicit                      action_get_featurelist(_IXML_Node *, _IXML_Document *&, const char *);

  virtual void                  set_response(const std::vector<std::string> &) override;

private:
//  _IXML_Node            * const src;
  const std::string              prefix;
};

// Microsoft MediaReceiverRegistrar IsAuthorized
class ActionIsAuthorized final : public MediaReceiverRegistrar::ActionIsAuthorized,
                           public xml_structure
{
public:
  explicit                      ActionIsAuthorized(_IXML_Node *, _IXML_Document *&, const char *);

  virtual std::string            getDeviceID() const override;
  virtual void                  set_response(int) override;

private:
  _IXML_Node            * const src;
  const std::string              prefix;
};

// Microsoft MediaReceiverRegistrar IsValidated
class ActionIsValidated final : public MediaReceiverRegistrar::ActionIsValidated,
                          public xml_structure
{
public:
  explicit                      ActionIsValidated(_IXML_Node *, _IXML_Document *&, const char *);

  virtual std::string            getDeviceID() const override;
  virtual void                  set_response(int) override;

private:
  _IXML_Node            * const src;
  const std::string              prefix;
};

// Microsoft MediaReceiverRegistrar RegisterDevice
class ActionRegisterDevice final : public MediaReceiverRegistrar::ActionRegisterDevice,
                             public xml_structure
{
public:
  explicit                      ActionRegisterDevice(_IXML_Node *, _IXML_Document *&, const char *);

  virtual std::string            getRegistrationReqMsg() const override;
  virtual void                  set_response(const std::string &) override;

private:
  _IXML_Node            * const src;
  const std::string              prefix;
};
#endif

} // End of namespace
} // End of namespace

#endif
