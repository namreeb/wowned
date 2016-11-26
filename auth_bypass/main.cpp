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

#include "method.hpp"
#include "misc.hpp"

#include <Windows.h>

#include <exception>
#include <vector>
#include <memory>

#pragma comment(lib, "version.lib")

#define THROW_IF(expr, message) if (expr) { throw std::exception(message); }

static constexpr unsigned int Build[] = { 5875, 8606, 12340 };


namespace
{
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

using VER = misc::Version;

extern "C" __declspec(dllexport) void Load1()
{
    const misc::Offsets *currentVersion = nullptr;

    try
    {
        switch (GetBuild())
        {
            case Build[VER::Classic]:
                currentVersion = &misc::Versions[VER::Classic];
                break;
            case Build[VER::TBC]:
                currentVersion = &misc::Versions[VER::TBC];
                break;
            case Build[VER::WOTLK]:
                currentVersion = &misc::Versions[VER::WOTLK];
                break;
            default:
                throw std::exception("Unsupported version");
        }
    }
    catch (std::exception const &e)
    {
        MessageBoxA(nullptr, e.what(), "Load1 ERROR", 0);
    }

    *const_cast<const misc::Offsets **>(&misc::Offsets::Current) = currentVersion;
    method::gMethod = std::make_unique<method::One>();
}

extern "C" __declspec(dllexport) void Load2()
{
    const misc::Offsets *currentVersion = nullptr;
    auto isClassic = false;

    try
    {
        switch (GetBuild())
        {
            case Build[VER::Classic]:
                currentVersion = &misc::Versions[VER::Classic];
                isClassic = true;
                break;
            case Build[VER::TBC]:
                currentVersion = &misc::Versions[VER::TBC];
                break;
            case Build[VER::WOTLK]:
                currentVersion = &misc::Versions[VER::WOTLK];
                break;
            default:
                throw std::exception("Unsupported version");
        }
    }
    catch (std::exception const &e)
    {
        MessageBoxA(nullptr, e.what(), "Load2 ERROR", 0);
    }

    *const_cast<const misc::Offsets **>(&misc::Offsets::Current) = currentVersion;
    method::gMethod = std::make_unique<method::Two>();
}