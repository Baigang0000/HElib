#include "RefreshBP.h"

namespace BinaryCKKS
{

Ciphertext Refresh(const Ciphertext& ct,
                   const SecretKey&  sk_old,
                   const PublicKey&  pk_new,
                   const Params&     par,
                   long              Bmax)
{
    if (!NeedRefresh(ct, Bmax))
        return ct;                        // still “fresh enough”

    /* -- decrypt under old secret -- */
    NTL::ZZX plainBP = Decrypt(sk_old, ct);

    /* -- re-encrypt under new public key (fresh noise bound) -- */
    Ciphertext fresh = Encrypt(pk_new, plainBP, par);
    return fresh;
}

} // namespace BinaryCKKS
