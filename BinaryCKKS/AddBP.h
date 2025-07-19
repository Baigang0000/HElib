#ifndef BINARY_CKKS_ADD_BP_H
#define BINARY_CKKS_ADD_BP_H

#include "CiphertextBP.h"

namespace BinaryCKKS
{

/** Component-wise homomorphic addition in BP. */
inline Ciphertext Add(const Ciphertext& c1,
                      const Ciphertext& c2)
{
    return Ciphertext::add(c1, c2);     // b/a add, B₁+B₂
}

} // namespace BinaryCKKS
#endif /* BINARY_CKKS_ADD_BP_H */
