#ifndef BINARY_CKKS_H
#define BINARY_CKKS_H

#include <vector>
#include <complex>
#include <random>
#include <cmath>
#include <iostream>

#include <helib/helib.h>
#include <helib/Context.h>
#include <helib/EncryptedArray.h>
#include <helib/Ctxt.h>
#include <helib/keys.h>
#include <helib/FHE.h>
#include <helib/polyArith.h>
#include <helib/NumbTh.h>

using namespace helib;
using namespace std;
using namespace NTL;

// Forward declarations
class BinaryCKKSKeys;
class BinaryCKKSCiphertext;

/**
 * Binary Polynomial Ring Element
 * Represents polynomials in Z_2[x]/(x^n + 1)
 */
class BinaryPoly {
private:
    vector<long> coeffs;
    long degree_bound;
    
public:
    BinaryPoly(long deg_bound = 0);
    BinaryPoly(const vector<long>& coefficients, long deg_bound);
    
    // Basic operations
    BinaryPoly operator+(const BinaryPoly& other) const;
    BinaryPoly operator*(const BinaryPoly& other) const;
    BinaryPoly& operator+=(const BinaryPoly& other);
    BinaryPoly& operator*=(const BinaryPoly& other);
    
    // Access methods
    long getCoeff(long i) const;
    void setCoeff(long i, long val);
    long getDegree() const;
    vector<long> getCoeffs() const;
    
    // Conversion methods
    void fromIntPoly(const ZZX& intPoly, long B); // Convert from integer polynomial with binary expansion
    ZZX toIntPoly() const; // Convert back to integer polynomial
    
    void print() const;
};

/**
 * Discrete Gaussian sampler
 */
class DiscreteGaussian {
private:
    double sigma;
    mt19937 rng;
    normal_distribution<double> dist;
    
public:
    DiscreteGaussian(double sigma_val, unsigned seed = 0);
    long sample();
    vector<long> sampleVector(long n);
};

/**
 * Hamming Weight Sampler
 */
class HammingWeightSampler {
private:
    mt19937 rng;
    
public:
    HammingWeightSampler(unsigned seed = 0);
    vector<long> sampleHWT(long n, long h); // Sample vector of length n with Hamming weight h
};

/**
 * Zero-One sampler (Bernoulli with p=0.5)
 */
class ZeroOneSampler {
private:
    mt19937 rng;
    bernoulli_distribution dist;
    
public:
    ZeroOneSampler(unsigned seed = 0);
    long sample();
    vector<long> sampleVector(long n);
};

/**
 * Binary CKKS Key Generation, Encryption, Decryption, and Operations
 */
class BinaryCKKS {
private:
    // Parameters
    long M;          // Cyclotomic polynomial parameter
    long p;          // Prime (set to 2 for binary)
    long r;          // Lifting parameter  
    long L;          // Number of levels
    long h;          // Hamming weight of secret key
    long P;          // Evaluation key parameter
    double sigma;    // Gaussian noise parameter
    long security;   // Security parameter
    
    // HElib context and helpers
    helib::Context* context;
    helib::EncryptedArray* ea;
    ZZX G;
    
    // Samplers
    DiscreteGaussian* dg_sampler;
    HammingWeightSampler* hwt_sampler;
    ZeroOneSampler* zo_sampler;
    
public:
    BinaryCKKS(long lambda = 128);
    ~BinaryCKKS();
    
    // Parameter selection
    void chooseParameters(long lambda);
    
    // Key Generation
    BinaryCKKSKeys keyGen();
    
    // Encoding/Decoding
    BinaryPoly encode(const vector<complex<double>>& z, double Delta);
    vector<complex<double>> decode(const BinaryPoly& m, double Delta);
    
    // Encryption/Decryption  
    BinaryCKKSCiphertext encrypt(const BinaryPoly& m, const BinaryCKKSKeys& keys);
    BinaryPoly decrypt(const BinaryCKKSCiphertext& c, const BinaryCKKSKeys& keys);
    
    // Homomorphic Operations
    BinaryCKKSCiphertext add(const BinaryCKKSCiphertext& c1, const BinaryCKKSCiphertext& c2);
    BinaryCKKSCiphertext multiply(const BinaryCKKSCiphertext& c1, const BinaryCKKSCiphertext& c2, 
                                  const BinaryCKKSKeys& keys);
    
    // Noise estimation and thresholding
    bool threshold(double B_max, double B_0);
    
    // Refresh operation
    BinaryCKKSCiphertext refresh(const BinaryCKKSCiphertext& c, 
                                const BinaryCKKSKeys& old_keys,
                                const BinaryCKKSKeys& new_keys,
                                double Delta);
    
    // Utilities
    long getSlots() const;
    void printParameters() const;
};

/**
 * Key structure for Binary CKKS
 */
class BinaryCKKSKeys {
public:
    // Secret key: (1, s) where s has Hamming weight h
    BinaryPoly s;
    
    // Public key: (b, a) where b = -as + e
    BinaryPoly pk_a;
    BinaryPoly pk_b;
    
    // Evaluation key: (b_0, a_0) where b_0 = -a_0*s + e_0 + s^2
    BinaryPoly evk_a;
    BinaryPoly evk_b;
    
    BinaryCKKSKeys();
    void print() const;
};

/**
 * Ciphertext structure for Binary CKKS
 */
class BinaryCKKSCiphertext {
public:
    BinaryPoly c0; // First component
    BinaryPoly c1; // Second component
    double noise_estimate; // Estimated noise level
    
    BinaryCKKSCiphertext();
    BinaryCKKSCiphertext(const BinaryPoly& c0_val, const BinaryPoly& c1_val, double noise = 0.0);
    
    void print() const;
};

/**
 * Canonical embedding utilities
 */
class CanonicalEmbedding {
private:
    long M;
    vector<long> T; // Index set for primitive M-th roots of unity
    
public:
    CanonicalEmbedding(long M_val);
    
    // pi: H -> C^{N/2} (canonical embedding)
    vector<complex<double>> embed(const vector<complex<double>>& h);
    
    // pi^{-1}: C^{N/2} -> H (inverse canonical embedding)
    vector<complex<double>> embedInverse(const vector<complex<double>>& z);
    
    // sigma: R -> C^N (coefficient embedding)  
    vector<complex<double>> coeffEmbed(const ZZX& poly);
    
    // sigma^{-1}: C^N -> R (inverse coefficient embedding)
    ZZX coeffEmbedInverse(const vector<complex<double>>& vals);
    
    long getSlots() const { return T.size(); }
    vector<long> getIndexSet() const { return T; }
};

#endif // BINARY_CKKS_H