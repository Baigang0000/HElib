#ifndef BINARY_CKKS_BCH127_H
#define BINARY_CKKS_BCH127_H
/********************************************************************
 *  Binary BCH (127, 106, 3)                                        *
 *    • length  n = 127                                             *
 *    • dimension k = 106 (21 parity bits)                          *
 *    • can correct up to t = 3 bit errors                          *
 *                                                                  *
 *  Field representation: GF(2)[x]/(x^7 + x + 1)  (primitive poly). *
 *  Generator polynomial (deg 21) for designed-distance 7:          *
 *      g(x) = x^21 + x^19 + x^18 + x^14 + x^13 + x^11 +            *
 *             x^9 + x^8 + x^7 + x^6 + x^4 + x^3 + x^2 + 1          *
 *                                                                  *
 *  The api encodes / decodes codewords in GF2X (least-significant  *
 *  bit = x^0 coefficient).                                         *
 ********************************************************************/

#include <NTL/GF2X.h>
#include <vector>

namespace BinaryCKKS
{

class BCH127
{
public:
    static constexpr long n = 127;   //!< code length
    static constexpr long k = 106;   //!< data bits
    static constexpr long t = 3;     //!< error-correction capability

    /** Return the fixed generator polynomial g(x). */
    static const NTL::GF2X& generator();          // degree 21

    /** Systematic encode: 106-bit -> 127-bit codeword. */
    static NTL::GF2X encode(const NTL::GF2X& data);  // deg(data)<106

    /**
     * Decode received word (Chien + Berlekamp-Massey).
     * @return true  if ≤3 errors corrected and msg out;
     *         false if decoding failed (uncorrectable).
     */
    static bool decode(const NTL::GF2X& recv, NTL::GF2X& msgOut);

    /* convenience wrappers for std::vector<uint8_t> bits ------------ */

    static NTL::GF2X vecToPoly(const std::vector<uint8_t>& bits);
    static std::vector<uint8_t> polyToVec(const NTL::GF2X& poly, long len);
};

/* ------------------------------------------------------------------ */
/* inline helpers (tiny, keep in header for simplicity)               */
/* ------------------------------------------------------------------ */

inline NTL::GF2X BCH127::vecToPoly(const std::vector<uint8_t>& bits)
{
    NTL::GF2X p;
    for (long i = 0; i < (long)bits.size(); ++i)
        if (bits[i] & 1) NTL::SetCoeff(p, i);
    return p;
}

inline std::vector<uint8_t>
BCH127::polyToVec(const NTL::GF2X& poly, long len)
{
    std::vector<uint8_t> v(len, 0);
    for (long i = 0; i < len; ++i)
        if (NTL::IsOne(NTL::coeff(poly, i))) v[i] = 1;
    return v;
}

} // namespace BinaryCKKS
#endif /* BINARY_CKKS_BCH127_H */
