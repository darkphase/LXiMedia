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

#include "process.h"

#ifndef PLATFORM_PROCESS_PIPE_STREAMBUF_CPP
#define PLATFORM_PROCESS_PIPE_STREAMBUF_CPP

#if defined(__unix__) || defined(__APPLE__)
# include <unistd.h>
#elif defined(WIN32)
# include <io.h>
# include <windows.h>
#endif

namespace {

class pipe_streambuf : public std::streambuf
{
public:
    explicit pipe_streambuf(int fd);
    ~pipe_streambuf();

    int underflow() override;
    int overflow(int value) override;
    int sync() override;

private:
    const int fd;
    static const size_t putback = 8;
    char buffer[4096];
};

pipe_streambuf::pipe_streambuf(int fd)
    : fd(fd)
{
}

pipe_streambuf::~pipe_streambuf()
{
    ::close(fd);
}

int pipe_streambuf::underflow()
{
    if ((gptr() != nullptr) && (gptr() < egptr())) // buffer not exhausted
        return traits_type::to_int_type(*this->gptr());

    const int rc = ::read(fd, &buffer[putback], sizeof(buffer) - putback);
    if ((rc > 0) && (rc <= int(sizeof(buffer) - putback)))
    {
        setg(&buffer[0], &buffer[putback], &buffer[putback] + rc);

        return traits_type::to_int_type(*gptr());
    }

    return traits_type::eof();
}

int pipe_streambuf::overflow(int value)
{
    const int size = pptr() - pbase();
    for (int pos = 0; pos < size; )
    {
        const int rc = ::write(fd, &buffer[pos], size);
        if (rc > 0)
            pos += size;
        else
            return traits_type::eof();
    }

    this->setp(&buffer[0], &buffer[sizeof(buffer)]);
    if (!traits_type::eq_int_type(value, traits_type::eof()))
        this->sputc(value);

    return traits_type::not_eof(value);
}

int pipe_streambuf::sync()
{
    int result = overflow(traits_type::eof());
#if defined(WIN32)
    ::FlushFileBuffers(HANDLE(_get_osfhandle(fd)));
#else
    ::fsync(fd);
#endif

    return traits_type::eq_int_type(result, traits_type::eof()) ? -1 : 0;
}

}
#endif

namespace platform {

process::process(
        const std::function<void(process &, std::ostream &)> &function,
        priority priority_)
    : process([function](class process &process, int fd)
        {
            pipe_streambuf streambuf(fd);
            std::ostream stream(&streambuf);

            function(process, stream);
        }, priority_)
{
}

} // End of namespace

#ifndef PROCESS_USES_THREAD
#include <cstdio>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdexcept>
#ifdef __linux__
# include <sys/prctl.h>
# include <sys/syscall.h>
#endif

namespace {

volatile bool term_received = false;

void signal_handler(int /*signal*/)
{
    term_received = true;
}

}

namespace platform {

void * process::map(size_t size)
{
    return ::mmap(
                nullptr, size,
                PROT_READ | PROT_WRITE,
#if !defined(__APPLE__)
                MAP_SHARED | MAP_ANONYMOUS,
#else
                MAP_SHARED | MAP_ANON,
#endif
                -1, 0);
}

void process::unmap(void *addr, size_t size)
{
    ::munmap(addr, size);
}

process::process()
    : std::istream(nullptr),
      child(0)
{
}

process::process(
        const std::function<void(process &, int)> &function,
        priority priority_)
    : std::istream(nullptr),
      child(0)
{
    if (pipe(pipe_desc) != 0)
        throw std::runtime_error("Creating pipe failed");

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

            case priority::idle:
#ifdef __linux__
                ::syscall(SYS_ioprio_set, 1, getpid(), 0x6007);
#endif
                ::nice(15);
                break;
            }

            ::close(pipe_desc[0]);
            function(*this, pipe_desc[1]);

            ::fflush(::stdout);
            ::fflush(::stderr);

            ::_exit(0);
        }
        else
        {   // Parent process
            ::close(pipe_desc[1]);

            std::istream::rdbuf(new pipe_streambuf(pipe_desc[0]));
        }
    }
    else
        throw std::runtime_error("Failed to fork process.");
}

process::process(process &&from)
    : std::istream(from.rdbuf(nullptr)),
      child(from.child)
{
    from.child = 0;
}

process & process::operator=(process &&from)
{
    delete std::istream::rdbuf(from.rdbuf(nullptr));

    if (child != 0)
        throw std::runtime_error("Process still running while destructing.");

    child = from.child;
    from.child = 0;

    return *this;
}

process::~process()
{
    delete std::istream::rdbuf(nullptr);

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

void process::join()
{
    if (child != 0)
    {
        int stat_loc = 0;
        while (waitpid(child, &stat_loc, 0) != child)
            continue;

        child = 0;
    }
    else
        throw std::runtime_error("Process not joinable.");
}

} // End of namespace
#else // PROCESS_USES_THREAD
#include <fcntl.h>

namespace platform {

void * process::map(size_t size)
{
    return new char[size];
}

void process::unmap(void *addr, size_t)
{
    delete [] reinterpret_cast<char *>(addr);
}

process::process()
    : std::istream(nullptr),
      thread(),
      term_received(false)
{
}

process::process(
        const std::function<void(process &, int)> &function,
        priority priority_)
    : std::istream(nullptr),
      thread(),
      term_received(false)
{
#if defined(WIN32)
    if (_pipe(pipe_desc, 4096, _O_BINARY) != 0)
#else
    if (pipe(pipe_desc) != 0)
#endif
    {
        throw std::runtime_error("Creating pipe failed");
    }

    thread = std::thread([this, function, priority_]
    {
        switch (priority_)
        {
        case priority::normal:
            break;

        case priority::low:
#if defined(WIN32)
            ::SetThreadPriority(::GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
            ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_LOWEST);
#endif
            break;

        case priority::idle:
#if defined(WIN32)
            ::SetThreadPriority(::GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
            ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_IDLE);
#endif
            break;
        }


        function(*this, pipe_desc[1]);
    });

    std::istream::rdbuf(new pipe_streambuf(pipe_desc[0]));
}

process::process(process &&from)
    : std::istream(from.rdbuf(nullptr)),
      thread(std::move(from.thread)),
      term_received(from.term_received)
{
}

process & process::operator=(process &&from)
{
    delete std::istream::rdbuf(from.rdbuf(nullptr));

    thread = std::move(from.thread);
    term_received = from.term_received;

    return *this;
}

process::~process()
{
    delete std::istream::rdbuf(nullptr);
}

void process::send_term()
{
    term_received = true;
}

bool process::term_pending() const
{
    return term_received;
}

bool process::joinable() const
{
    return thread.joinable();
}

void process::join()
{
    thread.join();
}

} // End of namespace
#endif
