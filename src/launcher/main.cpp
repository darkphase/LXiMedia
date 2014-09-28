#include "platform/fstream.h"
#include "platform/messageloop.h"
#include "platform/string.h"
#include "pupnp/client.h"
#include "pupnp/upnp.h"
#include <clocale>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>

static bool open_url(const std::string &location);

static bool find_server(
        class messageloop &messageloop,
        class pupnp::upnp &upnp,
        class pupnp::client &client)
{
    bool found = false;
    client.device_discovered = [&messageloop, &upnp, &client, &found](const std::string &device_id, const std::string &location)
    {
        if (!found && starts_with(location, "http://"))
        {
            const size_t sl = location.find_first_of(":/", 7);
            if ((sl != location.npos) && upnp.is_my_address(location.substr(7, sl - 7)))
            {
                struct pupnp::client::device_description device_description;
                if (client.read_device_description(location, device_description))
                {
                    open_url(device_description.presentation_url);
                    found = true;
                    messageloop.stop(0);
                }
            }
        }
    };

    client.start_search("urn:schemas-upnp-org:device:MediaServer:1");

    messageloop.process_events(std::chrono::seconds(3));

    return found;
}

static void send_error(
        class messageloop &messageloop,
        class pupnp::upnp &upnp)
{
    upnp.http_callback_register("/", [&messageloop](const pupnp::upnp::request &, std::string &content_type, std::shared_ptr<std::istream> &response)
    {
        content_type = pupnp::upnp::mime_text_html;

        auto out = std::make_shared<std::stringstream>();
        (*out) << "<!DOCTYPE html>\n"
                  "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\">"
                  "<head>"
                  "<title>Not Found</title>"
                  "<meta http-equiv=\"Content-Type\" content=\"" << content_type << "\" />"
                  "<style>body { padding: 0; margin: 0; font-family: sans-serif; font-size: 75%; "
                  "font-weight: normal; font-style: normal; text-align: left; background-color: #FFFFFF; "
                  "color: #000000; } p { padding: 0; margin: 0.5em; } h1 { font-size: 2em; font-weight: bold; "
                  "color: #808080; border: 1px solid #FFFFFF; border-bottom-color: #808080; }</style>"
                  "</head><body>"
                  "<h1>Failed to locate LXiMediaCenter on your PC</h1>"
                  "<p>Please check if it is installed correctly, try to re-install it to fix the problem.</p>"
                  "</body></html>";

        response = out;

        messageloop.stop(0);
        return pupnp::upnp::http_ok;
    });

    auto &bound_addresses = upnp.bound_addresses();
    if (!bound_addresses.empty())
    {
        open_url("http://" + (*bound_addresses.begin()) + ':' + std::to_string(upnp.bound_port()) + "/");

        messageloop.process_events(std::chrono::seconds(3));
        messageloop.process_events(std::chrono::milliseconds(500));
    }
}

int main(int /*argc*/, const char */*argv*/[])
{
    setlocale(LC_ALL, "");

    class messageloop messageloop;
    class pupnp::upnp upnp(messageloop);
    class pupnp::client client(messageloop, upnp);
    upnp.initialize(0);

    if (!find_server(messageloop, upnp, client))
        send_error(messageloop, upnp);

    upnp.close();

    return 0;
}

#if defined(__unix__)
static bool open_url(const std::string &location)
{
    return system(("xdg-open " + location).c_str()) == 0;
}
#elif defined(WIN32)
static bool open_url(const std::string &location)
{
    return system(("start " + location).c_str()) == 0;
}
#endif
