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

#ifndef PUPNP_ROOTDEVICE_H
#define PUPNP_ROOTDEVICE_H

#include "upnp.h"
#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace pupnp {

class rootdevice : private upnp::child
{
public:
    struct device_description
    {
        virtual void set_devicetype(const std::string &, const std::string &dlnaDoc) = 0;
        virtual void set_friendlyname(const std::string &) = 0;
        virtual void set_manufacturer(const std::string &manufacturer, const std::string &url) = 0;
        virtual void set_model(const std::string &description, const std::string &name, const std::string &url, const std::string &number) = 0;
        virtual void set_serialnumber(const std::string &) = 0;
        virtual void set_udn(const std::string &) = 0;
        virtual void add_icon(const std::string &url, const char *mimetype, int width, int height, int depth) = 0;
        virtual void set_presentation_url(const std::string &) = 0;
    };

    struct service_description
    {
        virtual void           add_action(const char *name, const char * const *argname, const char * const *argdir, const char * const *argvar, int argcount) = 0;
        template <int _c> void add_action(const char *name, const char * const(&argname)[_c], const char * const(&argdir)[_c], const char * const(&argvar)[_c]) { add_action(name, argname, argdir, argvar, _c); }
        virtual void           add_statevariable(const char *name, const char *type, bool sendEvents, const char * const *values, int valcount) = 0;
        template <int _c> void add_statevariable(const char *name, const char *type, bool sendEvents, const char * const(&values)[_c]) { add_statevariable(name, type, sendEvents, values, _c); }
        void                   add_statevariable(const char *name, const char *type, bool sendEvents) { add_statevariable(name, type, sendEvents, NULL, 0); }
    };

    struct eventable_propertyset
    {
        virtual void add_property(const std::string &name, const std::string &value) = 0;
    };

    struct service
    {
        virtual const char * get_service_type(void) = 0;

        virtual void initialize(void) = 0;
        virtual void close(void) = 0;

        virtual void write_service_description(service_description &) const = 0;
        virtual void write_eventable_statevariables(eventable_propertyset &) const = 0;
    };

public:
    rootdevice(class messageloop &, class upnp &, const std::string &uuid, const std::string &devicetype);
    virtual ~rootdevice();

    const std::string &http_basedir() const;

    void set_devicename(const std::string &);
    void add_icon(const std::string &path);

    void service_register(const std::string &service_id, struct service &);
    void service_unregister(const std::string &service_id);

    virtual bool initialize();
    virtual void close();

    void emit_event(const std::string &service_id);

    std::string udn() const;

    std::map<void *, std::function<void()>> handled_action;

private:
    void handle_event(const std::string &service_id, eventable_propertyset &);
    void write_device_description(device_description &);
    int http_request(const upnp::request &, std::string &, std::shared_ptr<std::istream> &);
    bool enable_rootdevice(void);

private:
    static const char devicedescriptionfile[];
    static const char servicedescriptionfile[];
    static const char servicecontrolfile[];
    static const char serviceeventfile[];

    class messageloop &messageloop;
    class upnp &upnp;
    const std::string uuid;
    const std::string devicetype;
    const std::string basedir;
    std::string devicename;
    std::vector<std::string> icons;
    bool initialized;

    std::map<std::string, std::pair<struct service *, std::string>> services;

    bool rootdevice_registred;
    std::map<std::string, int> rootdevice_handles;
};

} // End of namespace

#endif
