#include "DecodeBP.h"
#include <NTL/ZZX.h>

namespace BinaryCKKS
{

std::vector<std::complex<double>>
Dcd(const NTL::ZZX& bpPoly,
    double          Delta,
    const Params&   par)
{
    const long λ = par.lambdaB;
    const long N = par.N;
    std::vector<long> coeffInt(N, 0);

    /* ---- gather bits back into integer coefficients ---- */
    for (long idx = 0; idx <= NTL::deg(bpPoly); ++idx)
    {
        if (NTL::IsZero(NTL::coeff(bpPoly, idx))) continue;
        long base = idx / λ;
        long bit  = idx %  λ;
        if (base < N)
            coeffInt[base] += (1L << bit);
    }

    /* ---- scale down by Δ ---- */
    std::vector<std::complex<double>> z;
    z.reserve(N);
    for (long i = 0; i < N; ++i)
        z.emplace_back(static_cast<double>(coeffInt[i]) / Delta, 0.0);

    return z;
}

} // namespace BinaryCKKS
