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

#include <hadesmem/patcher.hpp>

#include <memory>
#include <string>
#include <vector>

namespace method
{
class Interface
{
    public:
        // this class must have a body, even though we don't use it, or the lambda
        // in method.cpp will not produce sane assembly
        class WowConnection
        {
            int __unused;
        };

        using RealmSendT = int(__thiscall WowConnection::*)(void *data, int len, bool disableEncryption);
        using RealmSendCataT = int(__thiscall WowConnection::*)(void *data, int len);

    private:
        std::unique_ptr<hadesmem::PatchDetour<RealmSendT>> m_realmSendHook;
        std::unique_ptr<hadesmem::PatchDetour<RealmSendCataT>> m_realmSendCataHook;

        std::unique_ptr<hadesmem::PatchRaw> m_ignoreSRP6Patch;

    protected:
        std::string m_username;

    public:
        Interface(bool cata);

        const std::string &GetUsername() const { return m_username; }

        virtual bool IsOne() const = 0;
};

class One : public Interface
{
    private:
        std::unique_ptr<hadesmem::PatchRaw> m_reconnectChallengePatch;
        std::unique_ptr<hadesmem::PatchRaw> m_gruntClientLinkPatch;

    public:
        One(bool cata);

        virtual bool IsOne() const { return true; }
};

class Two : public Interface
{
    private:
        // this class must have a body, even though we don't use it, or the lambda
        // in method.cpp will not produce sane assembly
        class SRP6_Client
        {
            int __unused;
        };

        using CalculateProofT = int (__thiscall SRP6_Client::*)(
            const std::uint8_t *N, unsigned int NLength,
            const std::uint8_t *g, unsigned int gLength,
            const std::uint8_t *s, unsigned int sLength,
            const std::uint8_t *B, unsigned int BLength,
            void *srpRandom);

        std::unique_ptr<hadesmem::PatchDetour<CalculateProofT>> m_calculateProofHook;

    public:
        Two(bool cata);

        virtual bool IsOne() const { return false; }
};

extern std::unique_ptr<method::Interface> gMethod;
}