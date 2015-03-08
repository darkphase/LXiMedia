#include "platform/inifile.h"
#include "platform/messageloop.h"
#include "platform/path.h"
#include "platform/process.h"
#include "platform/string.h"
#include "pupnp/client.h"
#include "pupnp/upnp.h"
#include "server/server.h"
#include "server/settings.h"
#include "vlc/instance.h"
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <thread>

static FILE * logfile_open(const std::string &location);
static bool open_url(const std::string &location);
static bool is_root();
static void detach();
#if defined(__APPLE__)
static void register_agent(const char *argv0, const char *);
#endif

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

    std::clog << "starting LXiMediaServer version " << VERSION << std::endl;

    class platform::messageloop_ref messageloop_ref(messageloop);

    class platform::inifile media_cache_file(
                platform::config_dir() + "/media_cache", false);

    class platform::timer save_media_cache_timer(
                messageloop_ref,
                std::bind(&platform::inifile::save, &media_cache_file));
    media_cache_file.on_touched = [&save_media_cache_timer]
    {
        save_media_cache_timer.start(std::chrono::seconds(5), true);
    };

    class platform::inifile watchlist_file(
                platform::config_dir() + "/watchlist", false);

    class platform::timer save_watchlist_timer(
                messageloop_ref,
                std::bind(&platform::inifile::save, &watchlist_file));
    watchlist_file.on_touched = [&save_watchlist_timer]
    {
        save_watchlist_timer.start(std::chrono::seconds(1), true);
    };

    server::recreate_server = [
            &messageloop, &messageloop_ref,
            &media_cache_file, &watchlist_file,
            &upnp, &settings, &logfile]
    {
        server_ptr = nullptr;
        server_ptr.reset(new class server(
                             messageloop_ref, settings, upnp, logfile,
                             media_cache_file, watchlist_file));

        if (!server_ptr->initialize())
        {
            std::clog << "failed to initialize server; stopping." << std::endl;
            messageloop.stop(1);
        }
    };

    upnp.close();
    server::recreate_server();

    for (auto &i : upnp.bound_addresses())
        std::clog << "bound address: " << i << ":" << upnp.bound_port() << std::endl;

    // If the last exit was not clean, the server is re-initialized immediately
    // to make sure all cleints get a bye-bye message followed by an alive
    // message.
    if (!settings.was_clean_exit())
    {
        platform::timer::single_shot(
                    messageloop_ref,
                    std::chrono::seconds(1),
                    server::recreate_server);
    }

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

    platform::process::process_entry(argc, argv);

    if (argc == 1)
        detach();

    vlc::instance::initialize(argc, argv);

    class platform::messageloop messageloop;
    class platform::messageloop_ref messageloop_ref(messageloop);
    std::unique_ptr<class settings> settings(
                new class settings(messageloop_ref, true));

    class pupnp::upnp upnp(messageloop_ref);
    if (upnp.initialize(0, false))
    {
        const auto urls = find_server(messageloop, *settings, upnp);

        for (int i = 1; i < argc; i++)
            if (strcmp(argv[i], "--run") == 0)
            {
                if (urls.empty())
                {
                    settings = nullptr;
                    settings.reset(new class settings(messageloop_ref, false));
                    return run_server(messageloop, *settings, upnp, false);
                }
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

                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
                else
                    std::cerr << "LXiMediaServer not running." << std::endl;

                return 0;
            }
            else if (strcmp(argv[i], "--probeplugins") == 0)
            {
                std::vector<std::string> vlc_options;
                vlc_options.push_back("--reset-plugins-cache");
                vlc::instance instance(vlc_options);

                return 0;
            }

        // If one found, open in browser.
        if (!urls.empty())
            return open_url(get_url(urls)) ? 0 : 1;

        // Otherwise start and open this one in browser.
        settings = nullptr;

#if defined(__APPLE__)
        register_agent(argv[0], "net.sf.lximediaserver");
#endif

        settings.reset(new class settings(messageloop_ref, false));
        return run_server(messageloop, *settings, upnp, true);
    }

    return 1;
}

#if defined(__unix__) || defined(__APPLE__)
#include <sys/stat.h>
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

    ::fclose(::stdout);
    return ::freopen(location.c_str(), "w", ::stderr);
}

static bool is_root()
{
    return (getuid() == 0) || (getgid() == 0);
}

static bool open_url(const std::string &location)
{
#if defined(__unix__)
    return system(("xdg-open " + location).c_str()) == 0;
#elif defined(__APPLE__)
    return system(("open " + location).c_str()) == 0;
#endif
}

static void detach()
{
    // Spawn child process
    ::fflush(::stdout);
    ::fflush(::stderr);

    const auto child = fork();
    if (child >= 0)
    {
        if (child == 0)
        {   // Child process
            return;
        }
        else
        {   // Parent process
            ::_exit(0);
        }
    }
}

#if defined(__APPLE__)
static void register_agent(const char *argv0, const char *name)
{
    const std::string home = platform::home_dir();
    if (!home.empty())
    {
        const auto launch_agents = home + "/Library/LaunchAgents/";
        mkdir(launch_agents.c_str(), S_IRWXU);

        std::ofstream file(launch_agents + name + ".plist");
        if (file.is_open())
        {
            file << "<plist version=\"1.0\">\n"
                 << " <dict>\n"
                 << "  <key>Label</key>\n"
                 << "  <string>" << escape_xml(name) << ".agent</string>\n"
                 << "  <key>RunAtLoad</key>\n"
                 << "  <true/>\n"
                 << "  <key>ProgramArguments</key>\n"
                 << "  <array>\n"
                 << "   <string>" << escape_xml(argv0) << "</string>\n"
                 << "   <string>--run</string>\n"
                 << "  </array>\n"
                 << " </dict>\n"
                 << "</plist>\n";
        }
    }
}
#endif

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

static void detach()
{
    // Get executable
    wchar_t exec[MAX_PATH + 1] = { 0 };
    if (::GetModuleFileName(::GetModuleHandle(NULL), exec, MAX_PATH) == 0)
        throw std::runtime_error("Failed to determine executable.");

    // Spawn child process
    const auto child = _wspawnl(
                P_NOWAIT,
                exec,
                exec,
                L"--detached",
                NULL);

    if (child != -1)
        ::_exit(0);
}

static bool open_url(const std::string &location)
{
    return int(ShellExecute(NULL, L"open", to_utf16(location).c_str(), NULL, NULL, SW_SHOWNORMAL)) > 32;
}
#endif
