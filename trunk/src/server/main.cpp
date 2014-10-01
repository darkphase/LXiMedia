#include "backend.h"
#include "settings.h"
#include "platform/messageloop.h"
#include "platform/string.h"
#include "pupnp/client.h"
#include "pupnp/upnp.h"
#include <clocale>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>

static std::unique_ptr<class backend> backend_ptr;

static int run_server(class platform::messageloop &messageloop)
{
    std::clog << "Starting LXiMediaServer version " << VERSION << std::endl;

    backend::recreate_backend = [&messageloop]
    {
        backend_ptr = nullptr;
        backend_ptr.reset(new class backend(messageloop, std::string()));
        if (!backend_ptr->initialize())
        {
            std::clog << "[" << backend_ptr.get() << "] failed to initialize backend; stopping." << std::endl;
            messageloop.stop(1);
        }
    };

    backend::recreate_backend();
    const int result = messageloop.run();
    backend::recreate_backend = nullptr;

    backend_ptr = nullptr;

    return result;
}

static bool open_url(const std::string &location);

static bool find_server(class platform::messageloop &messageloop, std::chrono::milliseconds duration)
{
    class pupnp::upnp upnp(messageloop);
    class pupnp::client client(messageloop, upnp);

    bool found = false;
    if (upnp.initialize(0))
    {
        const std::string my_id = "uuid:" + settings(messageloop).uuid();

        client.device_discovered = [&messageloop, &upnp, &client, &found, &my_id](const std::string &device_id, const std::string &location)
        {
            if (!found && (device_id == my_id) && starts_with(location, "http://"))
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

        messageloop.process_events(duration);
    }

    return found;
}

static bool is_root();

int main(int argc, const char *argv[])
{
    if (is_root())
    {
        std::cerr << "Cannot run as root." << std::endl;
        return 1;
    }

    setlocale(LC_ALL, "");

    class platform::messageloop messageloop;

    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--run") == 0)
            return run_server(messageloop);

    if (!find_server(messageloop, std::chrono::milliseconds(500)))
    {
        messageloop.post([]
        {
            if (backend_ptr)
            {
                auto addresses = backend_ptr->bound_addresses();
                auto port = backend_ptr->bound_port();

                auto address = addresses.find("127.0.0.1");
                if (address == addresses.end())
                    address = addresses.begin();

                if (port && (address != addresses.end()))
                    open_url("http://" + *address + ":" + std::to_string(port) + "/");
            }
        });

        return run_server(messageloop);
    }

    return 0;
}

#if defined(__unix__)
#include <unistd.h>

static bool is_root()
{
    return (getuid() == 0) || (getgid() == 0);
}

static bool open_url(const std::string &location)
{
    return system(("xdg-open " + location).c_str()) == 0;
}

#elif defined(WIN32)

static bool is_root()
{
    return false;
}

static bool open_url(const std::string &location)
{
    return system(("start " + location).c_str()) == 0;
}
#endif
