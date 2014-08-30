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

#include "setup.h"
#include "platform/string.h"
#include "platform/translator.h"
#include <iostream>

static bool shutdown();

setup::setup(
        class messageloop &messageloop,
        pupnp::content_directory &content_directory,
        const class settings &settings)
    : messageloop(messageloop),
      content_directory(content_directory),
      settings(settings),
      basedir('/' + tr("Setup") + '/'),
      shutdown_pending(false)
{
    content_directory.item_source_register(basedir, *this);
}

setup::~setup()
{
    content_directory.item_source_unregister(basedir);
}

std::vector<pupnp::content_directory::item> setup::list_contentdir_items(
        const std::string &client,
        const std::string &,
        size_t start, size_t &count)
{
    const bool return_all = count == 0;

    std::vector<pupnp::content_directory::item> items;
    if (!shutdown_pending && settings.allow_shutdown())
        items.emplace_back(get_contentdir_item(client, basedir + "shutdown"));

    std::vector<pupnp::content_directory::item> result;
    for (size_t i=start, n=0; (i<items.size()) && (return_all || (n<count)); i++, n++)
        result.emplace_back(std::move(items[i]));

    count = items.size();

    return result;
}

pupnp::content_directory::item setup::get_contentdir_item(const std::string &, const std::string &path)
{
    pupnp::content_directory::item item;

    if (ends_with(path, "/shutdown"))
    {
        item.is_dir = false;
        item.path = path;
        item.mrl = "file:///";
        item.type = pupnp::content_directory::item_type::video_broadcast;
        item.title = tr("Shut down PC");

        item.sample_rate = 44100;
        item.channels = 2;
        item.width = 352;
        item.height = 288;
        item.frame_rate = 25.0f;
        item.duration = std::chrono::seconds(10);
    }

    return item;
}

void setup::correct_protocol(const pupnp::content_directory::item &, pupnp::connection_manager::protocol &)
{
}

int setup::play_item(
        const std::string &,
        const pupnp::content_directory::item &item,
        const std::string &,
        std::string &,
        std::shared_ptr<std::istream> &)
{
    if (ends_with(item.path, "/shutdown") && settings.allow_shutdown())
    {
        if (!shutdown_pending)
            shutdown_pending = shutdown();

        return pupnp::upnp::http_no_content;
    }

    return pupnp::upnp::http_not_found;
}

#if defined(__unix__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static bool shutdown()
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // This only works if "your_username ALL = NOPASSWD: /sbin/shutdown" is added to sudoers.
        execl("/usr/bin/sudo", "sudo", "shutdown", "-h", "now", nullptr);
        _exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        int exitcode;
        if (waitpid(pid, &exitcode, 0) != pid)
          exitcode = -1;

        if (exitcode == 0)
            return true;

        std::clog << "\"sudo shutdown -h now\" failed with exit code " << exitcode << '.' << std::endl;
    }
    else
        std::clog << "Failed to fork process." << std::endl;

    return false;
}
#elif defined(WIN32)
#include <windows.h>
#ifndef SHTDN_REASON_FLAG_PLANNED
# define SHTDN_REASON_FLAG_PLANNED 0x80000000
#endif

static bool shutdown()
{
    HANDLE token;
    ::OpenProcessToken(
                ::GetCurrentProcess(),
                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                &token);

    TOKEN_PRIVILEGES privileges;
    memset(&privileges, 0, sizeof(privileges));
    ::LookupPrivilegeValue(
                NULL,
                SE_SHUTDOWN_NAME,
                &privileges.Privileges[0].Luid);

    privileges.PrivilegeCount = 1;
    privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    ::AdjustTokenPrivileges(
                token,
                FALSE,
                &privileges,
                0, NULL, 0);

    static const wchar_t message[] = L"Shutdown requested by LXiMediaCenter.";

    bool result = false;
    if (::GetLastError() != ERROR_SUCCESS)
    {
        std::clog << "Failed to enable shutdown privilege." << std::endl;
    }
    else if (::InitiateSystemShutdownW(NULL, const_cast<wchar_t *>(message), 30, TRUE, FALSE) == FALSE)
    {
        std::clog << "InitiateSystemShutdown failed" << ::GetLastError() << " trying WTSShutdownSystem." << std::endl;

        HMODULE lib = ::LoadLibraryW(L"wtsapi32.dll");
        if (lib)
        {
            FARPROC proc = ::GetProcAddress(lib, "WTSShutdownSystem");
            if (proc == NULL)
                std::clog << "Failed to find WTSShutdownSystem in wtsapi32.dll." << std::endl;
            else if (((BOOL (WINAPI *)(HANDLE, DWORD))proc)(NULL, 0x00000008) == FALSE)
                std::clog << "WTSShutdownSystem failed." << std::endl;
            else
                result = true;

            ::FreeLibrary(lib);
        }
        else
            std::clog << "Failed to load wtsapi32.dll." << std::endl;
    }
    else
        result = true;

    return result;
}
#endif
