#include "KeyGenBP.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <unordered_set>

namespace BinaryCKKS
{
/*===========================  internals  ===========================*/
static std::mt19937_64& prng()
{
    static std::mt19937_64 g{std::random_device{}()};
    return g;
}

/*  Bernoulli(½) bit  */
static inline long randBit() { return prng()() & 1ULL; }

/*  Discrete Gaussian coefficient via Box–Muller + rounding.          */
static long sampleGauss(double sigma)
{
    static std::normal_distribution<double> nd(0.0, 1.0);
    return static_cast<long>(std::llround(nd(prng()) * sigma));
}

/*  Sample sparse ±1 polynomial with exactly h non-zero coeffs.       */
static NTL::ZZX sampleHWT(long M, long h)
{
    NTL::ZZX s;
    std::uniform_int_distribution<long> dist(0, M - 1);
    std::unordered_set<long> chosen;
    while (static_cast<long>(chosen.size()) < h) {
        long pos = dist(prng());
        if (chosen.insert(pos).second)
            NTL::SetCoeff(s, pos, randBit() ? 1 : -1);
    }
    return s;
}

/*  Uniform binary polynomial in BP                                   */
static NTL::ZZX sampleUniformBP(const Params& par)
{
    long M = par.ringDegree();
    NTL::ZZX a;
    for (long i = 0; i < M; ++i)
        if (randBit()) NTL::SetCoeff(a, i, 1);
    return a;
}


/*  Discrete Gaussian → BP (binaryEncode later ensures 0/1 coeffs)    */
static NTL::ZZX sampleGaussianBP(long M, double sigma)
{
    NTL::ZZX e;
    for (long i = 0; i < M; ++i)
        if (long v = sampleGauss(sigma))
            NTL::SetCoeff(e, i, v);
    return binaryEncode(e);     // reduce to {0,1}
}
/*  Binary encode polynomial (reduce to {0,1} coefficients)           */

NTL::ZZX binaryEncode(const NTL::ZZX& poly, long lambdaB)
{
    NTL::ZZX out;
    for (long i = 0; i <= NTL::deg(poly); ++i) {
        long coeff = NTL::to_long(NTL::coeff(poly, i));
        // handle negative values
        if (coeff < 0) coeff = std::abs(coeff);  //  sign can be carried elsewhere
        for (long j = 0; j < lambdaB; ++j) {
            if (coeff & (1L << j))
                NTL::SetCoeff(out, i * lambdaB + j, 1L);
        }
    }
    return out;    // coefficients ∈ {0,1}
}

/*============================  KeyGen  =============================*/
KeyPair KeyGen(const Params& par)
{
    /* --- secret key --------------------------------------------- */
    NTL::ZZX s_raw = sampleHWT(par.ringDegree(), par.h);      // ±1 sparse
    NTL::ZZX s     = binaryEncode(s_raw, par.lambdaB);   // p^{-1}(s)

    /* --- public key (b,a) --------------------------------------- */
    NTL::ZZX a = sampleUniformBP(par);           // uniform binary
    NTL::ZZX e = sampleGaussianBP(par.ringDegree(), par.sigma);
    NTL::ZZX b = -(a * s) + e;                     // −a·s + e   (BP arithmetic)

    /* --- evaluation key (b0,a0) --------------------------------- */
    NTL::ZZX a0 = sampleUniformBP(par);
    NTL::ZZX e0 = sampleGaussianBP(par.ringDegree(), par.sigma);
    NTL::ZZX s2 = s * s;                           // s²  (still binary)
    NTL::ZZX b0 = -(a0 * s) + e0 + s2;             // −a0·s + e0 + s²

    KeyPair kp;
    kp.sk.s  = s;
    kp.pk.a  = a;
    kp.pk.b  = b;
    kp.evk.a0 = a0;
    kp.evk.b0 = b0;
    return kp;
}

} // namespace BinaryCKKS
