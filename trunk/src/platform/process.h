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

#ifndef PLATFORM_PROCESS_H
#define PLATFORM_PROCESS_H

// For debugging:
#ifndef NDEBUG
# define PROCESS_USE_THREAD
#endif

#include <cstdint>
#include <iostream>
#include <memory>
#if defined(PROCESS_USE_THREAD)
# include <thread>
#else
# include <sys/types.h>
#endif

namespace platform {

class process : public std::iostream
{
public:
    enum class priority { normal, low };
    class function_handle { friend class process; const char *name; };
    typedef int(* function)(process &);

public:
    static function_handle register_function(const char *, function);
#define register_function(F) register_function(#F, F)

    static void process_entry(int argc, const char *argv[]);
    static unsigned hardware_concurrency();

    process(function_handle, priority = priority::normal);
    ~process();

    process(const process &) = delete;
    process & operator=(const process &) = delete;
    process(process &&) = delete;
    process & operator=(process &&) = delete;

    operator bool() const;

    template <typename _Type>
    unsigned alloc_shared();
    template <typename _Type>
    volatile _Type & get_shared(unsigned off);
    template <typename _Type>
    const volatile _Type & get_shared(unsigned off) const;

    int input_fd();
    int output_fd();

    void send_term();
    bool term_pending() const;

    bool joinable() const;
    int join();

private:
    struct shm_data;

#if defined(PROCESS_USE_THREAD)
    process(volatile shm_data *, int, int);
#elif defined(WIN32)
    process(priority, void *, int, int);
#endif

    unsigned alloc_shared(size_t);
    volatile void *get_shared(unsigned, size_t size);
    const volatile void *get_shared(unsigned, size_t size) const;

private:
#if defined(PROCESS_USE_THREAD)
    std::unique_ptr<std::thread> thread;
    int exit_code;
#elif defined(__unix__) || defined(__APPLE__)
    pid_t child;
#elif defined(WIN32)
    intptr_t child;
    void *file_mapping;
#endif
    volatile shm_data *shm;
};

template <typename _Type>
unsigned process::alloc_shared()
{
    return alloc_shared(sizeof(_Type));
}

template <typename _Type>
volatile _Type & process::get_shared(unsigned off)
{
    return *reinterpret_cast<volatile _Type *>(
                get_shared(off, sizeof(_Type)));
}

template <typename _Type>
const volatile _Type & process::get_shared(unsigned off) const
{
    return *reinterpret_cast<const volatile _Type *>(
                get_shared(off, sizeof(_Type)));
}

} // End of namespace

#endif
