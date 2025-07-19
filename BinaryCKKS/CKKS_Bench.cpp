/*********************************************************************
 *  Compare HElib CKKS vs. Binary-CKKS complexity across N            *
 *********************************************************************/
#include "ComplexityBench.h"
#include "AddBP.h"        // Binary-CKKS homomorphic addition

/* ---- HElib CKKS ---- */
#include <helib/helib.h>

/* ---- Binary-CKKS headers ---- */
#include "CiphertextBP.h"
#include "KeyGenBP.h"
#include "EncodeBP.h"
#include "EncryptBP.h"
#include "DecryptBP.h"
#include "MultBP.h"
#include "RefreshBP.h"

using std::cout;
using namespace helib;
using namespace BinaryCKKS;

/* ----------------------------------------------------------------- */
Timings benchHELCKKS(long N)
{
    // CKKS parameters (very small mod chain just to measure timing)
    long m   = 2*N;                      // Cyclotomic index
    long bits = 30;                      // size of prime in chain
    long c   = 2;

    Context context = ContextBuilder<CKKS>()
                        .m(m)
                        .bits(bits)
                        .precision(30)
                        .c(c)
                        .build();

    Timings tm;

    tm.keygen = benchRepeated([&]{
        SecKey sk(context);
        sk.GenSecKey();
        PubKey pk(sk);
    });

    SecKey sk(context);
    sk.GenSecKey();
    PubKey pk(sk);
    PtxtArray ptxt(context, std::vector<double>(N/2, 1.23));
    Ctxt ctxt(pk);


    tm.encrypt = benchRepeated([&]{
    Ctxt tmp(pk);                       // tmp must be fresh each repeat
    ptxt.encrypt(tmp);
    });

    ptxt.encrypt(ctxt);

    tm.decrypt = benchRepeated([&]{
    PtxtArray tmp(context);     // output container
    tmp.decrypt(ctxt, sk);      // fill it
    });


    Ctxt c2 = ctxt;

    tm.add = benchRepeated([&]{
        Ctxt tmp = ctxt;
        tmp += c2;
    });

    tm.mult = benchRepeated([&]{
        Ctxt tmp = ctxt;
        tmp *= c2;
    });

    return tm;
}

/* ----------------------------------------------------------------- */
Timings benchBinary(long N, long lambdaB = 32)
{
    Params par;
    par.N       = N;
    par.lambdaB = lambdaB;   // log2 B
    par.h       = 64;
    par.sigma   = 3.2;

    Timings tm;

    tm.keygen = benchRepeated([&]{
        auto kp = KeyGen(par);
    });

    auto kp = KeyGen(par);

    NTL::ZZX mPoly;                        // trivial plaintext (all zero)
    Ciphertext ct;

    tm.encrypt = benchRepeated([&]{
        ct = Encrypt(kp.pk, mPoly, par);
    });
    ct = Encrypt(kp.pk, mPoly, par);

    tm.decrypt = benchRepeated([&]{
        Decrypt(kp.sk, ct);
    });

    Ciphertext ct2 = ct;

    tm.add = benchRepeated([&]{
        Ciphertext tmp = Add(ct, ct2);
    });

    tm.mult = benchRepeated([&]{
        Ciphertext tmp = Mult(ct, ct2, kp.evk, par);
    });

    return tm;
}

/* ----------------------------------------------------------------- */
int main()
{
    std::vector<long> dims = {1024, 2048, 4096, 8192};

    cout << "N,scheme,keygen,encrypt,decrypt,add,mult (micro-seconds)\n";

    for (long N : dims)
    {
        Timings tCKKS  = benchHELCKKS(N);
        Timings tBin   = benchBinary(N);

        auto print = [&](const char* name, const Timings& t)
        {
            cout << N << ',' << name << ','
                 << t.keygen  << ',' << t.encrypt << ','
                 << t.decrypt << ',' << t.add     << ','
                 << t.mult    << '\n';
        };

        print("stdCKKS", tCKKS);
        print("binCKKS", tBin);
    }
}
