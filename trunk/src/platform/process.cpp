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

namespace platform {

class process::pipe_streambuf : public std::streambuf
{
public:
    explicit pipe_streambuf(int fd);
    ~pipe_streambuf();

    int underflow() override;
    int overflow(int value) override;
    int sync() override;

private:
    const int fd;
    char buffer[4096];
};

process::pipe_streambuf::pipe_streambuf(int fd)
    : fd(fd)
{
}

process::pipe_streambuf::~pipe_streambuf()
{
    ::close(fd);
}

int process::pipe_streambuf::underflow()
{
    if ((gptr() != nullptr) && (gptr() < egptr())) // buffer not exhausted
        return traits_type::to_int_type(*this->gptr());

    const int rc = ::read(fd, &buffer[0], sizeof(buffer));
    if (rc > 0)
    {
        setg(&buffer[0], &buffer[0], &buffer[rc]);

        return traits_type::to_int_type(*gptr());
    }

    return traits_type::eof();
}

int process::pipe_streambuf::overflow(int value)
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

int process::pipe_streambuf::sync()
{
    int result = overflow(traits_type::eof());
    ::fsync(fd);

    return traits_type::eq_int_type(result, traits_type::eof()) ? -1 : 0;
}

} // End of namespace

#if defined(__unix__) || defined(__APPLE__)
#include <sys/wait.h>
#include <stdexcept>
#ifdef __linux__
# include <sys/syscall.h>
#endif

namespace platform {

process::process(
        const std::function<void(std::ostream &)> &function,
        bool background_task)
    : child(0)
{
    if (pipe(pipe_desc) != 0)
        throw std::runtime_error("Creating pipe failed");

    child = fork();
    if (child >= 0)
    {
        if (child == 0)
        {   // Child process
            if (background_task)
            {
                nice(5);
#ifdef __linux__
                syscall(SYS_ioprio_set, 1, getpid(), 0x6007);
#endif
            }

            close(pipe_desc[0]);
            {
                pipe_streambuf streambuf(pipe_desc[1]);
                std::ostream stream(&streambuf);

                function(stream);
            }

            _exit(0);
        }
        else
        {   // Parent process
            close(pipe_desc[1]);

            std::istream::rdbuf(new pipe_streambuf(pipe_desc[0]));
        }
    }
    else
        throw std::runtime_error("Failed to fork process.");
}

process::process(process &&from)
    : child(from.child)
{
    std::istream::rdbuf(from.rdbuf(nullptr));
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

namespace platform {

process::process(
        const std::function<void(std::ostream &)> &function,
        bool background_task)
    : thread()
{
    if (_pipe(pipe_desc, 4096, _O_BINARY) != 0)
        throw std::runtime_error("Creating pipe failed");

    thread = std::thread([this, function]
    {
        pipe_streambuf streambuf(pipe_desc[1]);
        std::ostream stream(&streambuf);

        function(stream);
    });

    std::istream::rdbuf(new pipe_streambuf(pipe_desc[0]));
}

process::process(process &&from)
    : thread(std::move(from.thread))
{
    std::istream::rdbuf(from.rdbuf(nullptr));
}

process & process::operator=(process &&from)
{
    delete std::istream::rdbuf(from.rdbuf(nullptr));

    thread = std::move(from.thread);

    return *this;
}

process::~process()
{
    delete std::istream::rdbuf(nullptr);
}

void process::join()
{
    thread.join();
}

} // End of namespace
#endif
