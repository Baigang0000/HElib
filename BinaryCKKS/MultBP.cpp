#include "MultBP.h"
#include <cmath>
#include <algorithm>   // std::max

namespace BinaryCKKS
{

Ciphertext Mult(const Ciphertext& c1,
                const Ciphertext& c2,
                const EvalKey&    evk,
                const Params&     par)
{
    /* ---- polynomial part ------------------------------------ */
    NTL::ZZX d0 = c1.b * c2.b;                 // b₁·b₂
    NTL::ZZX d1 = c1.a * c2.b + c2.a * c1.b;   // a₁b₂ + a₂b₁
    NTL::ZZX d2 = c1.a * c2.a;                 // a₁·a₂

    NTL::ZZX res_b = d0 + d2 * evk.b0;         // add relinearisation
    NTL::ZZX res_a = d1 + d2 * evk.a0;

    /* ---- noise bound  (Lemma 3) ------------------------------ */
    long  B1   = c1.B;
    long  B2   = c2.B;
    long  h    = par.h;
    long  BmaxCoeff = (1L << par.lambdaB) - 1;     // B  (upper coeff bound)
    long  sigmaPart = static_cast<long>(std::ceil(6.0 * par.sigma * par.N));

    long  Bmul =  B1 * B2
                + (B1 + B2) * (BmaxCoeff + h)
                + h * B1 * B2
                + h * (B1 + B2)
                + sigmaPart;

    return Ciphertext{res_b, res_a, Bmul};
}

} // namespace BinaryCKKS
