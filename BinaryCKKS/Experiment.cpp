/********************************************************************
 *  Binary-CKKS  +  BCH(127,106,3) experiment                       *
 ********************************************************************/
#include <iostream>
#include <random>

/* ---- NTL ---- */
#include <NTL/ZZX.h>
#include <NTL/GF2X.h>

/* ---- our project headers ---- */
#include "CiphertextBP.h"
#include "KeyGenBP.h"
#include "EncodeBP.h"
#include "EncryptBP.h"
#include "DecryptBP.h"
#include "MultBP.h"
#include "RefreshBP.h"
#include "BCH127.h"

/*  an extra NTL include for helpers below  */
#include <NTL/ZZX.h>

using namespace BinaryCKKS;

/* ------------ helpers: convert bit-poly <-> ZZX ----------------- */
static NTL::ZZX gf2xToZZX(const NTL::GF2X& g)
{
    NTL::ZZX z;
    for (long i = 0; i <= NTL::deg(g); ++i)
        if (NTL::IsOne(NTL::coeff(g, i)))
            NTL::SetCoeff(z, i, 1);
    return z;
}
static NTL::GF2X zzxBitsToGF2X(const NTL::ZZX& z, long len)
{
    NTL::GF2X g;
    for (long i = 0; i < len; ++i)
        if (!NTL::IsZero(NTL::coeff(z, i)))
            NTL::SetCoeff(g, i);
    return g;
}

int main()
{
    /* ---------- scheme parameters ---------- */
    Params par;                 // defaults from KeyGenBP.h
    par.N        = 8192;
    par.lambdaB  = 32;
    par.h        = 64;
    par.sigma    = 3.2;
    double Delta = static_cast<double>(1ULL << 40);

    long Bmax = 1L << 35;       // refresh threshold

    /* ---------- keys ---------- */
    auto kp = KeyGen(par);

    /* ---------- random source ---------- */
    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<int> bit(0, 1);

    const long TRIALS = 10;
    long failures = 0;

    for (long t = 0; t < TRIALS; ++t)
    {
        /* ---- 1. random 106-bit message ---- */
        std::vector<uint8_t> msgBits(BCH127::k);
        for (auto& b : msgBits) b = bit(rng);

        /* ---- 2. BCH encode ---- */
        NTL::GF2X data = BCH127::vecToPoly(msgBits);
        NTL::GF2X code = BCH127::encode(data);          // 127 bits

        /* ---- 3. embed as BP polynomial ---- */
        NTL::ZZX mBP = gf2xToZZX(code);

        /* ---- 4. encrypt ---- */
        Ciphertext ct = Encrypt(kp.pk, mBP, par);

        /* ---- 5. simple workload: square once ---- */
        ct = Mult(ct, ct, kp.evk, par);
        if (NeedRefresh(ct, Bmax))
            ct = Refresh(ct, kp.sk, kp.pk, par, Bmax);

        /* ---- 6. decrypt ---- */
        NTL::ZZX noisyBP = Decrypt(kp.sk, ct);

        /* ---- 7. extract 127 LSBs, BCH decode ---- */
        NTL::GF2X recv = zzxBitsToGF2X(noisyBP, BCH127::n);

        NTL::GF2X decoded;
        bool ok = BCH127::decode(recv, decoded);

        if (!ok || decoded != data)
            ++failures;
    }

    std::cout << "Trials   : " << TRIALS   << '\n'
              << "Failures : " << failures << '\n'
              << "Fail rate: "
              << static_cast<double>(failures) / TRIALS << std::endl;
}
