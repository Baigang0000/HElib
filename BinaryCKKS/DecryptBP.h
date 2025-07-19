#ifndef BINARY_CKKS_DECRYPT_BP_H
#define BINARY_CKKS_DECRYPT_BP_H

#include "CiphertextBP.h"
#include "KeyGenBP.h"

namespace BinaryCKKS
{

/**
 * Decrypt a BP ciphertext:
 *      m̃(X) = b(X) + a(X) · s(X)
 * The caller may pass m̃ to Dcd() to obtain the real vector.
 */
NTL::ZZX Decrypt(const SecretKey& sk,
                 const Ciphertext& ct);

} // namespace BinaryCKKS
#endif /* BINARY_CKKS_DECRYPT_BP_H */
