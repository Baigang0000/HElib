#include "CiphertextBP.h"
#include <iostream>
#include <algorithm>   // std::max
#include <cstdlib>     // std::abs

namespace BinaryCKKS
{

/* ---------- internal helper ---------- */
static long maxAbsCoeff(const NTL::ZZX& poly)
{
    long B = 0;
    for (long i = 0; i <= NTL::deg(poly); ++i)
        if (!NTL::IsZero(NTL::coeff(poly, i)))
            B = std::max(B, std::abs(NTL::to_long(NTL::coeff(poly, i))));
    return B;
}

/* ---------- factory & helpers ---------- */
Ciphertext Ciphertext::zero()
{
    return Ciphertext(NTL::ZZX::zero(), NTL::ZZX::zero(), 0);
}

Ciphertext Ciphertext::add(const Ciphertext& c1, const Ciphertext& c2)
{
    Ciphertext r;
    r.b = c1.b + c2.b;
    r.a = c1.a + c2.a;
    r.B = c1.B + c2.B;           // absolute errors add
    return r;
}

Ciphertext Ciphertext::mulConst(const Ciphertext& c, long k)
{
    Ciphertext r;
    r.b = c.b * k;
    r.a = c.a * k;
    r.B = std::abs(k) * c.B;
    return r;
}

void Ciphertext::debugPrint(long maxTerms) const
{
    auto printPoly = [&](const NTL::ZZX& p, const char* tag)
    {
        std::cout << tag << ": ";
        long shown = 0;
        for (long i = 0; i <= NTL::deg(p) && shown < maxTerms; ++i)
            if (!NTL::IsZero(NTL::coeff(p, i))) {
                std::cout << NTL::coeff(p, i) << "*x^" << i << " ";
                ++shown;
            }
        if (NTL::deg(p) + 1 > maxTerms) std::cout << "...";
        std::cout << '\n';
    };

    printPoly(b, "b(x)");
    printPoly(a, "a(x)");
    std::cout << "B = " << B << std::endl;
}

} // namespace BinaryCKKS
