#ifndef BINARY_CKKS_MULT_BP_H
#define BINARY_CKKS_MULT_BP_H

#include "CiphertextBP.h"
#include "KeyGenBP.h"

namespace BinaryCKKS
{

/**
 * Homomorphic multiplication in the BP ring:
 *      c₃ = Mult(evk, c₁, c₂)
 *
 * Implements
 *      (d₀,d₁,d₂) = (b₁b₂ , a₁b₂ + a₂b₁ , a₁a₂)
 *      c₃         = (d₀,d₁) + d₂ · evk
 * with evk = (b₀,a₀).
 *
 * The fresh error-bound is the Lemma 3 expression.
 */
Ciphertext Mult(const Ciphertext& c1,
                const Ciphertext& c2,
                const EvalKey&    evk,
                const Params&     par);

} // namespace BinaryCKKS
#endif /* BINARY_CKKS_MULT_BP_H */
