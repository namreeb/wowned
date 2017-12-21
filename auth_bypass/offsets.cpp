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

#include "offsets.hpp"
#include "misc.hpp"

#include <Windows.h>

#include <cstdint>
#include <cstring>
#include <exception>
#include <vector>
#include <cassert>

#pragma comment(lib, "version.lib")

namespace
{
constexpr std::uint32_t gStaticOffsets[misc::Version::MAX][Offset::MAX] =
{
    // Classic
    {
        0x5B5DD0,   // WowConnection__SendRaw
        0x85E2A0,   // ReconnectChallengeHandler
        0x5BAD49,   // IgnoreServerSRP6
        0x5ABB00,   // GetLogonServer
        0x5B87C9,   // GruntClientLinkInit
        0x5D3650,   // SRP6CalculateProof
        0x80,       // GruntClientLinkState
        0xA4,       // GruntClientSessionKey
        0x194,      // GruntClientLinkConnection
        0x00,       // SRP6A
        0x20,       // SRP6SessionKey
        0x48        // SRP6M
    },
    // TBC
    {
        0x420710,
        0x910BF8,
        0x42276B,
        0x5B3E10,
        0x878909,
        0x5CCF10,
        0x84,
        0xA8,
        0x198,
        0x00,
        0x20,
        0x48
    },
    // WotLK
    {
        0x467990,
        0xB24D28,
        0x8CC866,
        0x6B0F00,
        0x9D1819,
        0x9A83E0,
        0xA4,
        0xC8,
        0x1C8,
        0x00,
        0x20,
        0x48
    },
    // Cataclysm
    {
        0x54E5A0,
        0xD27628,
        0xB03A36,
        0x4CF0E0,
        0xB79BB9,
        0xB3A580,
        0xA4,
        0xC8,
        0x1CC,
        0x00,
        0x20,
        0x48
    }
};

#define THROW_IF(expr, message) if (expr) { throw std::exception(message); }

static constexpr unsigned int Build[] = { 5875, 8606, 12340, 15595 };
static constexpr size_t Builds = sizeof(Build) / sizeof(Build[0]);
static_assert(Builds == misc::Version::MAX, "Incorrect build information");

unsigned int GetBuild()
{
    wchar_t filename[255];
    DWORD size = sizeof(filename) / sizeof(filename[0]);

    THROW_IF(!QueryFullProcessImageName(::GetCurrentProcess(), 0, reinterpret_cast<LPWSTR>(&filename), &size), "QueryFullProcessImageName failed");

    size = ::GetFileVersionInfoSize(filename, nullptr);

    THROW_IF(!size, "Bad VersionInfo size");

    std::vector<std::uint8_t> versionInfo(size);

    THROW_IF(!::GetFileVersionInfo(filename, 0, size, &versionInfo[0]), "GetFileVersionInfo failed");

    VS_FIXEDFILEINFO *verInfo;
    UINT length;

    THROW_IF(!::VerQueryValue(&versionInfo[0], L"\\", reinterpret_cast<LPVOID *>(&verInfo), &length), "VerQueryValue failed");
    THROW_IF(verInfo->dwSignature != 0xFEEF04BD, "Incorrect version signature");

    return static_cast<unsigned int>(verInfo->dwFileVersionLS & 0xFFFF);
}
}

Offsets::Offsets() : _base(reinterpret_cast<std::uint32_t>(::GetModuleHandle(nullptr)))
{
    auto const build = GetBuild();
    for (auto i = 0u; i < Builds; ++i)
    {
        if (build != Build[i])
            continue;

        _cata = i == misc::Version::Cata;

        memcpy(_offsets, gStaticOffsets[i], sizeof(_offsets));
        return;
    }

    throw std::exception("Unsupported version");
}

std::uint32_t Offsets::Get(Offset offset) const
{
    assert(offset < Offset::MAX);
    return _base + (_offsets[offset] - DefaultBase);
}

std::uint32_t Offsets::GetStatic(Offset offset) const
{
    assert(offset < Offset::MAX);
    return _offsets[offset];
}

const Offsets &sOffsets()
{
    static Offsets s;
    return s;
}