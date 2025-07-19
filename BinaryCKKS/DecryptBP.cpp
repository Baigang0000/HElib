#include "DecryptBP.h"

namespace BinaryCKKS
{

NTL::ZZX Decrypt(const SecretKey& sk,
                 const Ciphertext& ct)
{
    /*  m̃ = b + a·s   (BP arithmetic) */
    NTL::ZZX plain = ct.b + (ct.a * sk.s);
    return plain;                     // still contains additive noise
}

} // namespace BinaryCKKS
