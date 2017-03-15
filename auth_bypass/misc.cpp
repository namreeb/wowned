/*
    MIT License

    Copyright (c) 2017, namreeb (legal@namreeb.org)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#include "misc.hpp"
#include "CDataStore.hpp"
#include "socket.hpp"

#include <string>
#include <cstring>
#include <memory>
#include <cstdint>
#include <vector>
#include <sstream>

using GetRealmT = const char * (*)();

namespace misc
{
const Offsets *Offsets::Current = nullptr;

void GetgN(const std::string &username, std::vector<std::uint8_t> &g, std::vector<std::uint8_t> &N)
{
    auto const getRealm = reinterpret_cast<GetRealmT>(Offsets::Current->GetLogonServer);
    auto const realmName = getRealm();

    std::unique_ptr<Socket> socket;

    if (auto const colon = strchr(realmName, ':'))
        socket = std::make_unique<Socket>(std::string(realmName, static_cast<size_t>(colon - realmName)), std::atoi(colon + 1));
    else
        socket = std::make_unique<Socket>(realmName, 3724);

    try
    {
        socket->Connect();
    }
    catch (boost::system::system_error const &e)
    {
        std::stringstream str;
        str << "Error connecting to " << realmName << ".\n" << e.what();
        MessageBoxA(nullptr, str.str().c_str(), "GetgN failed auth connect", 0);
    }

    try
    {
        const std::uint32_t localAddress = (192 << 24) | (168 << 16) | ((rand() % 10) << 8) | (1 + (rand() % 254));

        socket->PushEndBuffer<std::uint8_t>(0);                                                 // CMD_AUTH_LOGON_CHALLENGE
        socket->PushEndBuffer<std::uint8_t>(0);                                                 // error = 0
        socket->PushEndBuffer<std::uint16_t>(static_cast<std::uint16_t>(30 + username.size())); // size
        socket->PushEndBuffer<std::uint32_t>('WoW\0');                                          // game name
        socket->PushEndBuffer<std::uint8_t>(1);                                                 // version major
        socket->PushEndBuffer<std::uint8_t>(12);                                                // version minor
        socket->PushEndBuffer<std::uint8_t>(1);                                                 // version patch
        socket->PushEndBuffer<std::uint16_t>(5875);                                             // version build
        socket->PushEndBuffer<std::uint32_t>('x86\0');                                          // platform
        socket->PushEndBuffer<std::uint32_t>('Win\0');                                          // operating system
        socket->PushEndBuffer<std::uint32_t>('enUS');                                           // locale
        socket->PushEndBuffer<std::uint32_t>(1);                                                // timezone bias
        socket->PushEndBuffer<std::uint32_t>(localAddress);                                     // local ip
        socket->PushEndBuffer<std::uint8_t>(static_cast<std::uint8_t>(username.size()));        // username length
        socket->PushEndBuffer(username.c_str(), username.size());                               // username

        if (!socket->WriteBuffer())
            throw std::runtime_error("Failed to write CMD_AUTH_LOGON_CHALLENGE");

        std::uint8_t one;

        if (!socket->Read<std::uint8_t>(one) || !!one)
            throw std::runtime_error("Failed to read CMD_AUTH_LOGON_CHALLENGE");

        if (!socket->Read<std::uint8_t>(one) || !!one)
            throw std::runtime_error("Unknown field in CMD_AUTH_LOGON_CHALLENGE is non-zero");

        if (!socket->Read<std::uint8_t>(one))
            throw std::runtime_error("Failed to read result from CMD_AUTH_LOGON_CHALLENGE");

        switch (static_cast<AuthResult>(one))
        {
            case AuthResult::WOW_FAIL_BANNED:
                throw std::runtime_error("Account is banned");
            case AuthResult::WOW_FAIL_SUSPENDED:
                throw std::runtime_error("Account is temporarily suspended");
            case AuthResult::WOW_FAIL_UNKNOWN_ACCOUNT:
                throw std::runtime_error("Unknown account");
            case AuthResult::WOW_SUCCESS:
                break;
            default:
                throw std::runtime_error("Unknown response to CMD_AUTH_LOGON_CHALLENGE");
        }

        struct
        {
            std::uint8_t B[32];
            std::uint8_t g_len;
            std::uint8_t g[1];
            std::uint8_t N_len;
            std::uint8_t N[32];
        } challenge;

        if (!socket->Read(challenge))
            throw std::runtime_error("Failed to read challenge");

        socket->Disconnect();

        if (challenge.g_len > sizeof(challenge.g))
            throw std::runtime_error("g too big");

        g.resize(challenge.g_len);
        memcpy(&g[0], challenge.g, g.size());
        std::reverse(g.begin(), g.end());

        if (challenge.N_len > sizeof(challenge.N))
            throw std::runtime_error("N too big");

        N.resize(challenge.N_len);
        memcpy(&N[0], challenge.N, N.size());
        std::reverse(N.begin(), N.end());
    }
    catch (std::exception const &e)
    {
        MessageBoxA(nullptr, e.what(), "GetgN failure", 0);
    }
}
}
