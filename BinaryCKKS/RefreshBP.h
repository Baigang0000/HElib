#ifndef BINARY_CKKS_REFRESH_BP_H
#define BINARY_CKKS_REFRESH_BP_H

#include "CiphertextBP.h"
#include "KeyGenBP.h"
#include "EncryptBP.h"
#include "DecryptBP.h"

namespace BinaryCKKS
{

/** Threshold predicate: true iff ct.B exceeds the public bound. */
inline bool NeedRefresh(const Ciphertext& ct, long Bmax)
{
    return ct.B > Bmax;
}

/**
 * Refresh = Decrypt ∘ Encrypt.
 *
 * If the ciphertext’s current noise bound B ≤ Bmax
 *   → return it unchanged.
 * Otherwise
 *   → m̃ ← Dec(sk_old, ct)
 *   → ct' ← Enc(pk_new, m̃)
 *   → return ct'.
 */
Ciphertext Refresh(const Ciphertext& ct,
                   const SecretKey&  sk_old,
                   const PublicKey&  pk_new,
                   const Params&     par,
                   long              Bmax);

} // namespace BinaryCKKS
#endif /* BINARY_CKKS_REFRESH_BP_H */
