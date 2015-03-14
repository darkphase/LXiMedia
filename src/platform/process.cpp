/******************************************************************************
 *   Copyright (C) 2015  A.J. Admiraal                                        *
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

#include "process.h"
#undef register_function
#include "platform/string.h"
#include "platform/uuid.h"
#include <cassert>
#include <cstring>
#include <functional>
#include <map>
#include <stdexcept>
#include <thread>

#if defined(__unix__) || defined(__APPLE__)
# include <unistd.h>
#elif defined(WIN32)
# include <io.h>
#endif

namespace {

struct pipe_streambuf : public std::streambuf
{
    explicit pipe_streambuf(int ifd, int ofd);
    ~pipe_streambuf();

    int underflow() override;
    int overflow(int value) override;
    int sync() override;

    const int ifd, ofd;
    static const unsigned putback = 8;
    char ibuffer[65536];
    char obuffer[4096];
};

pipe_streambuf::pipe_streambuf(int ifd, int ofd)
    : ifd(ifd), ofd(ofd)
{
}

pipe_streambuf::~pipe_streambuf()
{
#if !defined(WIN32)
    if (ifd) ::close(ifd);
    if (ofd) ::close(ofd);
#else
    if (ifd) ::_close(ifd);
    if (ofd) ::_close(ofd);
#endif
}

int pipe_streambuf::underflow()
{
    if ((gptr() != nullptr) && (gptr() < egptr())) // buffer not exhausted
        return traits_type::to_int_type(*this->gptr());

#if !defined(WIN32)
    const int rc = ::read(ifd, &ibuffer[putback], sizeof(ibuffer) - putback);
#else
    const int rc = ::_read(ifd, &ibuffer[putback], sizeof(ibuffer) - putback);
#endif
    if ((rc > 0) && (rc <= int(sizeof(ibuffer) - putback)))
    {
        setg(&ibuffer[0], &ibuffer[putback], &ibuffer[putback] + rc);

        return traits_type::to_int_type(*gptr());
    }

    return traits_type::eof();
}

int pipe_streambuf::overflow(int value)
{
    const int size = pptr() - pbase();
    for (int pos = 0; pos < size; )
    {
#if !defined(WIN32)
        const int rc = ::write(ofd, &obuffer[pos], size);
#else
        const int rc = ::_write(ofd, &obuffer[pos], size);
#endif
        if (rc > 0)
            pos += size;
        else
            return traits_type::eof();
    }

    this->setp(&obuffer[0], &obuffer[sizeof(obuffer)]);
    if (!traits_type::eq_int_type(value, traits_type::eof()))
        this->sputc(value);

    return traits_type::not_eof(value);
}

int pipe_streambuf::sync()
{
    return traits_type::eq_int_type(
                overflow(traits_type::eof()),
                traits_type::eof()) ? -1 : 0;
}

struct strcmp_less
{
    bool operator()(const char *a, const char *b) const
    {
        return strcmp(a, b) < 0;
    }
};

std::map<const char *, platform::process::function, strcmp_less> &functions()
{
    static std::map<const char *, platform::process::function, strcmp_less> f;
    return f;
}

}

namespace platform {

struct process::shm_data
{
    uint64_t size; // data should be 8-byte aligned.
    uint8_t data[4088];
};

process::function_handle process::register_function(
        const char *name,
        function func)
{
    function_handle handle;
    handle.name = name;
    while (*handle.name == '&')
        handle.name++;

    functions()[handle.name] = func;
    return handle;
}

process::operator bool() const
{
    return !term_pending() && *static_cast<const std::iostream *>(this);
}

int process::input_fd()
{
    return static_cast<pipe_streambuf *>(std::iostream::rdbuf())->ifd;
}

int process::output_fd()
{
    return static_cast<pipe_streambuf *>(std::iostream::rdbuf())->ofd;
}

unsigned process::alloc_shared(size_t size)
{
    static_assert(sizeof(shm_data) == 4096, "sizeof(shm_data) != 4096");

    if (size <= sizeof(shm->data))
    {
        // Make sure the pointer is 8-byte aligned.
        const uint64_t u64_size = uint64_t(size + 7) & ~uint64_t(7);

        uint64_t after_size = __sync_add_and_fetch(&shm->size, u64_size);
        if (after_size <= sizeof(shm->data))
            return after_size - u64_size;

        __sync_sub_and_fetch(&shm->size, u64_size);
    }

    return unsigned(-1);
}

volatile void * process::get_shared(unsigned offset, size_t size)
{
    if ((size + offset) < sizeof(shm->data))
    {
        // Check if the pointer is 8-byte aligned.
        assert((uintptr_t(shm->data + offset) & 7) == 0);

        return shm->data + offset;
    }

    throw std::runtime_error("Invalid offset passed to get_shared");
}

const volatile void * process::get_shared(unsigned offset, size_t size) const
{
    if ((size + offset) < sizeof(shm->data))
        return shm->data + offset;

    throw std::runtime_error("Invalid offset passed to get_shared");
}

} // End of namespace

#if defined(PROCESS_USE_THREAD)
#if defined(WIN32)
#  include <fcntl.h>
#endif

namespace platform {

void process::process_entry(int, const char *[])
{
}

unsigned process::hardware_concurrency()
{
    return 1;
}

process::process(function_handle handle, priority priority_)
    : std::iostream(nullptr),
      thread(),
      exit_code(-1),
      shm(nullptr)
{
    const auto &functions = ::functions();
    auto function = functions.find(handle.name);
    if (function == functions.end())
        throw std::runtime_error("Failed to find child function");

    // Create pipes.
    int ipipe[2], opipe[2];
#if !defined(WIN32)
    if ((pipe(ipipe) != 0) || (pipe(opipe) != 0))
#else
    if ((_pipe(ipipe, sizeof(pipe_streambuf::obuffer), _O_BINARY) != 0) ||
        (_pipe(opipe, sizeof(pipe_streambuf::ibuffer), _O_BINARY) != 0))
#endif
    {
        throw std::runtime_error("Creating pipes failed");
    }

    // Create shared memory.
    shm = new shm_data();

    // Will get offset 0; used to emulate the term signal.
    alloc_shared<bool>();

    // Spawn thread
    thread.reset(new std::thread([this, function, ipipe, opipe]
    {
        class process process(shm, ipipe[0], opipe[1]);
        exit_code = function->second(process);
    }));

    std::iostream::rdbuf(new pipe_streambuf(opipe[0], ipipe[1]));
}

process::process(volatile shm_data *shm, int ifd, int ofd)
    : std::iostream(new pipe_streambuf(ifd, ofd)),
      thread(),
      exit_code(-1),
      shm(shm)
{
}

process::~process()
{
    if (thread != nullptr)
        delete shm;

    delete std::iostream::rdbuf(nullptr);
}

void process::send_term()
{
    if ((thread != nullptr) && shm)
        get_shared<bool>(0) = true;
}

bool process::term_pending() const
{
    if ((thread == nullptr) && shm)
        return get_shared<bool>(0);

    return false;
}

bool process::joinable() const
{
    return thread != nullptr;
}

int process::join()
{
    if (thread != nullptr)
    {
        thread->join();
        return exit_code;
    }
    else
        throw std::runtime_error("Process not joinable.");
}

} // End of namespace
#elif defined(__unix__) || defined(__APPLE__)
#include <cstdio>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdexcept>
#ifdef __linux__
# include <sys/prctl.h>
# include <sys/syscall.h>
#endif

namespace platform {

void process::process_entry(int, const char *[])
{
}

unsigned process::hardware_concurrency()
{
    return std::thread::hardware_concurrency();
}

static volatile bool term_received = false;
static void signal_handler(int /*signal*/)
{
    term_received = true;
}

process::process(function_handle handle, priority priority_)
    : std::iostream(nullptr),
      child(0),
      shm(nullptr)
{
    const auto &functions = ::functions();
    auto function = functions.find(handle.name);
    if (function == functions.end())
        throw std::runtime_error("Failed to find child function");

    // Create pipes.
    int ipipe[2], opipe[2];
    if ((pipe(ipipe) != 0) || (pipe(opipe) != 0))
        throw std::runtime_error("Creating pipes failed");

    // Create shared memory.
    shm = reinterpret_cast<volatile shm_data *>(
                ::mmap(
                    nullptr, sizeof(shm_data),
                    PROT_READ | PROT_WRITE,
#if !defined(__APPLE__)
                    MAP_SHARED | MAP_ANONYMOUS,
#else
                    MAP_SHARED | MAP_ANON,
#endif
                    -1, 0));

    if (shm == nullptr)
        throw std::runtime_error("Creating shared memory failed");

    // Spawn child process
    ::fflush(::stdout);
    ::fflush(::stderr);

    child = fork();
    if (child >= 0)
    {
        if (child == 0)
        {   // Child process
#ifdef __linux__
            // Kill this process when the parent dies.
            prctl(PR_SET_PDEATHSIG, SIGKILL);
#endif

            struct sigaction act;
            act.sa_handler = &signal_handler;
            sigemptyset(&act.sa_mask);
            act.sa_flags = 0;

            ::sigaction(SIGHUP, &act, nullptr);
            ::sigaction(SIGTERM, &act, nullptr);
            ::sigaction(SIGINT, &act, nullptr);

            switch (priority_)
            {
            case priority::normal:
                break;

            case priority::low:
#ifdef __linux__
                ::syscall(SYS_ioprio_set, 1, getpid(), 0x6007);
#endif
                ::nice(5);
                break;
            }

            ::close(ipipe[1]);
            ::close(opipe[0]);

            std::iostream::rdbuf(new pipe_streambuf(ipipe[0], opipe[1]));
            const int exit_code = function->second(*this);

            ::fflush(::stdout);
            ::fflush(::stderr);

            delete std::iostream::rdbuf(nullptr);

            ::_exit(exit_code);
        }
        else
        {   // Parent process
            ::close(ipipe[0]);
            ::close(opipe[1]);

            std::iostream::rdbuf(new pipe_streambuf(opipe[0], ipipe[1]));
        }
    }
    else
        throw std::runtime_error("Failed to fork process.");
}

process::~process()
{
    if (shm != nullptr)
        ::munmap(const_cast<shm_data *>(shm), sizeof(shm_data));

    delete std::iostream::rdbuf(nullptr);

    if (child != 0)
        throw std::runtime_error("Process still running while destructing.");
}

void process::send_term()
{
    if (child != 0)
        ::kill(child, SIGTERM);
    else
        throw std::runtime_error("Process not started.");
}

bool process::term_pending() const
{
    return term_received;
}

bool process::joinable() const
{
    return child != 0;
}

int process::join()
{
    if (child != 0)
    {
        int stat_loc = 0;
        while (waitpid(child, &stat_loc, 0) != child)
            continue;

        child = 0;
        return WEXITSTATUS(stat_loc);
    }
    else
        throw std::runtime_error("Process not joinable.");
}

} // End of namespace
#elif defined(WIN32)
#include <cstdlib>
#include <fcntl.h>
#include <windows.h>

namespace platform {

static const char child_process_arg[] = "start_function";

void process::process_entry(int argc, const char *argv[])
{
    for (int i = 1; (i + 5) < argc; i++)
        if (strcmp(argv[i], child_process_arg) == 0)
        {
            int exit_code = 1;

            const auto &functions = ::functions();
            auto function = functions.find(argv[i + 1]);
            if (function != functions.end())
            {
                // Suppress crash dialogs
                ::_set_error_mode(_OUT_TO_STDERR);
                ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

                class process process(
                            priority(std::stoi(argv[i + 2])),
                            HANDLE(std::stoi(argv[i + 3])),
                            std::stoi(argv[i + 4]),
                            std::stoi(argv[i + 5]));

                exit_code = function->second(process);
            }
            else
                throw std::runtime_error("Failed to find child function");

            ::exit(exit_code);
        }
}

unsigned process::hardware_concurrency()
{
    return std::thread::hardware_concurrency();
}

process::process(function_handle handle, priority priority_)
    : std::iostream(nullptr),
      child(0),
      file_mapping(nullptr),
      shm(nullptr)
{
    // Create pipes.
    int ipipe[2], opipe[2];
    if ((_pipe(ipipe, sizeof(pipe_streambuf::obuffer), _O_BINARY) != 0) ||
        (_pipe(opipe, sizeof(pipe_streambuf::ibuffer), _O_BINARY) != 0))
    {
        throw std::runtime_error("Creating pipes failed");
    }

    // Create shared memory.
    file_mapping = ::CreateFileMapping(
                INVALID_HANDLE_VALUE,
                NULL,
                PAGE_READWRITE,
                0, sizeof(shm_data),
                NULL);

    if ((file_mapping == NULL) || (file_mapping == INVALID_HANDLE_VALUE))
        throw std::runtime_error("Creating shared memory failed");

    shm = reinterpret_cast<shm_data *>(
                ::MapViewOfFile(
                    file_mapping,
                    FILE_MAP_WRITE,
                    0, 0,
                    sizeof(shm_data)));

    if (shm == nullptr)
        throw std::runtime_error("Creating shared memory failed");

    HANDLE duplicate_file_mapping = INVALID_HANDLE_VALUE;
    if (::DuplicateHandle(
                ::GetCurrentProcess(), file_mapping,
                ::GetCurrentProcess(), &duplicate_file_mapping,
                0, TRUE, DUPLICATE_SAME_ACCESS) == FALSE)
    {
        throw std::runtime_error("Duplicating shared memory handle failed");
    }

    // Will get offset 0; used to emulate the term signal.
    alloc_shared<bool>();

    // Get executable
    wchar_t exec[MAX_PATH + 1] = { 0 };
    if (::GetModuleFileName(::GetModuleHandle(NULL), exec, MAX_PATH) == 0)
        throw std::runtime_error("Failed to determine executable.");

    // Spawn child process
    child = _wspawnl(
                P_NOWAIT,
                exec,
                exec,
                to_utf16(child_process_arg).c_str(),
                to_utf16(handle.name).c_str(),
                std::to_wstring(int(priority_)).c_str(),
                std::to_wstring(int(duplicate_file_mapping)).c_str(),
                std::to_wstring(ipipe[0]).c_str(),
                std::to_wstring(opipe[1]).c_str(),
                NULL);

    if (child != -1)
    {
        ::CloseHandle(duplicate_file_mapping);
        ::close(ipipe[0]);
        ::close(opipe[1]);

        std::iostream::rdbuf(new pipe_streambuf(opipe[0], ipipe[1]));
    }
    else
        throw std::runtime_error("Failed to spawn process.");
}

process::process(priority priority_, void *file_mapping, int ifd, int ofd)
    : std::iostream(new pipe_streambuf(ifd, ofd)),
      child(0),
      file_mapping(file_mapping),
      shm(nullptr)
{
    // Set priority.
    switch (priority_)
    {
    case priority::normal:
        break;

    case priority::low:
        ::SetPriorityClass(
                    ::GetCurrentProcess(),
                    BELOW_NORMAL_PRIORITY_CLASS);
        ::SetPriorityClass(
                    ::GetCurrentProcess(),
                    PROCESS_MODE_BACKGROUND_BEGIN);
        break;
    }

    // Map shared memory.
    shm = reinterpret_cast<shm_data *>(
                ::MapViewOfFile(
                    file_mapping,
                    FILE_MAP_WRITE,
                    0, 0,
                    sizeof(shm_data)));

    if (shm == nullptr)
        throw std::runtime_error("Mapping shared memory failed");
}

process::~process()
{
    ::UnmapViewOfFile(const_cast<shm_data *>(shm));
    ::CloseHandle(file_mapping);

    delete std::iostream::rdbuf(nullptr);

    if (child != 0)
        throw std::runtime_error("Process still running while destructing.");
}

void process::send_term()
{
    if ((child != 0) && shm)
        get_shared<bool>(0) = true;
}

bool process::term_pending() const
{
    if ((child == 0) && shm)
        return get_shared<bool>(0);

    return false;
}

bool process::joinable() const
{
    return child != 0;
}

int process::join()
{
    if (child != 0)
    {
        int termstat = -1;
        while (_cwait(&termstat, child, WAIT_CHILD) != child)
            continue;

        child = 0;

        return termstat;
    }
    else
        throw std::runtime_error("Process not joinable.");
}

} // End of namespace
#endif
