#include "server.h"
#include "settings.h"
#include "platform/messageloop.h"
#include "platform/path.h"
#include "platform/string.h"
#include "pupnp/client.h"
#include "pupnp/upnp.h"
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <set>

static FILE * logfile_open(const std::string &location);
static bool open_url(const std::string &location);
static bool is_root();
static std::unique_ptr<class server> server_ptr;

static std::set<std::string> find_server(
        class platform::messageloop &messageloop,
        class settings &settings,
        class pupnp::upnp &upnp)
{
    const std::string my_id = "uuid:" + std::string(settings.uuid());
    class platform::messageloop_ref messageloop_ref(messageloop);
    class pupnp::client client(messageloop_ref, upnp);

    std::set<std::string> result;
    if (client.initialize())
    {
        client.device_discovered = [&messageloop, &upnp, &client, &result, &my_id](const std::string &device_id, const std::string &location)
        {
            if ((device_id == my_id) && starts_with(location, "http://"))
            {
                const size_t sl = location.find_first_of(":/", 7);
                if ((sl != location.npos) && upnp.is_my_address(location.substr(7, sl - 7)))
                {
                    struct pupnp::client::device_description device_description;
                    if (client.read_device_description(location, device_description))
                        result.insert(device_description.presentation_url);
                }
            }
        };

        client.start_search("urn:schemas-upnp-org:device:MediaServer:1");
        messageloop.process_events(std::chrono::milliseconds(500));
        client.close();
    }

    return result;
}

static int run_server(
        class platform::messageloop &messageloop,
        class settings &settings,
        class pupnp::upnp &upnp,
        bool open_browser)
{
    const std::string logfile = platform::config_dir() + "/lximediaserver.log";
    FILE * logfilehandle = logfile_open(logfile);

    std::clog << "Starting LXiMediaServer version " << VERSION << std::endl;

    class platform::messageloop_ref messageloop_ref(messageloop);

    server::recreate_server = [&messageloop, &messageloop_ref, &upnp, &settings, &logfile]
    {
        server_ptr = nullptr;
        server_ptr.reset(new class server(messageloop_ref, settings, upnp, logfile));
        if (!server_ptr->initialize())
        {
            std::clog << "[" << server_ptr.get() << "] failed to initialize server; stopping." << std::endl;
            messageloop.stop(1);
        }
    };

    server::recreate_server();
    if (open_browser)
    {
        auto addresses = server_ptr->bound_addresses();
        auto port = server_ptr->bound_port();

        auto address = addresses.find("127.0.0.1");
        if (address == addresses.end())
            address = addresses.begin();

        if (port && (address != addresses.end()))
            open_url("http://" + *address + ":" + std::to_string(port) + "/");
    }

    const int result = messageloop.run();

    server::recreate_server = nullptr;
    server_ptr = nullptr;
    fclose(logfilehandle);
    return result;
}

static std::string get_url(const std::set<std::string> &urls)
{
    for (auto &i : urls)
        if (i.find("127.0.0.1") != i.npos)
            return i;

    if (!urls.empty())
        return *urls.begin();

    return std::string();
}

int main(int argc, const char *argv[])
{
    if (is_root())
    {
        std::cerr << "Cannot run as root." << std::endl;
        return 1;
    }

    setlocale(LC_ALL, "");

    class platform::messageloop messageloop;
    class platform::messageloop_ref messageloop_ref(messageloop);
    class settings settings(messageloop_ref);

    class pupnp::upnp upnp(messageloop_ref);
    if (upnp.initialize(settings.http_port(), false))
    {
        const auto urls = find_server(messageloop, settings, upnp);

        for (int i = 1; i < argc; i++)
            if (strcmp(argv[i], "--run") == 0)
            {
                if (urls.empty())
                    return run_server(messageloop, settings, upnp, false);
                else
                    std::cerr << "LXiMediaServer already running." << std::endl;

                return 0;
            }
            else if (strcmp(argv[i], "--quit") == 0)
            {
                if (!urls.empty())
                {
                    class pupnp::client client(messageloop_ref, upnp);

                    client.get(get_url(urls) + "quit");
                }
                else
                    std::cerr << "LXiMediaServer not running." << std::endl;

                return 0;
            }

        // If one found, open in browser.
        if (!urls.empty())
            return open_url(get_url(urls)) ? 0 : 1;

        // Otherwise start and open this one in browser.
        return run_server(messageloop, settings, upnp, true);
    }

    return 1;
}

#if defined(__unix__)
#include <unistd.h>

static std::string log_location(const std::string &location, int i)
{
    return (i == 0) ? location : (location + '.' + std::to_string(i));
}

static FILE * logfile_open(const std::string &location)
{
    // Rotate logs
    std::remove(log_location(location, 5).c_str());
    for (int i = 5; i > 0; i--)
        std::rename(log_location(location, i - 1).c_str(), log_location(location, i).c_str());

    return freopen(location.c_str(), "w", stderr);
}

static bool is_root()
{
    return (getuid() == 0) || (getgid() == 0);
}

static bool open_url(const std::string &location)
{
    return system(("xdg-open " + location).c_str()) == 0;
}

#elif defined(WIN32)
#include <windows.h>
#include <shellapi.h>

static std::wstring log_location(const std::string &location, int i)
{
    return platform::to_windows_path((i == 0) ? location : (location + '.' + std::to_string(i)));
}

static FILE * logfile_open(const std::string &location)
{
    // Rotate logs
    _wremove(log_location(location, 5).c_str());
    for (int i = 5; i > 0; i--)
        _wrename(log_location(location, i - 1).c_str(), log_location(location, i).c_str());

    return _wfreopen(platform::to_windows_path(location).c_str(), L"w", stderr);
}

static bool is_root()
{
    wchar_t buffer[64];
    DWORD bufferSize = sizeof(buffer) / sizeof(*buffer);

    if (GetUserName(buffer, &bufferSize))
        return _wcsicmp(buffer, L"SYSTEM") == 0;

    return false;
}

static bool open_url(const std::string &location)
{
    return int(ShellExecute(NULL, L"open", to_utf16(location).c_str(), NULL, NULL, SW_SHOWNORMAL)) > 32;
}
#endif
