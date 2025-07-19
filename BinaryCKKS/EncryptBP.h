#ifndef BINARY_CKKS_ENCRYPT_BP_H
#define BINARY_CKKS_ENCRYPT_BP_H

#include "CiphertextBP.h"
#include "KeyGenBP.h"

namespace BinaryCKKS
{

/** Fresh encryption of a BP plaintext m under public key pk. */
Ciphertext Encrypt(const PublicKey& pk,
                   const NTL::ZZX&  mBP,
                   const Params&    par);

} // namespace BinaryCKKS
#endif /* BINARY_CKKS_ENCRYPT_BP_H */
