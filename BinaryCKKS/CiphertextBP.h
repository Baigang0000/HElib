#ifndef BINARY_CKKS_CIPHERTEXT_BP_H
#define BINARY_CKKS_CIPHERTEXT_BP_H

#include <NTL/ZZX.h>

namespace BinaryCKKS
{

/**
 * Ciphertext in the BP ring:
 *     c = (b , a)   with  ⟨c , sk⟩ = m + e ,
 * plus a running absolute-error bound  B ≥ ‖e‖_{R,∞}.
 *
 * All polynomials must have coefficients in {0,1}.
 */
struct Ciphertext
{
    NTL::ZZX b;   //!< first polynomial component  (BP)
    NTL::ZZX a;   //!< second polynomial component (BP)
    long     B;   //!< absolute error bound |e|

    Ciphertext() : B(0) {}
    Ciphertext(const NTL::ZZX& b_, const NTL::ZZX& a_, long B_)
        : b(b_), a(a_), B(B_) {}

    /** Return the all-zero ciphertext. */
    static Ciphertext zero();

    /** Component-wise homomorphic addition. */
    static Ciphertext add(const Ciphertext& c1, const Ciphertext& c2);

    /** Multiply ciphertext by a public (signed) integer k. */
    static Ciphertext mulConst(const Ciphertext& c, long k);

    /** Debug helper: print a few non-zero terms of each polynomial. */
    void debugPrint(long maxTerms = 8) const;
};

} // namespace BinaryCKKS

#endif /* BINARY_CKKS_CIPHERTEXT_BP_H */
