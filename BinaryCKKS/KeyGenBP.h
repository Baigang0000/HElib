#ifndef BINARY_CKKS_KEYGEN_BP_H
#define BINARY_CKKS_KEYGEN_BP_H

#include <NTL/ZZX.h>
#include <random>

namespace BinaryCKKS
{

/* ------------------------------------------------------------------ */
/* Parameter bundle                                                   */
/* ------------------------------------------------------------------ */
struct Params
{
    long   N   = 4096;   //!< degree of base cyclotomic ring R
    long   lambdaB = 32; //!< ⌈log2 B⌉  (bits per coefficient)
    long   h   = 64;     //!< Hamming weight of secret
    double sigma = 3.2;  //!< Gaussian σ

    long ringDegree() const { return N * lambdaB; }   // = M
};

/* ------------------------------------------------------------------ */
/* Key containers                                                     */
/* ------------------------------------------------------------------ */
struct SecretKey
{
    NTL::ZZX s;       //!< sparse ±1 polynomial  (binary encoded)
};

struct PublicKey
{
    NTL::ZZX b;       //!< b = −a·s + e      (BP)
    NTL::ZZX a;       //!< a ∈_U BP
};

struct EvalKey
{
    NTL::ZZX b0;      //!< b0 = −a0·s + e0 + s²  (BP)
    NTL::ZZX a0;      //!< a0 ∈_U BP
};

/* ------------------------------------------------------------------ */
/* Top-level key generator                                            */
/* ------------------------------------------------------------------ */
struct KeyPair
{
    SecretKey sk;
    PublicKey pk;
    EvalKey   evk;
};

KeyPair KeyGen(const Params& par);

/* ------------------------------------------------------------------ */
/* Utility exposed for other modules                                  */
/* ------------------------------------------------------------------ */
NTL::ZZX binaryEncode(const NTL::ZZX& poly, long P = 2);

} // namespace BinaryCKKS
#endif /* BINARY_CKKS_KEYGEN_BP_H */
