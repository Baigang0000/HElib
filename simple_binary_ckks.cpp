#include "simple_binary_ckks.h"
#include <algorithm>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

// ===================== SimpleBinaryPoly Implementation =====================

SimpleBinaryPoly::SimpleBinaryPoly(long ring_dim) : n(ring_dim) {
    coeffs.resize(n, 0);
}

SimpleBinaryPoly::SimpleBinaryPoly(const vector<long>& coefficients, long ring_dim) 
    : coeffs(coefficients), n(ring_dim) {
    coeffs.resize(n, 0);
}

SimpleBinaryPoly SimpleBinaryPoly::operator+(const SimpleBinaryPoly& other) const {
    assert(n == other.n);
    SimpleBinaryPoly result(n);
    for (long i = 0; i < n; i++) {
        result.coeffs[i] = (coeffs[i] + other.coeffs[i]) % 2;
    }
    return result;
}

SimpleBinaryPoly SimpleBinaryPoly::operator*(const SimpleBinaryPoly& other) const {
    assert(n == other.n);
    SimpleBinaryPoly result(n);
    
    // Polynomial multiplication in Z_2[x]/(x^n + 1)
    for (long i = 0; i < n; i++) {
        for (long j = 0; j < n; j++) {
            if (coeffs[i] && other.coeffs[j]) {
                long pos = (i + j) % n;
                if (i + j >= n) {
                    // x^n = -1 = 1 in Z_2
                    result.coeffs[pos] = (result.coeffs[pos] + 1) % 2;
                } else {
                    result.coeffs[pos] = (result.coeffs[pos] + 1) % 2;
                }
            }
        }
    }
    return result;
}

SimpleBinaryPoly& SimpleBinaryPoly::operator+=(const SimpleBinaryPoly& other) {
    *this = *this + other;
    return *this;
}

SimpleBinaryPoly& SimpleBinaryPoly::operator*=(const SimpleBinaryPoly& other) {
    *this = *this * other;
    return *this;
}

long SimpleBinaryPoly::getCoeff(long i) const {
    return (i >= 0 && i < n) ? coeffs[i] : 0;
}

void SimpleBinaryPoly::setCoeff(long i, long val) {
    if (i >= 0 && i < n) {
        coeffs[i] = val % 2;
    }
}

long SimpleBinaryPoly::getDegree() const {
    for (long i = n - 1; i >= 0; i--) {
        if (coeffs[i] != 0) return i;
    }
    return -1;
}

vector<long> SimpleBinaryPoly::getCoeffs() const {
    return coeffs;
}

void SimpleBinaryPoly::print(int max_terms) const {
    cout << "Poly[" << n << "]: ";
    bool first = true;
    int count = 0;
    
    for (long i = 0; i < n && count < max_terms; i++) {
        if (coeffs[i] != 0) {
            if (!first) cout << " + ";
            if (i == 0) {
                cout << coeffs[i];
            } else if (i == 1) {
                cout << coeffs[i] << "*x";
            } else {
                cout << coeffs[i] << "*x^" << i;
            }
            first = false;
            count++;
        }
    }
    if (first) cout << "0";
    if (count >= max_terms) cout << " + ...";
    cout << endl;
}

void SimpleBinaryPoly::randomize(mt19937& rng) {
    uniform_int_distribution<int> dist(0, 1);
    for (long i = 0; i < n; i++) {
        coeffs[i] = dist(rng);
    }
}

// ===================== SimpleGaussian Implementation =====================

SimpleGaussian::SimpleGaussian(double sigma_val, unsigned seed) 
    : sigma(sigma_val), rng(seed), dist(0.0, sigma_val) {}

long SimpleGaussian::sample() {
    return round(dist(rng));
}

vector<long> SimpleGaussian::sampleVector(long n) {
    vector<long> result(n);
    for (long i = 0; i < n; i++) {
        result[i] = sample();
    }
    return result;
}

// ===================== UniformBinary Implementation =====================

UniformBinary::UniformBinary(unsigned seed) : rng(seed), dist(0, 1) {}

long UniformBinary::sample() {
    return dist(rng);
}

vector<long> UniformBinary::sampleVector(long n) {
    vector<long> result(n);
    for (long i = 0; i < n; i++) {
        result[i] = sample();
    }
    return result;
}

// ===================== SimpleHWT Implementation =====================

SimpleHWT::SimpleHWT(unsigned seed) : rng(seed) {}

vector<long> SimpleHWT::sampleHWT(long n, long h) {
    vector<long> result(n, 0);
    vector<long> positions(n);
    iota(positions.begin(), positions.end(), 0);
    
    shuffle(positions.begin(), positions.end(), rng);
    
    for (long i = 0; i < h && i < n; i++) {
        result[positions[i]] = 1;
    }
    
    return result;
}

// ===================== SimpleBinaryCKKSKeys Implementation =====================

SimpleBinaryCKKSKeys::SimpleBinaryCKKSKeys(long n) : s(n), pk_a(n), pk_b(n), evk_a(n), evk_b(n) {}

void SimpleBinaryCKKSKeys::print() const {
    cout << "=== Binary CKKS Keys ===" << endl;
    cout << "Secret key:" << endl;
    s.print();
    cout << "Public key (a, b):" << endl;
    pk_a.print();
    pk_b.print();
    cout << "Evaluation key (a_0, b_0):" << endl;
    evk_a.print();
    evk_b.print();
}

// ===================== SimpleBinaryCKKSCiphertext Implementation =====================

SimpleBinaryCKKSCiphertext::SimpleBinaryCKKSCiphertext(long n) : c0(n), c1(n), noise_estimate(0.0) {}

SimpleBinaryCKKSCiphertext::SimpleBinaryCKKSCiphertext(const SimpleBinaryPoly& c0_val, 
                                                      const SimpleBinaryPoly& c1_val, double noise)
    : c0(c0_val), c1(c1_val), noise_estimate(noise) {}

void SimpleBinaryCKKSCiphertext::print() const {
    cout << "=== Binary CKKS Ciphertext ===" << endl;
    cout << "c0:" << endl;
    c0.print();
    cout << "c1:" << endl;
    c1.print();
    cout << "Noise estimate: " << noise_estimate << endl;
}

// ===================== SimpleBinaryCKKS Implementation =====================

SimpleBinaryCKKS::SimpleBinaryCKKS(long security, long ring_dim) 
    : n(ring_dim), lambda(security) {
    
    // Set parameters based on security level
    h = security / 2;  // Hamming weight of secret key
    sigma = 3.2;       // Gaussian noise parameter
    
    // Initialize samplers
    gaussian_sampler = new SimpleGaussian(sigma);
    uniform_sampler = new UniformBinary();
    hwt_sampler = new SimpleHWT();
}

SimpleBinaryCKKS::~SimpleBinaryCKKS() {
    delete gaussian_sampler;
    delete uniform_sampler;
    delete hwt_sampler;
}

SimpleBinaryCKKSKeys SimpleBinaryCKKS::keyGen() {
    SimpleBinaryCKKSKeys keys(n);
    
    // Sample secret key s with Hamming weight h
    vector<long> s_coeffs = hwt_sampler->sampleHWT(n, h);
    keys.s = SimpleBinaryPoly(s_coeffs, n);
    
    // Sample a uniformly from binary polynomial ring
    vector<long> a_coeffs = uniform_sampler->sampleVector(n);
    keys.pk_a = SimpleBinaryPoly(a_coeffs, n);
    
    // Sample error e from discrete Gaussian, reduce to binary
    vector<long> e_coeffs = gaussian_sampler->sampleVector(n);
    for (auto& coeff : e_coeffs) coeff = abs(coeff) % 2;
    SimpleBinaryPoly e(e_coeffs, n);
    
    // Compute b = -as + e = as + e (since -1 = 1 in Z_2)
    keys.pk_b = keys.pk_a * keys.s + e;
    
    // Generate evaluation key
    vector<long> a0_coeffs = uniform_sampler->sampleVector(n);
    keys.evk_a = SimpleBinaryPoly(a0_coeffs, n);
    
    vector<long> e0_coeffs = gaussian_sampler->sampleVector(n);
    for (auto& coeff : e0_coeffs) coeff = abs(coeff) % 2;
    SimpleBinaryPoly e0(e0_coeffs, n);
    
    // Compute b0 = -a0*s + e0 + s^2 = a0*s + e0 + s^2
    keys.evk_b = keys.evk_a * keys.s + e0 + keys.s * keys.s;
    
    return keys;
}

SimpleBinaryPoly SimpleBinaryCKKS::encode(const vector<long>& data) {
    SimpleBinaryPoly result(n);
    
    // Simple encoding: just place data values as coefficients (mod 2)
    for (size_t i = 0; i < data.size() && i < (size_t)n; i++) {
        result.setCoeff(i, data[i] % 2);
    }
    
    return result;
}

vector<long> SimpleBinaryCKKS::decode(const SimpleBinaryPoly& poly, long expected_size) {
    vector<long> result(expected_size);
    
    // Simple decoding: extract coefficients
    for (long i = 0; i < expected_size && i < poly.getRingDim(); i++) {
        result[i] = poly.getCoeff(i);
    }
    
    return result;
}

SimpleBinaryCKKSCiphertext SimpleBinaryCKKS::encrypt(const SimpleBinaryPoly& plaintext, 
                                                    const SimpleBinaryCKKSKeys& keys) {
    // Sample v from uniform binary
    vector<long> v_coeffs = uniform_sampler->sampleVector(n);
    SimpleBinaryPoly v(v_coeffs, n);
    
    // Sample e0, e1 from discrete Gaussian, reduce to binary
    vector<long> e0_coeffs = gaussian_sampler->sampleVector(n);
    vector<long> e1_coeffs = gaussian_sampler->sampleVector(n);
    for (auto& coeff : e0_coeffs) coeff = abs(coeff) % 2;
    for (auto& coeff : e1_coeffs) coeff = abs(coeff) % 2;
    SimpleBinaryPoly e0(e0_coeffs, n);
    SimpleBinaryPoly e1(e1_coeffs, n);
    
    // Compute ciphertext: v * pk + (m + e0, e1)
    SimpleBinaryPoly c0 = v * keys.pk_b + plaintext + e0;
    SimpleBinaryPoly c1 = v * keys.pk_a + e1;
    
    return SimpleBinaryCKKSCiphertext(c0, c1, sigma);
}

SimpleBinaryPoly SimpleBinaryCKKS::decrypt(const SimpleBinaryCKKSCiphertext& ciphertext, 
                                          const SimpleBinaryCKKSKeys& keys) {
    // Decrypt: c0 + c1 * s
    return ciphertext.c0 + ciphertext.c1 * keys.s;
}

SimpleBinaryCKKSCiphertext SimpleBinaryCKKS::add(const SimpleBinaryCKKSCiphertext& ct1, 
                                                const SimpleBinaryCKKSCiphertext& ct2) {
    SimpleBinaryPoly c0_sum = ct1.c0 + ct2.c0;
    SimpleBinaryPoly c1_sum = ct1.c1 + ct2.c1;
    double noise_sum = ct1.noise_estimate + ct2.noise_estimate;
    
    return SimpleBinaryCKKSCiphertext(c0_sum, c1_sum, noise_sum);
}

SimpleBinaryCKKSCiphertext SimpleBinaryCKKS::multiply(const SimpleBinaryCKKSCiphertext& ct1, 
                                                     const SimpleBinaryCKKSCiphertext& ct2,
                                                     const SimpleBinaryCKKSKeys& keys) {
    // Tensor product: (c0, c1) * (d0, d1) = (c0*d0, c0*d1 + c1*d0, c1*d1)
    SimpleBinaryPoly d0 = ct1.c0 * ct2.c0;
    SimpleBinaryPoly d1 = ct1.c0 * ct2.c1 + ct1.c1 * ct2.c0;
    SimpleBinaryPoly d2 = ct1.c1 * ct2.c1;
    
    // Key switching: (d0, d1, d2) -> (d0 + d2*evk_b, d1 + d2*evk_a)
    SimpleBinaryPoly c0_mult = d0 + d2 * keys.evk_b;
    SimpleBinaryPoly c1_mult = d1 + d2 * keys.evk_a;
    
    double noise_mult = ct1.noise_estimate * ct2.noise_estimate + sigma;
    
    return SimpleBinaryCKKSCiphertext(c0_mult, c1_mult, noise_mult);
}

bool SimpleBinaryCKKS::needsRefresh(const SimpleBinaryCKKSCiphertext& ct, double threshold) {
    return ct.noise_estimate > threshold;
}

SimpleBinaryCKKSCiphertext SimpleBinaryCKKS::refresh(const SimpleBinaryCKKSCiphertext& ct,
                                                    const SimpleBinaryCKKSKeys& old_keys,
                                                    const SimpleBinaryCKKSKeys& new_keys) {
    // Decrypt under old key
    SimpleBinaryPoly m = decrypt(ct, old_keys);
    
    // Encrypt under new key
    return encrypt(m, new_keys);
}

void SimpleBinaryCKKS::printParameters() const {
    cout << "=== Simple Binary CKKS Parameters ===" << endl;
    cout << "Security parameter (lambda): " << lambda << endl;
    cout << "Ring dimension (n): " << n << endl;
    cout << "Hamming weight (h): " << h << endl;
    cout << "Gaussian sigma: " << sigma << endl;
    cout << "===============================" << endl;
}

// ===================== Utility Functions =====================

namespace SimpleBinaryCKKSUtils {

template<typename T>
void printVector(const vector<T>& vec, const string& name, int max_elements) {
    cout << name << ": [";
    for (size_t i = 0; i < vec.size() && i < (size_t)max_elements; i++) {
        cout << vec[i];
        if (i < vec.size() - 1 && i < (size_t)max_elements - 1) cout << ", ";
    }
    if (vec.size() > (size_t)max_elements) cout << ", ...";
    cout << "]" << endl;
}

// Explicit instantiation for common types
template void printVector<long>(const vector<long>&, const string&, int);
template void printVector<int>(const vector<int>&, const string&, int);
template void printVector<double>(const vector<double>&, const string&, int);

void testBasicOperations() {
    cout << "\n=== Testing Basic Operations ===" << endl;
    
    // Test polynomial operations
    SimpleBinaryPoly p1({1, 0, 1, 1, 0}, 8);
    SimpleBinaryPoly p2({0, 1, 1, 0, 1}, 8);
    
    cout << "Polynomial 1: ";
    p1.print();
    cout << "Polynomial 2: ";
    p2.print();
    
    SimpleBinaryPoly sum = p1 + p2;
    cout << "Sum: ";
    sum.print();
    
    SimpleBinaryPoly product = p1 * p2;
    cout << "Product: ";
    product.print();
    
    cout << "Basic operations test completed!\n" << endl;
}

void testHomomorphicOperations() {
    cout << "\n=== Testing Homomorphic Operations ===" << endl;
    
    SimpleBinaryCKKS scheme(64, 256); // Smaller parameters for demo
    scheme.printParameters();
    
    // Generate keys
    cout << "Generating keys..." << endl;
    auto start = high_resolution_clock::now();
    SimpleBinaryCKKSKeys keys = scheme.keyGen();
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Key generation completed in " << duration.count() << " ms" << endl;
    
    // Prepare test data
    vector<long> data1 = {1, 0, 1, 1, 0, 1, 0, 0};
    vector<long> data2 = {0, 1, 1, 0, 1, 0, 1, 1};
    
    printVector(data1, "Data 1");
    printVector(data2, "Data 2");
    
    // Encode
    SimpleBinaryPoly encoded1 = scheme.encode(data1);
    SimpleBinaryPoly encoded2 = scheme.encode(data2);
    
    // Encrypt
    cout << "Encrypting data..." << endl;
    start = high_resolution_clock::now();
    SimpleBinaryCKKSCiphertext ct1 = scheme.encrypt(encoded1, keys);
    SimpleBinaryCKKSCiphertext ct2 = scheme.encrypt(encoded2, keys);
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start);
    cout << "Encryption completed in " << duration.count() << " ms" << endl;
    
    // Homomorphic addition
    cout << "Performing homomorphic addition..." << endl;
    start = high_resolution_clock::now();
    SimpleBinaryCKKSCiphertext ct_add = scheme.add(ct1, ct2);
    end = high_resolution_clock::now();
    auto add_duration = duration_cast<microseconds>(end - start);
    cout << "Addition completed in " << add_duration.count() << " μs" << endl;
    
    // Homomorphic multiplication
    cout << "Performing homomorphic multiplication..." << endl;
    start = high_resolution_clock::now();
    SimpleBinaryCKKSCiphertext ct_mult = scheme.multiply(ct1, ct2, keys);
    end = high_resolution_clock::now();
    auto mult_duration = duration_cast<milliseconds>(end - start);
    cout << "Multiplication completed in " << mult_duration.count() << " ms" << endl;
    
    // Decrypt and decode results
    cout << "Decrypting results..." << endl;
    SimpleBinaryPoly decrypted_add = scheme.decrypt(ct_add, keys);
    SimpleBinaryPoly decrypted_mult = scheme.decrypt(ct_mult, keys);
    
    vector<long> result_add = scheme.decode(decrypted_add, data1.size());
    vector<long> result_mult = scheme.decode(decrypted_mult, data1.size());
    
    // Compute expected results
    vector<long> expected_add(data1.size()), expected_mult(data1.size());
    for (size_t i = 0; i < data1.size(); i++) {
        expected_add[i] = (data1[i] + data2[i]) % 2;
        expected_mult[i] = (data1[i] * data2[i]) % 2;
    }
    
    // Display results
    cout << "\n=== Results ===" << endl;
    printVector(expected_add, "Expected Addition");
    printVector(result_add, "HE Addition Result");
    
    printVector(expected_mult, "Expected Multiplication");
    printVector(result_mult, "HE Multiplication Result");
    
    // Check correctness
    bool add_correct = (expected_add == result_add);
    bool mult_correct = (expected_mult == result_mult);
    
    cout << "\nCorrectness check:" << endl;
    cout << "Addition: " << (add_correct ? "✓ PASS" : "✗ FAIL") << endl;
    cout << "Multiplication: " << (mult_correct ? "✓ PASS" : "✗ FAIL") << endl;
    
    cout << "\nNoise levels:" << endl;
    cout << "Addition result: " << ct_add.noise_estimate << endl;
    cout << "Multiplication result: " << ct_mult.noise_estimate << endl;
    
    cout << "Homomorphic operations test completed!\n" << endl;
}

void benchmarkOperations() {
    cout << "\n=== Performance Benchmark ===" << endl;
    
    SimpleBinaryCKKS scheme(128, 512);
    SimpleBinaryCKKSKeys keys = scheme.keyGen();
    
    vector<long> data1 = {1, 0, 1, 1, 0, 1, 0, 0};
    vector<long> data2 = {0, 1, 1, 0, 1, 0, 1, 1};
    
    SimpleBinaryPoly encoded1 = scheme.encode(data1);
    SimpleBinaryPoly encoded2 = scheme.encode(data2);
    
    SimpleBinaryCKKSCiphertext ct1 = scheme.encrypt(encoded1, keys);
    SimpleBinaryCKKSCiphertext ct2 = scheme.encrypt(encoded2, keys);
    
    const int num_tests = 100;
    
    // Benchmark addition
    auto start = high_resolution_clock::now();
    for (int i = 0; i < num_tests; i++) {
        SimpleBinaryCKKSCiphertext ct_add = scheme.add(ct1, ct2);
        (void)ct_add; // Suppress unused variable warning
    }
    auto end = high_resolution_clock::now();
    auto add_duration = duration_cast<microseconds>(end - start);
    
    // Benchmark multiplication
    start = high_resolution_clock::now();
    for (int i = 0; i < num_tests; i++) {
        SimpleBinaryCKKSCiphertext ct_mult = scheme.multiply(ct1, ct2, keys);
        (void)ct_mult; // Suppress unused variable warning
    }
    end = high_resolution_clock::now();
    auto mult_duration = duration_cast<microseconds>(end - start);
    
    cout << "Performance Results (" << num_tests << " operations):" << endl;
    cout << "Average addition time: " << add_duration.count() / num_tests << " μs" << endl;
    cout << "Average multiplication time: " << mult_duration.count() / num_tests << " μs" << endl;
    
    cout << "Benchmark completed!\n" << endl;
}

} // namespace SimpleBinaryCKKSUtils