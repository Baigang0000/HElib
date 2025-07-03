#include "binary_ckks.h"

// ===================== BinaryPoly Implementation =====================

BinaryPoly::BinaryPoly(long deg_bound) : degree_bound(deg_bound) {
    coeffs.resize(deg_bound, 0);
}

BinaryPoly::BinaryPoly(const vector<long>& coefficients, long deg_bound) 
    : coeffs(coefficients), degree_bound(deg_bound) {
    coeffs.resize(deg_bound, 0);
}

BinaryPoly BinaryPoly::operator+(const BinaryPoly& other) const {
    BinaryPoly result(max(degree_bound, other.degree_bound));
    for (long i = 0; i < result.degree_bound; i++) {
        long a = (i < coeffs.size()) ? coeffs[i] : 0;
        long b = (i < other.coeffs.size()) ? other.coeffs[i] : 0;
        result.coeffs[i] = (a + b) % 2; // Binary addition (XOR)
    }
    return result;
}

BinaryPoly BinaryPoly::operator*(const BinaryPoly& other) const {
    long result_deg = degree_bound;
    BinaryPoly result(result_deg);
    
    // Polynomial multiplication in Z_2[x]/(x^n + 1)
    for (long i = 0; i < coeffs.size(); i++) {
        for (long j = 0; j < other.coeffs.size(); j++) {
            if (coeffs[i] && other.coeffs[j]) {
                long pos = (i + j) % result_deg;
                if (i + j >= result_deg) {
                    // x^n = -1 in the quotient ring, but in Z_2, -1 = 1
                    result.coeffs[pos] = (result.coeffs[pos] + 1) % 2;
                } else {
                    result.coeffs[pos] = (result.coeffs[pos] + 1) % 2;
                }
            }
        }
    }
    return result;
}

BinaryPoly& BinaryPoly::operator+=(const BinaryPoly& other) {
    *this = *this + other;
    return *this;
}

BinaryPoly& BinaryPoly::operator*=(const BinaryPoly& other) {
    *this = *this * other;
    return *this;
}

long BinaryPoly::getCoeff(long i) const {
    return (i < coeffs.size()) ? coeffs[i] : 0;
}

void BinaryPoly::setCoeff(long i, long val) {
    if (i < coeffs.size()) {
        coeffs[i] = val % 2;
    }
}

long BinaryPoly::getDegree() const {
    for (long i = coeffs.size() - 1; i >= 0; i--) {
        if (coeffs[i] != 0) return i;
    }
    return -1;
}

vector<long> BinaryPoly::getCoeffs() const {
    return coeffs;
}

void BinaryPoly::fromIntPoly(const ZZX& intPoly, long B) {
    // Convert integer polynomial to binary polynomial representation
    long log_B = ceil(log2(B));
    coeffs.clear();
    coeffs.resize(degree_bound, 0);
    
    for (long i = 0; i <= deg(intPoly); i++) {
        ZZ coeff = coeff(intPoly, i);
        long int_coeff = to_long(coeff % B);
        
        // Binary expansion of coefficient
        for (long j = 0; j < log_B; j++) {
            long bit_pos = i * log_B + j;
            if (bit_pos < degree_bound) {
                coeffs[bit_pos] = (int_coeff >> j) & 1;
            }
        }
    }
}

ZZX BinaryPoly::toIntPoly() const {
    // Convert binary polynomial back to integer polynomial
    ZZX result;
    long log_B = 8; // Assume 8-bit coefficients for reconstruction
    
    for (long i = 0; i < coeffs.size(); i += log_B) {
        long coeff_val = 0;
        for (long j = 0; j < log_B && (i + j) < coeffs.size(); j++) {
            coeff_val += coeffs[i + j] << j;
        }
        if (coeff_val != 0) {
            SetCoeff(result, i / log_B, coeff_val);
        }
    }
    return result;
}

void BinaryPoly::print() const {
    cout << "BinaryPoly: ";
    for (long i = 0; i < min((long)coeffs.size(), 20L); i++) {
        cout << coeffs[i];
    }
    if (coeffs.size() > 20) cout << "...";
    cout << endl;
}

// ===================== DiscreteGaussian Implementation =====================

DiscreteGaussian::DiscreteGaussian(double sigma_val, unsigned seed) 
    : sigma(sigma_val), rng(seed), dist(0.0, sigma_val) {}

long DiscreteGaussian::sample() {
    return round(dist(rng));
}

vector<long> DiscreteGaussian::sampleVector(long n) {
    vector<long> result(n);
    for (long i = 0; i < n; i++) {
        result[i] = sample();
    }
    return result;
}

// ===================== HammingWeightSampler Implementation =====================

HammingWeightSampler::HammingWeightSampler(unsigned seed) : rng(seed) {}

vector<long> HammingWeightSampler::sampleHWT(long n, long h) {
    vector<long> result(n, 0);
    vector<long> positions(n);
    iota(positions.begin(), positions.end(), 0);
    
    shuffle(positions.begin(), positions.end(), rng);
    
    for (long i = 0; i < h && i < n; i++) {
        result[positions[i]] = 1;
    }
    
    return result;
}

// ===================== ZeroOneSampler Implementation =====================

ZeroOneSampler::ZeroOneSampler(unsigned seed) : rng(seed), dist(0.5) {}

long ZeroOneSampler::sample() {
    return dist(rng) ? 1 : 0;
}

vector<long> ZeroOneSampler::sampleVector(long n) {
    vector<long> result(n);
    for (long i = 0; i < n; i++) {
        result[i] = sample();
    }
    return result;
}

// ===================== CanonicalEmbedding Implementation =====================

CanonicalEmbedding::CanonicalEmbedding(long M_val) : M(M_val) {
    // Compute set T of indices coprime to M
    for (long i = 1; i < M; i++) {
        if (GCD(i, M) == 1) {
            T.push_back(i);
        }
    }
}

vector<complex<double>> CanonicalEmbedding::embed(const vector<complex<double>>& h) {
    // Canonical embedding pi: H -> C^{N/2}
    vector<complex<double>> result(T.size());
    for (size_t i = 0; i < T.size(); i++) {
        result[i] = h[i]; // Simplified - actual implementation would use roots of unity
    }
    return result;
}

vector<complex<double>> CanonicalEmbedding::embedInverse(const vector<complex<double>>& z) {
    // Inverse canonical embedding pi^{-1}: C^{N/2} -> H
    vector<complex<double>> result(z.size());
    for (size_t i = 0; i < z.size(); i++) {
        result[i] = z[i]; // Simplified - actual implementation would use inverse roots of unity
    }
    return result;
}

vector<complex<double>> CanonicalEmbedding::coeffEmbed(const ZZX& poly) {
    // Coefficient embedding sigma: R -> C^N
    vector<complex<double>> result(M);
    for (long i = 0; i <= deg(poly) && i < M; i++) {
        result[i] = complex<double>(to_double(coeff(poly, i)), 0.0);
    }
    return result;
}

ZZX CanonicalEmbedding::coeffEmbedInverse(const vector<complex<double>>& vals) {
    // Inverse coefficient embedding sigma^{-1}: C^N -> R
    ZZX result;
    for (size_t i = 0; i < vals.size(); i++) {
        if (abs(vals[i]) > 1e-10) {
            SetCoeff(result, i, round(vals[i].real()));
        }
    }
    return result;
}

// ===================== BinaryCKKSKeys Implementation =====================

BinaryCKKSKeys::BinaryCKKSKeys() {}

void BinaryCKKSKeys::print() const {
    cout << "=== Binary CKKS Keys ===" << endl;
    cout << "Secret key s:" << endl;
    s.print();
    cout << "Public key (a, b):" << endl;
    pk_a.print();
    pk_b.print();
    cout << "Evaluation key (a_0, b_0):" << endl;
    evk_a.print();
    evk_b.print();
}

// ===================== BinaryCKKSCiphertext Implementation =====================

BinaryCKKSCiphertext::BinaryCKKSCiphertext() : noise_estimate(0.0) {}

BinaryCKKSCiphertext::BinaryCKKSCiphertext(const BinaryPoly& c0_val, const BinaryPoly& c1_val, double noise)
    : c0(c0_val), c1(c1_val), noise_estimate(noise) {}

void BinaryCKKSCiphertext::print() const {
    cout << "=== Binary CKKS Ciphertext ===" << endl;
    cout << "c0:" << endl;
    c0.print();
    cout << "c1:" << endl;
    c1.print();
    cout << "Noise estimate: " << noise_estimate << endl;
}

// ===================== BinaryCKKS Implementation =====================

BinaryCKKS::BinaryCKKS(long lambda) : security(lambda), context(nullptr), ea(nullptr) {
    chooseParameters(lambda);
    
    // Initialize samplers
    dg_sampler = new DiscreteGaussian(sigma);
    hwt_sampler = new HammingWeightSampler();
    zo_sampler = new ZeroOneSampler();
    
    // Initialize HElib context
    context = new helib::Context(helib::ContextBuilder<helib::BGV>()
                                    .m(M)
                                    .p(2)  // Binary field
                                    .r(r)
                                    .bits(L * 20)  // Approximate bit count
                                    .c(3)
                                    .build());
    
    context->buildModChain(L, 3);
    
    // Get the factorization polynomial for CKKS-like operations
    if (!context->getAlMod().getFactorsOverZZ().empty()) {
        G = context->getAlMod().getFactorsOverZZ()[0];
    }
    
    ea = new helib::EncryptedArray(*context, G);
}

BinaryCKKS::~BinaryCKKS() {
    delete dg_sampler;
    delete hwt_sampler;
    delete zo_sampler;
    delete ea;
    delete context;
}

void BinaryCKKS::chooseParameters(long lambda) {
    // Parameter selection based on security parameter
    security = lambda;
    p = 2;           // Binary field
    r = 1;           // Lifting parameter
    L = 16;          // Number of levels
    h = 64;          // Hamming weight of secret key
    P = lambda;      // Evaluation key parameter
    sigma = 3.2;     // Gaussian noise standard deviation
    
    // Choose M based on security requirements
    M = helib::FindM(security, L, 3, p, 0, 0, 0);
    if (M <= 0) {
        M = 32768; // Fallback value
    }
}

BinaryCKKSKeys BinaryCKKS::keyGen() {
    BinaryCKKSKeys keys;
    
    long n = M; // Polynomial degree
    
    // Sample secret key s with Hamming weight h
    vector<long> s_coeffs = hwt_sampler->sampleHWT(n, h);
    keys.s = BinaryPoly(s_coeffs, n);
    
    // Sample a uniformly from binary polynomial ring
    vector<long> a_coeffs = zo_sampler->sampleVector(n);
    keys.pk_a = BinaryPoly(a_coeffs, n);
    
    // Sample error e from discrete Gaussian
    vector<long> e_coeffs = dg_sampler->sampleVector(n);
    for (auto& coeff : e_coeffs) coeff = coeff % 2; // Reduce to binary
    BinaryPoly e(e_coeffs, n);
    
    // Compute b = -as + e
    BinaryPoly as = keys.pk_a * keys.s;
    keys.pk_b = e + as; // In Z_2, -as = as
    
    // Generate evaluation key
    vector<long> a0_coeffs = zo_sampler->sampleVector(n);
    keys.evk_a = BinaryPoly(a0_coeffs, n);
    
    vector<long> e0_coeffs = dg_sampler->sampleVector(n);
    for (auto& coeff : e0_coeffs) coeff = coeff % 2; // Reduce to binary
    BinaryPoly e0(e0_coeffs, n);
    
    // Compute b0 = -a0*s + e0 + s^2
    BinaryPoly a0s = keys.evk_a * keys.s;
    BinaryPoly s2 = keys.s * keys.s;
    keys.evk_b = e0 + a0s + s2; // In Z_2, -a0*s = a0*s
    
    return keys;
}

BinaryPoly BinaryCKKS::encode(const vector<complex<double>>& z, double Delta) {
    long n = z.size();
    CanonicalEmbedding ce(M);
    
    // Step 1: Apply inverse canonical embedding pi^{-1}(z)
    vector<complex<double>> h = ce.embedInverse(z);
    
    // Step 2: Scale by Delta
    for (auto& val : h) {
        val *= Delta;
    }
    
    // Step 3: Round to get values in sigma(R)
    vector<complex<double>> rounded_h(h.size());
    for (size_t i = 0; i < h.size(); i++) {
        rounded_h[i] = complex<double>(round(h[i].real()), round(h[i].imag()));
    }
    
    // Step 4: Apply sigma^{-1} to get polynomial in R
    ZZX m_poly = ce.coeffEmbedInverse(rounded_h);
    
    // Step 5: Convert to binary polynomial representation
    BinaryPoly result(M);
    result.fromIntPoly(m_poly, 256); // Assume 8-bit bound
    
    return result;
}

vector<complex<double>> BinaryCKKS::decode(const BinaryPoly& m, double Delta) {
    CanonicalEmbedding ce(M);
    
    // Step 1: Convert binary polynomial back to integer polynomial
    ZZX m_poly = m.toIntPoly();
    
    // Step 2: Apply coefficient embedding sigma
    vector<complex<double>> sigma_m = ce.coeffEmbed(m_poly);
    
    // Step 3: Scale by 1/Delta
    for (auto& val : sigma_m) {
        val /= Delta;
    }
    
    // Step 4: Apply canonical embedding pi
    vector<complex<double>> result = ce.embed(sigma_m);
    
    return result;
}

BinaryCKKSCiphertext BinaryCKKS::encrypt(const BinaryPoly& m, const BinaryCKKSKeys& keys) {
    long n = M;
    
    // Sample v from ZO(0.5)
    vector<long> v_coeffs = zo_sampler->sampleVector(n);
    BinaryPoly v(v_coeffs, n);
    
    // Sample e0, e1 from DG(sigma^2) and reduce to binary
    vector<long> e0_coeffs = dg_sampler->sampleVector(n);
    vector<long> e1_coeffs = dg_sampler->sampleVector(n);
    for (auto& coeff : e0_coeffs) coeff = coeff % 2;
    for (auto& coeff : e1_coeffs) coeff = coeff % 2;
    BinaryPoly e0(e0_coeffs, n);
    BinaryPoly e1(e1_coeffs, n);
    
    // Compute ciphertext: v * pk + (m + e0, e1)
    BinaryPoly vb = v * keys.pk_b;
    BinaryPoly va = v * keys.pk_a;
    
    BinaryPoly c0 = vb + m + e0;
    BinaryPoly c1 = va + e1;
    
    return BinaryCKKSCiphertext(c0, c1, sigma); // Estimate initial noise
}

BinaryPoly BinaryCKKS::decrypt(const BinaryCKKSCiphertext& c, const BinaryCKKSKeys& keys) {
    // Decrypt: c0 + c1 * s
    BinaryPoly cs = c.c1 * keys.s;
    return c.c0 + cs;
}

BinaryCKKSCiphertext BinaryCKKS::add(const BinaryCKKSCiphertext& c1, const BinaryCKKSCiphertext& c2) {
    BinaryPoly c0_sum = c1.c0 + c2.c0;
    BinaryPoly c1_sum = c1.c1 + c2.c1;
    double noise_sum = c1.noise_estimate + c2.noise_estimate; // Linear noise growth
    
    return BinaryCKKSCiphertext(c0_sum, c1_sum, noise_sum);
}

BinaryCKKSCiphertext BinaryCKKS::multiply(const BinaryCKKSCiphertext& c1, const BinaryCKKSCiphertext& c2, 
                                         const BinaryCKKSKeys& keys) {
    // Multiplication in binary polynomial ring
    BinaryPoly d0 = c1.c0 * c2.c0;
    BinaryPoly d1 = c1.c0 * c2.c1 + c1.c1 * c2.c0;
    BinaryPoly d2 = c1.c1 * c2.c1;
    
    // Key switching using evaluation key
    BinaryPoly d2_evk_a = d2 * keys.evk_a;
    BinaryPoly d2_evk_b = d2 * keys.evk_b;
    
    BinaryPoly c0_mult = d0 + d2_evk_b;
    BinaryPoly c1_mult = d1 + d2_evk_a;
    
    double noise_mult = c1.noise_estimate * c2.noise_estimate + sigma; // Multiplicative noise growth
    
    return BinaryCKKSCiphertext(c0_mult, c1_mult, noise_mult);
}

bool BinaryCKKS::threshold(double B_max, double B_0) {
    return B_0 > B_max;
}

BinaryCKKSCiphertext BinaryCKKS::refresh(const BinaryCKKSCiphertext& c, 
                                        const BinaryCKKSKeys& old_keys,
                                        const BinaryCKKSKeys& new_keys,
                                        double Delta) {
    // Step 1: Decrypt under old key
    BinaryPoly m = decrypt(c, old_keys);
    
    // Step 2: Re-encode (this should preserve the semantic value)
    // For now, we'll assume the message is already properly encoded
    
    // Step 3: Encrypt under new key
    BinaryCKKSCiphertext c_new = encrypt(m, new_keys);
    
    return c_new;
}

long BinaryCKKS::getSlots() const {
    return ea ? ea->size() : M / 2;
}

void BinaryCKKS::printParameters() const {
    cout << "=== Binary CKKS Parameters ===" << endl;
    cout << "Security parameter (lambda): " << security << endl;
    cout << "Cyclotomic parameter (M): " << M << endl;
    cout << "Prime (p): " << p << endl;
    cout << "Lifting parameter (r): " << r << endl;
    cout << "Levels (L): " << L << endl;
    cout << "Hamming weight (h): " << h << endl;
    cout << "Evaluation key parameter (P): " << P << endl;
    cout << "Gaussian sigma: " << sigma << endl;
    cout << "Number of slots: " << getSlots() << endl;
    cout << "=========================" << endl;
}