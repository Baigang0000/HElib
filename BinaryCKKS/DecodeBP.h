#ifndef BINARY_CKKS_DECODE_BP_H
#define BINARY_CKKS_DECODE_BP_H

#include "KeyGenBP.h"
#include <vector>
#include <complex>

namespace BinaryCKKS
{

/**
 * Decode a BP polynomial back to a (real) vector.
 *
 *  bpPoly :  polynomial with 0/1 coeffs, deg < N·λ
 *  Δ      :  scaling factor used in Ecd
 *  par    :  parameters (N, lambdaB, …)
 *
 * Returns  z  of length ≤ N ; imaginary parts are zero.
 */
std::vector<std::complex<double>>
Dcd(const NTL::ZZX&  bpPoly,
    double           Delta,
    const Params&    par);

} // namespace BinaryCKKS
#endif /* BINARY_CKKS_DECODE_BP_H */
