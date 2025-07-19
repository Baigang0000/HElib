#include "BCH127.h"
#include <NTL/GF2E.h>
#include <NTL/GF2EX.h>

namespace BinaryCKKS
{
/* =================================================================
 *  GF(2^7) setup : p(x) = x^7 + x + 1  (0x83)
 * ================================================================= */
static void gf_init()
{
    static bool done = false;
    if (done) return;
    NTL::GF2X P;
    NTL::SetCoeff(P, 7);
    NTL::SetCoeff(P, 1);
    NTL::SetCoeff(P, 0);
    NTL::GF2E::init(P);
    done = true;
}

/* =================================================================
 *  Fixed generator polynomial g(x)   (degree 21)
 * ================================================================= */
static const NTL::GF2X& get_g()
{
    static NTL::GF2X g;
    static bool init = false;
    if (init) return g;

    // explicit bits of g(x)
    const long ones[] = {21,19,18,14,13,11,9,8,7,6,4,3,2,0};
    for (long idx : ones)
        NTL::SetCoeff(g, idx);
    init = true;
    return g;
}

const NTL::GF2X& BCH127::generator()
{
    return get_g();
}

/* =================================================================
 *  Systematic encode : c(x) = d(x) x^{21} + r(x)
 *  where r = remainder(d x^{21} / g)
 * ================================================================= */
NTL::GF2X BCH127::encode(const NTL::GF2X& data)
{
    if (NTL::deg(data) >= k)
        throw std::runtime_error("BCH127::encode: data deg >= 106");

    const long shift = n - k;            // 21
    NTL::GF2X shifted;
    NTL::LeftShift(shifted, data, shift); // d(x) x^{21}

    NTL::GF2X q;                            // quotient (ignored)
    NTL::GF2X rem;
    NTL::DivRem(q, rem, shifted, generator());

    NTL::GF2X code = shifted + rem;      // (+) is XOR in GF(2)
    return code;
}

/* =================================================================
 *  Full algebraic decoder  (≤3 errors)                             *
 * ================================================================= */
namespace {

/* -------------------- syndrome computation ---------------------- */
void computeSyndromes(const NTL::GF2X& recv,
                      NTL::vec_GF2E&  S)      // length 2t = 6
{
    gf_init();
    NTL::GF2E alpha;          // primitive element  α = x
    NTL::GF2X  Xpoly;
    NTL::SetCoeff(Xpoly, 1);  // Xpoly = x
    conv(alpha, Xpoly);       // alpha ← x  in GF(2^7)


    S.SetLength(6);
    for (long j = 1; j <= 6; ++j)
    {
        NTL::GF2E Sj;
        NTL::GF2E alpha_pow = power(alpha, j);  // α^j
        NTL::GF2E term = NTL::GF2E::zero();

    for (long i = 0; i < BCH127::n; ++i)
    if (NTL::IsOne(NTL::coeff(recv, i)))
        term += power(alpha_pow, i);

    }
}

/* -------------------- Berlekamp–Massey -------------------------- */
NTL::GF2EX berlekampMassey(const NTL::vec_GF2E& S, long t)
{
    NTL::GF2EX C, B;
    C.SetLength(1);    SetCoeff(C, 0, 1);  // C(x) = 1
    B = C;

    long L = 0;
    long m = 1;
    NTL::GF2E b;           // b ← 1  in GF(2^7)
    conv(b, 1L);

    const long nS = S.length();
    for (long n = 0; n < nS; ++n)
    {
        /* discrepancy d */
        NTL::GF2E d = S[n];
        for (long i = 1; i <= L; ++i)
            d += coeff(C, i) * S[n-i];

        if (IsZero(d))
        {
            ++m;
        } else {
            NTL::GF2EX T = C;
            NTL::GF2E inv_b = b; inv_b = inv(inv_b);
            C -= (d * inv_b) * (B << m);

            if (2*L <= n) {
                L  = n + 1 - L;
                B  = T;
                b  = d;
                m  = 1;
            } else ++m;
        }
    }
    return C;   // σ(x)
}

/* -------------------- Chien search for up to 3 roots ------------ */
bool chienSearch(const NTL::GF2EX& sigma,
                 std::vector<long>& errorPos)   // indices 0..126
{
    gf_init();
    NTL::GF2E alpha;          // α = x  (primitive element)
    NTL::GF2X  Xpoly;
    NTL::SetCoeff(Xpoly, 1);  // Xpoly = x
    conv(alpha, Xpoly);       // alpha ← x in GF(2^7)


    errorPos.clear();
    for (long i = 0; i < BCH127::n; ++i)
    {
        NTL::GF2E val = eval(sigma, power(alpha, i));
        if (IsZero(val))
            errorPos.push_back((BCH127::n - 1 - i) % BCH127::n);
    }
    return (long)errorPos.size() == deg(sigma);
}

/* -------------------- flip bits --------------------------------- */
/* --------------------------------------------------------------- */
/* Toggle each listed bit position in a GF2X                       */
/* --------------------------------------------------------------- */
static void flipBits(NTL::GF2X& poly, const std::vector<long>& pos)
{
    for (long idx : pos)
    {
        NTL::GF2X mask;              // mask = x^{idx}
        NTL::SetCoeff(mask, idx);
        poly += mask;                // XOR → flips that coefficient
    }
}

} // anon namespace


/* =================================================================
 *  public decode()                                                 *
 * ================================================================= */
bool BCH127::decode(const NTL::GF2X& recv, NTL::GF2X& msgOut)
{
    if (deg(recv) >= n)
        throw std::runtime_error("BCH127::decode: deg(recv) ≥ 127");

    /* ---------- 1. syndromes ---------- */
    NTL::vec_GF2E S;
    computeSyndromes(recv, S);

    bool allZero = true;
    for (long i = 0; i < S.length(); ++i)
        if (!IsZero(S[i])) { allZero = false; break; }
    if (allZero) {
        msgOut = RightShift(recv, n - k);   // strip parity
        trunc(msgOut, k);
        return true;
    }

    /* ---------- 2. error locator σ(x) -- */
    NTL::GF2EX sigma = berlekampMassey(S, t);
    long L = deg(sigma);
    if (L < 0 || L > t)          // failed
        return false;

    /* ---------- 3. find roots --------- */
    std::vector<long> errPos;
    if (!chienSearch(sigma, errPos))
        return false;            // root count mismatch

    /* ---------- 4. correct word ------- */
    NTL::GF2X corrected = recv;
    flipBits(corrected, errPos);

    /* ---------- 5. check parity ------- */
    NTL::GF2X q2, rem;
    NTL::DivRem(q2, rem, corrected, generator());

    if (!IsZero(rem))
        return false;            // uncorrectable

    /* ---------- 6. output message ----- */
    msgOut = RightShift(corrected, n - k);   // drop parity (low 21 bits)
    trunc(msgOut, k);
    return true;
}

} // namespace BinaryCKKS



