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

#include "uuid.h"
#include <iomanip>
#include <sstream>
#include <cstring>

namespace platform {

uuid::uuid()
{
    memset(value, 0, sizeof(value));
}

bool uuid::operator==(const uuid &from) const
{
    return memcmp(value, from.value, sizeof(value)) == 0;
}

bool uuid::operator<(const uuid &from) const
{
    return memcmp(value, from.value, sizeof(value)) < 0;
}

bool uuid::is_null() const
{
    for (auto i : value)
        if (i != 0)
            return false;

    return true;
}

}

#if defined(__unix__)
#include <uuid/uuid.h>

namespace platform {

uuid uuid::generate()
{
    struct uuid result;
    uuid_generate(result.value);

    return result;
}

uuid::uuid(const std::string &str)
{
    if (uuid_parse(str.c_str(), value) != 0)
        memset(value, 0, sizeof(value));
}

uuid::operator std::string() const
{
    std::string result;
    result.resize(64);
    uuid_unparse(value, &result[0]);
    result.resize(strlen(&result[0]));

    return result;
}

}
#elif defined(WIN32)
#include <stdexcept>
#include <rpc.h>

namespace platform {

static struct uuid convert(const UUID &from)
{
    struct uuid uuid;

    uuid.value[0] = (from.Data1      ) & 0xFF;
    uuid.value[1] = (from.Data1 >>  8) & 0xFF;
    uuid.value[2] = (from.Data1 >> 16) & 0xFF;
    uuid.value[3] = (from.Data1 >> 24) & 0xFF;

    uuid.value[4] = (from.Data2      ) & 0xFF;
    uuid.value[5] = (from.Data2 >>  8) & 0xFF;

    uuid.value[6] = (from.Data3      ) & 0xFF;
    uuid.value[7] = (from.Data3 >>  8) & 0xFF;

    for (unsigned i = 0; i < sizeof(from.Data4); i++)
        uuid.value[8 + i] = from.Data4[i];

    return uuid;
}

static struct uuid make_random()
{
    struct uuid uuid;

    for (unsigned i = 0; i < sizeof(uuid.value); i++)
        uuid.value[i] = rand();

    uuid.value[6] = (uuid.value[6] & 0x0F) | 0x40;
    uuid.value[8] = (uuid.value[8] & 0x3F) | 0x80;

    return uuid;
}

uuid uuid::generate()
{
    UUID uuid;
    if (UuidCreate(&uuid) != RPC_S_UUID_NO_ADDRESS)
        return convert(uuid);

    return make_random();
}

uuid::uuid(const std::string &str)
{
    unsigned char rpcstr[64];
    strncpy(reinterpret_cast<char *>(rpcstr), str.c_str(), sizeof(rpcstr) - 1);
    rpcstr[sizeof(rpcstr) - 1] = '\0';

    UUID uuid;
    if (UuidFromStringA(rpcstr, &uuid) == RPC_S_OK)
        memcpy(value, convert(uuid).value, sizeof(value));
    else
        memset(value, 0, sizeof(value));
}

static UUID convert(const struct uuid &from)
{
    UUID uuid;
    uuid.Data1 =
            (DWORD(from.value[0])      ) |
            (DWORD(from.value[1]) <<  8) |
            (DWORD(from.value[2]) << 16) |
            (DWORD(from.value[3]) << 24);

    uuid.Data2 =
            (DWORD(from.value[4])      ) |
            (DWORD(from.value[5]) <<  8);

    uuid.Data3 =
            (DWORD(from.value[6])      ) |
            (DWORD(from.value[7]) <<  8);

    for (unsigned i = 0; i < sizeof(uuid.Data4); i++)
        uuid.Data4[i] = from.value[8 + i];

    return uuid;
}

uuid::operator std::string() const
{
    auto uuid = convert(*this);
    RPC_CSTR rpc_string;
    if (UuidToStringA(&uuid, &rpc_string) != RPC_S_OK)
        throw std::runtime_error("out of memory");

    const std::string result = reinterpret_cast<const char *>(rpc_string);
    RpcStringFreeA(&rpc_string);

    return result;
}

}
#endif
