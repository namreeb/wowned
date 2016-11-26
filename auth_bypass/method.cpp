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
#include "CDataStore.hpp"
#include "SRP6a.hpp"

#include <Windows.h>

#include <hadesmem/process.hpp>
#include <hadesmem/patcher.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace
{
void AmmendRealmPacket(void *data, std::string &un)
{
    auto const cmd = reinterpret_cast<std::uint8_t *>(data);

    // if this is not the initial logon packet, do nothing
    if (*cmd)
        return;

    // for method one, rewrite CMD_AUTH_LOGON_CHALLENGE to CMD_AUTH_RECONNECT_CHALLENGE
    if (method::gMethod->IsOne())
        *cmd = 2u;

    auto const usernameLength = *(reinterpret_cast<std::uint8_t *>(data) + 33);
    std::vector<char> username(usernameLength);
    memcpy(&username[0], reinterpret_cast<const std::uint8_t *>(data) + 34, username.size());

    un = std::string(&username[0], username.size());
}

struct GruntClientLink
{
void SetState(unsigned int state)
{
    auto const p = reinterpret_cast<std::uint32_t *>(reinterpret_cast<std::uint8_t *>(this) + misc::Offsets::Current->GruntClientLinkState);
    *p = state;
}

void SetSessionKey(const std::vector<std::uint8_t> &key)
{
    auto const p = reinterpret_cast<std::uint8_t *>(this) + misc::Offsets::Current->GruntClientSessionKey;
    memcpy(p, &key[0], key.size());
}

method::Interface::WowConnection *GetConnection()
{
    return *reinterpret_cast<method::Interface::WowConnection **>(reinterpret_cast<std::uint8_t *>(this) + misc::Offsets::Current->GruntClientLinkConnection);
}

int ReconnectChallenge(CDataStore *packet)
{
    SetState(4);

    std::vector<std::uint8_t> gBuff, NBuff;
    misc::GetgN(method::gMethod->GetUsername(), gBuff, NBuff);

    std::vector<std::uint8_t> zero;
    const crypto::SRP6a srp6(gBuff, NBuff, false, zero, zero);

    std::vector<std::uint8_t> ABuff;
    srp6.GetA(ABuff);

    std::vector<std::uint8_t> KBuff;
    srp6.GetK(KBuff);
    SetSessionKey(KBuff);

    std::vector<std::uint8_t> MBuff;
    srp6.GetM(method::gMethod->GetUsername(), MBuff);

    CDataStore newPacket(75);

    newPacket.Write<std::uint8_t>(1);
    newPacket.Write(&ABuff[0], ABuff.size());
    newPacket.Write(&MBuff[0], MBuff.size());

    // crc hash is ignored
    for (auto i = 0; i < 5; ++i)
        newPacket.Write<std::uint32_t>(0);

    for (auto i = newPacket.m_bytesWritten; i < newPacket.m_capacity; ++i)
        newPacket.Write<std::uint8_t>(0);

    Send(&newPacket);

    packet->m_bytesRead = packet->m_bytesWritten;

    return 2;
}

void Send(CDataStore *packet)
{
    auto const sendPacket = hadesmem::detail::AliasCastUnchecked<method::One::RealmSendT>(misc::Offsets::Current->WowConnection__SendRaw);
    auto const p = GetConnection();
    (p->*sendPacket)(packet->m_data, packet->m_bytesWritten, true);
}
};

// instead of doing all of the SRP6 work, we take a shortcut.  this function must set values for A, K and M
void UpdateSRP(const std::string &username, std::uint8_t *srp6Client, const std::vector<std::uint8_t> &g,
    const std::vector<std::uint8_t> &N, const std::vector<std::uint8_t> &s, const std::vector<std::uint8_t> &B)
{
    const crypto::SRP6a srp6(g, N, true, s, B);

    auto const sessionA = srp6Client + misc::Offsets::Current->SRP6A;
    std::vector<std::uint8_t> A;
    srp6.GetA(A);
    memcpy(sessionA, &A[0], A.size());

    auto const sessionKey = srp6Client + misc::Offsets::Current->SRP6SessionKey;
    std::vector<std::uint8_t> K;
    srp6.GetK(K);
    std::reverse(K.begin(), K.end());
    memcpy(sessionKey, &K[0], K.size());

    auto const sessionM = srp6Client + misc::Offsets::Current->SRP6M;
    std::vector<std::uint8_t> M;
    srp6.GetM(username, M);
    memcpy(sessionM, &M[0], M.size());
}
}

namespace method
{
std::unique_ptr<method::Interface> gMethod;

Interface::Interface()
{
    const hadesmem::Process process(::GetCurrentProcessId());

    m_realmSendHook = std::make_unique<hadesmem::PatchDetour<RealmSendT>>(process,
        hadesmem::detail::AliasCastUnchecked<RealmSendT>(misc::Offsets::Current->WowConnection__SendRaw),
        [&username = m_username] (hadesmem::PatchDetourBase *detourBase, WowConnection *realm, void *data, int len, bool disableEncryption)
        {
            AmmendRealmPacket(data, username);
            auto const orig = detourBase->GetTrampolineT<RealmSendT>();
            return (realm->*orig)(data, len, disableEncryption);
        }
    );

    m_realmSendHook->Apply();

    std::vector<std::uint8_t> nopPatch(2, 0x90);
    m_ignoreSRP6Patch = std::make_unique<hadesmem::PatchRaw>(process, reinterpret_cast<PVOID>(misc::Offsets::Current->IgnoreServerSRP6), nopPatch);
    m_ignoreSRP6Patch->Apply();
}

One::One()
{
    const hadesmem::Process process(::GetCurrentProcessId());

    // we want to be flexible in terms of when this code can be called.  therefore, we use hadesmme::PatchRaw rather than
    // editing memory ourselves, so as to bypass any page protection that may be present

    auto const reconnectChallenge = hadesmem::detail::AliasCast<FARPROC>(&GruntClientLink::ReconnectChallenge);
    std::vector<std::uint8_t> reconnectPatch(4);
    memcpy(&reconnectPatch[0], &reconnectChallenge, sizeof(reconnectChallenge));

    m_reconnectChallengePatch = std::make_unique<hadesmem::PatchRaw>(process, reinterpret_cast<PVOID>(misc::Offsets::Current->ReconnectChallengeHandler), reconnectPatch);
    m_reconnectChallengePatch->Apply();

    std::vector<std::uint8_t> nopPatch(5, 0x90);

    m_gruntClientLinkPatch = std::make_unique<hadesmem::PatchRaw>(process, reinterpret_cast<PVOID>(misc::Offsets::Current->GruntClientLinkInit), nopPatch);
    m_gruntClientLinkPatch->Apply();
}

Two::Two()
{
    const hadesmem::Process process(::GetCurrentProcessId());

    m_calculateProofHook = std::make_unique<hadesmem::PatchDetour<CalculateProofT>>(process,
        hadesmem::detail::AliasCastUnchecked<CalculateProofT>(misc::Offsets::Current->SRP6CalculateProof),
        [&username = m_username](hadesmem::PatchDetourBase *detourBase, SRP6_Client *srp6,
            const std::uint8_t *N, unsigned int NLength,
            const std::uint8_t *g, unsigned int gLength,
            const std::uint8_t *s, unsigned int sLength,
            const std::uint8_t *B, unsigned int BLength,
            void *srpRandom)
        {
            std::vector<std::uint8_t> gBuff(gLength);
            memcpy(&gBuff[0], g, gBuff.size());
            std::reverse(gBuff.begin(), gBuff.end());

            std::vector<std::uint8_t> NBuff(NLength);
            memcpy(&NBuff[0], N, NBuff.size());
            std::reverse(NBuff.begin(), NBuff.end());

            std::vector<std::uint8_t> sBuff(sLength);
            memcpy(&sBuff[0], s, sBuff.size());

            std::vector<std::uint8_t> BBuff(BLength);
            memcpy(&BBuff[0], B, BBuff.size());
            std::reverse(BBuff.begin(), BBuff.end());

            UpdateSRP(username, reinterpret_cast<std::uint8_t *>(srp6), gBuff, NBuff, sBuff, BBuff);

            return 0;
        }
    );

    m_calculateProofHook->Apply();
}
}