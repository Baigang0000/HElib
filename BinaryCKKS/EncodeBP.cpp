#include "EncodeBP.h"
#include <NTL/ZZX.h>
#include <cmath>

namespace BinaryCKKS
{

NTL::ZZX Ecd(const std::vector<std::complex<double>>& z,
             double                                  Delta,
             const Params&                           par)
{
    /* ---------------- Step 1: build base polynomial in R ---------------- */
    NTL::ZZX polyR;
    long nSlots = std::min<long>(z.size(), par.N);   // safety

    for (long i = 0; i < nSlots; ++i) {
        long scaled = static_cast<long>(std::llround(Delta * z[i].real()));
        if (scaled != 0)
            NTL::SetCoeff(polyR, i, scaled);        // coeff_i = ⌈Δ·z_i⌉
    }

    /* ---------------- Step 2: bit-slice into BP -------------------------- */
    return binaryEncode(polyR, par.lambdaB);         // -> 0/1 coeffs, deg < N*λ
}

} // namespace BinaryCKKS
