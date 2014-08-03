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

#ifndef PUPNP_IXML_STRUCTURES_H
#define PUPNP_IXML_STRUCTURES_H

#include "rootdevice.h"
#include "connection_manager.h"
#include "content_directory.h"
#include "mediareceiver_registrar.h"

struct _IXML_Document;
struct _IXML_Element;
struct _IXML_Node;

namespace pupnp {
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

    virtual void set_devicetype(const std::string &, const std::string &dlnadoc) override;
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
    action_get_current_connectionids(_IXML_Node *, _IXML_Document *&, const std::string &);

    virtual void set_response(const std::vector<int32_t> &) override;

private:
    const std::string prefix;
};

class action_get_current_connection_info final : public xml_structure, public connection_manager::action_get_current_connection_info
{
public:
    action_get_current_connection_info(_IXML_Node *, _IXML_Document *&, const std::string &);

    virtual int32_t get_connectionid() const override;

    virtual void set_response(const connection_manager::connection_info &info) override;

private:
    _IXML_Node * const src;
    const std::string prefix;
};

class action_get_protocol_info final : public xml_structure, public connection_manager::action_get_protocol_info
{
public:
    action_get_protocol_info(_IXML_Node *, _IXML_Document *&, const std::string &);

    virtual void set_response(const std::string &source, const std::string &sink) override;

private:
    const std::string prefix;
};

class action_browse final : public xml_structure, public content_directory::action_browse
{
public:
    explicit action_browse(_IXML_Node *, _IXML_Document *&, const std::string &);

    virtual std::string get_object_id() const override;
    virtual browse_flag get_browse_flag() const override;
    virtual std::string get_filter() const override;
    virtual size_t      get_starting_index() const override;
    virtual size_t      get_requested_count() const override;
    virtual std::string get_sort_criteria() const override;

    virtual void add_item(const content_directory::browse_item &) override;
    virtual void add_container(const content_directory::browse_container &) override;
    virtual void set_response(size_t total_matches, uint32_t update_id) override;

private:
    _IXML_Node * const src;
    const std::string prefix;
    xml_structure result;
    _IXML_Element * const didl;
    size_t number_returned;
};

class action_search final : public xml_structure, public content_directory::action_search
{
public:
    explicit action_search(_IXML_Node *, _IXML_Document *&, const std::string &);

    virtual std::string get_container_id() const override;
    virtual std::string get_search_criteria() const override;
    virtual std::string get_filter() const override;
    virtual size_t      get_starting_index() const override;
    virtual size_t      get_requested_count() const override;
    virtual std::string get_sort_criteria() const override;

    virtual void add_item(const content_directory::browse_item &) override;
    virtual void set_response(size_t total_matches, uint32_t update_id) override;

private:
    _IXML_Node * const src;
    const std::string prefix;
    xml_structure result;
    _IXML_Element * const didl;
    size_t number_returned;
};

class action_get_search_capabilities final : public xml_structure, public content_directory::action_get_search_capabilities
{
public:
    explicit action_get_search_capabilities(_IXML_Node *, _IXML_Document *&, const std::string &);

    virtual void set_response(const std::string &) override;

private:
    const std::string prefix;
};

class action_get_sort_capabilities final : public xml_structure, public content_directory::action_get_sort_capabilities
{
public:
    explicit action_get_sort_capabilities(_IXML_Node *, _IXML_Document *&, const std::string &);

    virtual void set_response(const std::string &) override;

private:
    const std::string prefix;
};

class action_get_system_update_id final : public xml_structure, public content_directory::action_get_system_update_id
{
public:
    explicit action_get_system_update_id(_IXML_Node *, _IXML_Document *&, const std::string &);

    virtual void set_response(uint32_t) override;

private:
    const std::string prefix;
};

// Samsung GetFeatureList
class action_get_featurelist final : public xml_structure, public content_directory::action_get_featurelist
{
public:
    explicit action_get_featurelist(_IXML_Node *, _IXML_Document *&, const std::string &);

    virtual void set_response(const std::vector<std::string> &) override;

private:
    const std::string prefix;
};

// Microsoft MediaReceiverRegistrar IsAuthorized
class action_is_authorized final : public xml_structure, public mediareceiver_registrar::action_is_authorized
{
public:
    explicit action_is_authorized(_IXML_Node *, _IXML_Document *&, const std::string &);

    virtual std::string get_deviceid() const override;
    virtual void set_response(int) override;

private:
    _IXML_Node * const src;
    const std::string prefix;
};

// Microsoft MediaReceiverRegistrar IsValidated
class action_is_validated final : public xml_structure, public mediareceiver_registrar::action_is_validated
{
public:
    explicit action_is_validated(_IXML_Node *, _IXML_Document *&, const std::string &);

    virtual std::string get_deviceid() const override;
    virtual void set_response(int) override;

private:
    _IXML_Node * const src;
    const std::string prefix;
};

// Microsoft MediaReceiverRegistrar RegisterDevice
class action_register_device final : public xml_structure, public mediareceiver_registrar::action_register_device
{
public:
    explicit action_register_device(_IXML_Node *, _IXML_Document *&, const std::string &);

    virtual std::string get_registration_req_msg() const override;
    virtual void set_response(const std::string &) override;

private:
    _IXML_Node * const src;
    const std::string prefix;
};

} // End of namespace
} // End of namespace

#endif
