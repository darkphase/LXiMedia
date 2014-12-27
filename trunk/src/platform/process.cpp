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

#if defined(__unix__) || defined(__APPLE__)
# include <unistd.h>
#elif defined(WIN32)
# include <io.h>
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

}

namespace platform {

process::process(
        const std::function<void(process &, std::ostream &)> &function,
        bool background_task)
    : process([function](class process &process, int fd)
        {
            pipe_streambuf streambuf(fd);
            std::ostream stream(&streambuf);

            function(process, stream);
        }, background_task)
{
}

void process::close()
{
    delete std::istream::rdbuf(nullptr);
}

} // End of namespace

#if defined(__unix__) || defined(__APPLE__)
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

int pipe_streambuf::sync()
{
    int result = overflow(traits_type::eof());
    ::fsync(fd);

    return traits_type::eq_int_type(result, traits_type::eof()) ? -1 : 0;
}

}

namespace platform {

process::process()
    : std::istream(nullptr),
      child(0)
{
}

process::process(
        const std::function<void(process &, int)> &function,
        bool background_task)
    : std::istream(nullptr),
      child(0)
{
    if (pipe(pipe_desc) != 0)
        throw std::runtime_error("Creating pipe failed");

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

            if (background_task)
            {
                ::nice(5);
#ifdef __linux__
                ::syscall(SYS_ioprio_set, 1, getpid(), 0x6007);
#endif
            }

            ::close(pipe_desc[0]);
            function(*this, pipe_desc[1]);
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
#elif defined(WIN32)
#include <fcntl.h>
#include <windows.h>

namespace {

int pipe_streambuf::sync()
{
    int result = overflow(traits_type::eof());
    ::FlushFileBuffers(HANDLE(_get_osfhandle(fd)));

    return traits_type::eq_int_type(result, traits_type::eof()) ? -1 : 0;
}

}

namespace platform {

process::process()
    : std::istream(nullptr),
      thread(),
      term_received(false)
{
}

process::process(
        const std::function<void(process &, int)> &function,
        bool background_task)
    : std::istream(nullptr),
      thread(),
      term_received(false)
{
    if (_pipe(pipe_desc, 4096, _O_BINARY) != 0)
        throw std::runtime_error("Creating pipe failed");

    thread = std::thread([this, function]
    {
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
