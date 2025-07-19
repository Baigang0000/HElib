#include "EncryptBP.h"
#include <random>
#include <cmath>

namespace BinaryCKKS
{

/* ---------------- PRNG helpers ---------------- */
static std::mt19937_64& prng()
{
    static std::mt19937_64 g{std::random_device{}()};
    return g;
}

static inline long randBit() { return prng()() & 1ULL; }

static long sampleGauss(double sigma)
{
    static std::normal_distribution<double> nd(0.0, 1.0);
    return static_cast<long>(std::llround(nd(prng()) * sigma));
}

/* uniform {0,1}^M */
static NTL::ZZX sampleZO(long M)
{
    NTL::ZZX v;
    for (long i = 0; i < M; ++i)
        if (randBit()) NTL::SetCoeff(v, i, 1);
    return v;
}

/* discrete Gaussian followed by bit-slice expansion */
static NTL::ZZX sampleGaussBP(const Params& par)
{
    long M = par.ringDegree();
    NTL::ZZX e;
    for (long i = 0; i < M; ++i)
        if (long v = sampleGauss(par.sigma))
            NTL::SetCoeff(e, i, v);
    return binaryEncode(e, par.lambdaB);
}

/* ---------------- Encrypt ---------------- */
Ciphertext Encrypt(const PublicKey& pk,
                   const NTL::ZZX&  mBP,
                   const Params&    par)
{
    const long M = par.ringDegree();

    /*   randomness  */
    NTL::ZZX vBP  = sampleZO(M);          // ZO(0.5) already in BP
    NTL::ZZX e0BP = sampleGaussBP(par);
    NTL::ZZX e1BP = sampleGaussBP(par);

    /*   c = v·pk + (m+e0, e1)   */
    NTL::ZZX c_b = vBP * pk.b + (mBP + e0BP);
    NTL::ZZX c_a = vBP * pk.a + e1BP;

    /*   fresh noise bound  (rounded up)   */
    double Ndbl = static_cast<double>(par.N);
    double Bclean =
        8.0 * std::sqrt(2.0) * par.sigma * Ndbl +
        6.0 * par.sigma * std::sqrt(Ndbl) +
        16.0 * par.sigma * std::sqrt(static_cast<double>(par.h) * Ndbl);
    long Benc = static_cast<long>(std::ceil(Bclean));

    return Ciphertext{c_b, c_a, Benc};
}

} // namespace BinaryCKKS
