#ifndef SIMPLE_BINARY_CKKS_H
#define SIMPLE_BINARY_CKKS_H

#include <vector>
#include <complex>
#include <random>
#include <cmath>
#include <iostream>
#include <cassert>

#include <NTL/ZZX.h>
#include <NTL/ZZ.h>
#include <NTL/BasicThreadPool.h>

using namespace std;
using namespace NTL;

/**
 * Simplified Binary Polynomial Ring Element
 * Represents polynomials in Z_2[x]/(x^n + 1)
 */
class SimpleBinaryPoly {
private:
    vector<long> coeffs;
    long n; // Ring dimension
    
public:
    SimpleBinaryPoly(long ring_dim = 1024);
    SimpleBinaryPoly(const vector<long>& coefficients, long ring_dim);
    
    // Basic operations
    SimpleBinaryPoly operator+(const SimpleBinaryPoly& other) const;
    SimpleBinaryPoly operator*(const SimpleBinaryPoly& other) const;
    SimpleBinaryPoly& operator+=(const SimpleBinaryPoly& other);
    SimpleBinaryPoly& operator*=(const SimpleBinaryPoly& other);
    
    // Access methods
    long getCoeff(long i) const;
    void setCoeff(long i, long val);
    long getDegree() const;
    vector<long> getCoeffs() const;
    long getRingDim() const { return n; }
    
    void print(int max_terms = 10) const;
    void randomize(mt19937& rng);
};

/**
 * Simple Discrete Gaussian sampler
 */
class SimpleGaussian {
private:
    double sigma;
    mt19937 rng;
    normal_distribution<double> dist;
    
public:
    SimpleGaussian(double sigma_val, unsigned seed = random_device{}());
    long sample();
    vector<long> sampleVector(long n);
};

/**
 * Simple Uniform Binary sampler
 */
class UniformBinary {
private:
    mt19937 rng;
    uniform_int_distribution<int> dist;
    
public:
    UniformBinary(unsigned seed = random_device{}());
    long sample();
    vector<long> sampleVector(long n);
};

/**
 * Hamming Weight sampler
 */
class SimpleHWT {
private:
    mt19937 rng;
    
public:
    SimpleHWT(unsigned seed = random_device{}());
    vector<long> sampleHWT(long n, long h);
};

/**
 * Simplified Binary CKKS Keys
 */
class SimpleBinaryCKKSKeys {
public:
    SimpleBinaryPoly s;      // Secret key
    SimpleBinaryPoly pk_a;   // Public key part a
    SimpleBinaryPoly pk_b;   // Public key part b = -as + e
    SimpleBinaryPoly evk_a;  // Evaluation key part a
    SimpleBinaryPoly evk_b;  // Evaluation key part b = -a*s + e + s^2
    
    SimpleBinaryCKKSKeys(long n = 1024);
    void print() const;
};

/**
 * Simplified Binary CKKS Ciphertext
 */
class SimpleBinaryCKKSCiphertext {
public:
    SimpleBinaryPoly c0;
    SimpleBinaryPoly c1;
    double noise_estimate;
    
    SimpleBinaryCKKSCiphertext(long n = 1024);
    SimpleBinaryCKKSCiphertext(const SimpleBinaryPoly& c0_val, const SimpleBinaryPoly& c1_val, 
                              double noise = 0.0);
    void print() const;
};

/**
 * Simplified Binary CKKS Scheme
 */
class SimpleBinaryCKKS {
private:
    long n;           // Ring dimension
    long h;           // Hamming weight of secret key
    double sigma;     // Gaussian noise parameter
    long lambda;      // Security parameter
    
    // Samplers
    SimpleGaussian* gaussian_sampler;
    UniformBinary* uniform_sampler;
    SimpleHWT* hwt_sampler;
    
public:
    SimpleBinaryCKKS(long security = 128, long ring_dim = 1024);
    ~SimpleBinaryCKKS();
    
    // Core operations
    SimpleBinaryCKKSKeys keyGen();
    
    // Simple encoding/decoding for demonstration
    SimpleBinaryPoly encode(const vector<long>& data);
    vector<long> decode(const SimpleBinaryPoly& poly, long expected_size);
    
    // Encryption/Decryption
    SimpleBinaryCKKSCiphertext encrypt(const SimpleBinaryPoly& plaintext, 
                                      const SimpleBinaryCKKSKeys& keys);
    SimpleBinaryPoly decrypt(const SimpleBinaryCKKSCiphertext& ciphertext, 
                            const SimpleBinaryCKKSKeys& keys);
    
    // Homomorphic operations
    SimpleBinaryCKKSCiphertext add(const SimpleBinaryCKKSCiphertext& ct1, 
                                  const SimpleBinaryCKKSCiphertext& ct2);
    SimpleBinaryCKKSCiphertext multiply(const SimpleBinaryCKKSCiphertext& ct1, 
                                       const SimpleBinaryCKKSCiphertext& ct2,
                                       const SimpleBinaryCKKSKeys& keys);
    
    // Noise management
    bool needsRefresh(const SimpleBinaryCKKSCiphertext& ct, double threshold = 1000.0);
    SimpleBinaryCKKSCiphertext refresh(const SimpleBinaryCKKSCiphertext& ct,
                                      const SimpleBinaryCKKSKeys& old_keys,
                                      const SimpleBinaryCKKSKeys& new_keys);
    
    // Utilities
    void printParameters() const;
    long getRingDim() const { return n; }
    double getNoiseLevel() const { return sigma; }
};

/**
 * Demo and testing utilities
 */
namespace SimpleBinaryCKKSUtils {
    // Test basic operations
    void testBasicOperations();
    
    // Test homomorphic operations
    void testHomomorphicOperations();
    
    // Performance benchmark
    void benchmarkOperations();
    
    // Print vector utility
    template<typename T>
    void printVector(const vector<T>& vec, const string& name, int max_elements = 8);
}

#endif // SIMPLE_BINARY_CKKS_H