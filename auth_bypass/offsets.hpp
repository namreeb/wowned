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

#pragma once

#include <cstdint>

enum Offset
{
    WowConnection__SendRaw = 0,
    ReconnectChallengeHandler,
    IgnoreServerSRP6,
    GetLogonServer,
    GruntClientLinkInit,
    SRP6CalculateProof,
    GruntClientLinkState,
    GruntClientSessionKey,
    GruntClientLinkConnection,
    SRP6A,
    SRP6SessionKey,
    SRP6M,
    MAX
};

class Offsets
{
    private:
        static constexpr std::uint32_t DefaultBase = 0x400000;
        const std::uint32_t _base;

        std::uint32_t _offsets[Offset::MAX];
        bool _cata;

    public:
        Offsets();

        std::uint32_t Get(Offset offset) const;
        std::uint32_t GetStatic(Offset offset) const;

        bool IsCata() const { return _cata; }
};

const Offsets &sOffsets();