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

#include "SRP6a.hpp"

#include <openssl/bn.h>
#include <openssl/sha.h>

#include <cassert>
#include <vector>
#include <cstdint>

#pragma comment(lib, "libcrypto.lib")

using PBN_CTX = std::unique_ptr<BN_CTX, decltype(&::BN_CTX_free)>;

namespace
{
void ConvertBN(const BIGNUM* bn, std::vector<std::uint8_t>& out)
{
    out.resize(BN_num_bytes(bn));

    if (!out.empty())
    {
        auto const len = ::BN_bn2bin(bn, &out[0]);
        assert(len == out.size());

        // im not sure why this needs to be reversed, but it does
        std::reverse(out.begin(), out.end());
    }
}

void ConvertBN(const crypto::SRP6a::PBIGNUM& bn, std::vector<std::uint8_t>& out)
{
    ConvertBN(bn.get(), out);
}

void SHA1_BN_Update(SHA_CTX* ctx, const BIGNUM* bn)
{
    std::vector<std::uint8_t> bytes;
    ConvertBN(bn, bytes);

    if (!bytes.empty())
        ::SHA1_Update(ctx, &bytes[0], bytes.size());
}

void SHA1_BN_Update(SHA_CTX* ctx, const crypto::SRP6a::PBIGNUM& bn)
{
    SHA1_BN_Update(ctx, bn.get());
}

void HashBN(const BIGNUM* bn, std::uint8_t* hash)
{
    assert(bn);
    assert(hash);

    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_BN_Update(&ctx, bn);
    SHA1_Final(hash, &ctx);
}

void HashBN(const crypto::SRP6a::PBIGNUM& bn, std::uint8_t* hash)
{
    HashBN(bn.get(), hash);
}

void HashString(const std::string& str, std::uint8_t* hash)
{
    assert(hash);
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, str.c_str(), str.length());
    SHA1_Final(hash, &ctx);
}

// std::unique_ptr openssl trampolines
BIGNUM* BN_bin2bn(const std::uint8_t* s, int len, const crypto::SRP6a::PBIGNUM& ret)
{
    return ::BN_bin2bn(s, len, ret.get());
}
int BN_bn2bin(const crypto::SRP6a::PBIGNUM& a, std::uint8_t* to)
{
    return ::BN_bn2bin(a.get(), to);
}
int BN_rand(const crypto::SRP6a::PBIGNUM& rnd, int bits, int top, int bottom)
{
    return ::BN_rand(rnd.get(), bits, top, bottom);
}
int BN_mod_exp(const crypto::SRP6a::PBIGNUM& r, const crypto::SRP6a::PBIGNUM& a, const crypto::SRP6a::PBIGNUM& p,
    const crypto::SRP6a::PBIGNUM& m, PBN_CTX& ctx)
{
    return ::BN_mod_exp(r.get(), a.get(), p.get(), m.get(), ctx.get());
}
int BN_set_word(const crypto::SRP6a::PBIGNUM& a, BN_ULONG w)
{
    return ::BN_set_word(a.get(), w);
}
int BN_mul(const crypto::SRP6a::PBIGNUM& r, const crypto::SRP6a::PBIGNUM& a, const crypto::SRP6a::PBIGNUM& b, PBN_CTX& ctx)
{
    return ::BN_mul(r.get(), a.get(), b.get(), ctx.get());
}
int BN_add(const crypto::SRP6a::PBIGNUM& r, const crypto::SRP6a::PBIGNUM& a, const crypto::SRP6a::PBIGNUM& b)
{
    return ::BN_add(r.get(), a.get(), b.get());
}
int BN_sub(const crypto::SRP6a::PBIGNUM& r, const crypto::SRP6a::PBIGNUM& a, const crypto::SRP6a::PBIGNUM& b)
{
    return ::BN_sub(r.get(), a.get(), b.get());
}
}

#define BN_EMPTY(x) const PBIGNUM x(BN_new(), ::BN_free)
#define BN(x, y) const PBIGNUM x(y, ::BN_free)

namespace crypto
{
SRP6a::SRP6a(const std::vector<std::uint8_t> &gBuff,
    const std::vector<std::uint8_t> &NBuff, bool forgeA,
    const std::vector<std::uint8_t> &sBuff,
    const std::vector<std::uint8_t> &BBuff)
    : salt(sBuff), A(BN_new(), ::BN_free), g(BN_new(), ::BN_free), N(BN_new(), ::BN_free), B(BN_new(), ::BN_free)
{
    PBN_CTX ctx(BN_CTX_new(), ::BN_CTX_free);

    if (!gBuff.empty())
        BN_bin2bn(&gBuff[0], gBuff.size(), g);

    if (!NBuff.empty())
        BN_bin2bn(&NBuff[0], NBuff.size(), N);

    if (!BBuff.empty())
        BN_bin2bn(&BBuff[0], BBuff.size(), B);

    std::vector<unsigned char> t(32);

    if (forgeA)
        BN_bin2bn(&NBuff[0], NBuff.size(), A);
    else
    {
        BN_EMPTY(a);
        BN_rand(a, 19 * 8, 1, 1);

        BN_mod_exp(A, g, a, N, ctx);
        t[31] = 1;
    }

    std::vector<unsigned char> t1(t.size() / 2);
    {
        for (size_t i = 0; i < t1.size(); ++i)
            t1[i] = t[i * 2];

        SHA_CTX tCtx;
        SHA1_Init(&tCtx);

        SHA1_Update(&tCtx, &t1[0], t1.size());

        unsigned char t1Hash[SHA_DIGEST_LENGTH];
        SHA1_Final(t1Hash, &tCtx);

        for (auto i = 0; i < SHA_DIGEST_LENGTH; ++i)
            m_K[i * 2] = t1Hash[i];
    }
    {
        for (size_t i = 0; i < t1.size(); ++i)
            t1[i] = t[i * 2 + 1];

        SHA_CTX tCtx;
        SHA1_Init(&tCtx);

        SHA1_Update(&tCtx, &t1[0], t1.size());

        unsigned char t1Hash[SHA_DIGEST_LENGTH];
        SHA1_Final(t1Hash, &tCtx);

        for (auto i = 0; i < SHA_DIGEST_LENGTH; ++i)
            m_K[i * 2 + 1] = t1Hash[i];
    }
}

void SRP6a::GetA(std::vector<std::uint8_t>& out) const
{
    ConvertBN(A, out);
}

void SRP6a::GetK(std::vector<std::uint8_t>& out) const
{
    out.resize(sizeof(m_K));
    memcpy(&out[0], m_K, out.size());
    std::reverse(out.begin(), out.end());
}

void SRP6a::GetM(const std::string& username, std::vector<std::uint8_t>& out) const
{
    unsigned char nHash[SHA_DIGEST_LENGTH];
    HashBN(N, nHash);

    unsigned char gHash[SHA_DIGEST_LENGTH];
    HashBN(g, gHash);

    unsigned char usernameHash[SHA_DIGEST_LENGTH];
    HashString(username, usernameHash);

    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
        nHash[i] ^= gHash[i];

    {
        SHA_CTX mCtx;
        SHA1_Init(&mCtx);

        SHA1_Update(&mCtx, nHash, SHA_DIGEST_LENGTH);
        SHA1_Update(&mCtx, usernameHash, SHA_DIGEST_LENGTH);
        if (!salt.empty())
            SHA1_Update(&mCtx, &salt[0], salt.size());
        SHA1_BN_Update(&mCtx, A);
        SHA1_BN_Update(&mCtx, B);
        SHA1_Update(&mCtx, m_K, sizeof(m_K));

        out.resize(SHA_DIGEST_LENGTH);
        SHA1_Final(&out[0], &mCtx);
    }
}
}