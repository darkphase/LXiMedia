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

#include "fork.h"

#if defined(__unix__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdexcept>
#ifdef __linux__
# include <sys/syscall.h>
#endif

namespace platform {

std::string run_forked(const std::function<std::string()> &function, bool background_task)
{
    int filedes[2];
    if (pipe(filedes) != 0)
        throw std::runtime_error("Creating pipe failed");

    pid_t child = fork();
    if (child >= 0)
    {
        if (child == 0)
        {   // Child process
            close(filedes[0]);

            if (background_task)
            {
                nice(5);
#ifdef __linux__
                syscall(SYS_ioprio_set, 1, getpid(), 0x6007);
#endif
            }

            const auto result = function();
            for (size_t i = 0; i < result.size(); i++)
            {
                const auto rc = write(filedes[1], &result[i], result.size() - i);
                if (rc > 0)
                    i += rc;
                else
                    break;
            }

            close(filedes[1]);
            _exit(0);
        }
        else
        {   // Parent process
            close(filedes[1]);

            std::string result, buffer;
            for (;;)
            {
                buffer.resize(4096);
                const auto rc = read(filedes[0], &buffer[0], buffer.size());
                if (rc > 0)
                {
                    buffer.resize(rc);
                    result += buffer;
                }
                else
                    break;
            }

            close(filedes[0]);
            int stat_loc = 0;
            while (waitpid(child, &stat_loc, 0) != child)
                continue;

            return (WEXITSTATUS(stat_loc) == 0) ? result : std::string();
        }
    }
    else
    {
        close(filedes[0]);
        close(filedes[1]);
        throw std::runtime_error("Failed to fork process.");
    }
}

} // End of namespace
#elif defined(WIN32)
namespace platform {

std::string run_forked(const std::function<std::string()> &function, bool)
{
    return function();
}

} // End of namespace
#endif
