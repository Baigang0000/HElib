#ifndef BINARY_CKKS_ENCODE_BP_H
#define BINARY_CKKS_ENCODE_BP_H

#include "KeyGenBP.h"     // brings in Params and binaryEncode
#include <vector>
#include <complex>

namespace BinaryCKKS
{

/**
 * Encode a vector z (|z| ≤  B/Δ) into the BP ring.
 *
 *  z  :  length ≤ N/2 real-or-complex vector (imag part ignored here)
 *  Δ  :  scaling factor (e.g. 2^40)
 *  par:  scheme parameters (N, lambdaB, …)
 *
 * Returns BP polynomial Q(X) of degree < M = N·lambdaB.
 */
NTL::ZZX Ecd(const std::vector<std::complex<double>>& z,
             double                                Delta,
             const Params&                         par);

} // namespace BinaryCKKS
#endif /* BINARY_CKKS_ENCODE_BP_H */
