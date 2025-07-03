#include <iostream>
#include <chrono>
#include <vector>
#include <iomanip>
#include <fstream>
#include <map>
#include <memory>

// Standard CKKS includes
#include <helib/helib.h>

// Binary CKKS includes
#include "simple_binary_ckks.h"

using namespace std;
using namespace std::chrono;

class BenchmarkResults {
public:
    struct OperationTiming {
        double mean_microseconds;
        double stddev_microseconds;
        size_t iterations;
        
        OperationTiming() : mean_microseconds(0), stddev_microseconds(0), iterations(0) {}
        OperationTiming(double mean, double stddev, size_t iter) 
            : mean_microseconds(mean), stddev_microseconds(stddev), iterations(iter) {}
    };
    
    map<string, OperationTiming> standard_ckks;
    map<string, OperationTiming> binary_ckks;
    
    void printResults() const;
    void saveToCSV(const string& filename) const;
};

class StandardCKKSBenchmark {
private:
    unique_ptr<helib::Context> context;
    unique_ptr<helib::SecKey> secretKey;
    unique_ptr<helib::PubKey> publicKey;
    unique_ptr<helib::EncryptedArray> ea;
    
public:
    StandardCKKSBenchmark(long m, long precision, long bits);
    BenchmarkResults::OperationTiming benchmarkKeyGeneration(size_t iterations = 10);
    BenchmarkResults::OperationTiming benchmarkEncryption(size_t iterations = 100);
    BenchmarkResults::OperationTiming benchmarkDecryption(size_t iterations = 100);
    BenchmarkResults::OperationTiming benchmarkAddition(size_t iterations = 1000);
    BenchmarkResults::OperationTiming benchmarkMultiplication(size_t iterations = 100);
    BenchmarkResults::OperationTiming benchmarkRotation(size_t iterations = 100);
    
private:
    BenchmarkResults::OperationTiming measureOperation(function<void()> operation, size_t iterations);
};

class BinaryCKKSBenchmark {
private:
    unique_ptr<SimpleBinaryCKKS> scheme;
    SimpleBinaryCKKSKeys keys;
    bool keys_generated;
    
public:
    BinaryCKKSBenchmark(long security, long ring_dim);
    BenchmarkResults::OperationTiming benchmarkKeyGeneration(size_t iterations = 10);
    BenchmarkResults::OperationTiming benchmarkEncryption(size_t iterations = 100);
    BenchmarkResults::OperationTiming benchmarkDecryption(size_t iterations = 100);
    BenchmarkResults::OperationTiming benchmarkAddition(size_t iterations = 1000);
    BenchmarkResults::OperationTiming benchmarkMultiplication(size_t iterations = 100);
    
private:
    BenchmarkResults::OperationTiming measureOperation(function<void()> operation, size_t iterations);
    vector<long> generateTestData(size_t size);
};

// ===================== Implementation =====================

StandardCKKSBenchmark::StandardCKKSBenchmark(long m, long precision, long bits) {
    cout << "Initializing Standard CKKS with m=" << m << ", precision=" << precision << ", bits=" << bits << endl;
    
    context = make_unique<helib::Context>(
        helib::ContextBuilder<helib::CKKS>()
            .m(m)
            .precision(precision)
            .bits(bits)
            .scale(10)
            .build()
    );
    
    secretKey = make_unique<helib::SecKey>(*context);
    secretKey->GenSecKey();
    helib::addSome1DMatrices(*secretKey);
    
    publicKey = make_unique<helib::PubKey>(*secretKey);
    ea = make_unique<helib::EncryptedArray>(context->getEA());
    
    cout << "Standard CKKS initialization complete" << endl;
}

BenchmarkResults::OperationTiming StandardCKKSBenchmark::measureOperation(
    function<void()> operation, size_t iterations) {
    
    vector<double> times;
    times.reserve(iterations);
    
    // Warmup
    for (size_t i = 0; i < min(iterations/10, 5UL); i++) {
        operation();
    }
    
    // Actual measurement
    for (size_t i = 0; i < iterations; i++) {
        auto start = high_resolution_clock::now();
        operation();
        auto end = high_resolution_clock::now();
        
        auto duration = duration_cast<microseconds>(end - start);
        times.push_back(duration.count());
    }
    
    // Calculate statistics
    double mean = 0;
    for (double time : times) {
        mean += time;
    }
    mean /= times.size();
    
    double variance = 0;
    for (double time : times) {
        variance += (time - mean) * (time - mean);
    }
    variance /= times.size();
    double stddev = sqrt(variance);
    
    return BenchmarkResults::OperationTiming(mean, stddev, iterations);
}

BenchmarkResults::OperationTiming StandardCKKSBenchmark::benchmarkKeyGeneration(size_t iterations) {
    cout << "Benchmarking Standard CKKS Key Generation..." << endl;
    
    return measureOperation([this]() {
        helib::SecKey sk(*context);
        sk.GenSecKey();
        helib::addSome1DMatrices(sk);
    }, iterations);
}

BenchmarkResults::OperationTiming StandardCKKSBenchmark::benchmarkEncryption(size_t iterations) {
    cout << "Benchmarking Standard CKKS Encryption..." << endl;
    
    helib::Ptxt<helib::CKKS> ptxt(*context);
    ptxt.random();
    
    return measureOperation([this, &ptxt]() {
        helib::Ctxt ctxt(*publicKey);
        publicKey->Encrypt(ctxt, ptxt);
    }, iterations);
}

BenchmarkResults::OperationTiming StandardCKKSBenchmark::benchmarkDecryption(size_t iterations) {
    cout << "Benchmarking Standard CKKS Decryption..." << endl;
    
    helib::Ptxt<helib::CKKS> ptxt(*context);
    ptxt.random();
    helib::Ctxt ctxt(*publicKey);
    publicKey->Encrypt(ctxt, ptxt);
    
    return measureOperation([this, &ctxt]() {
        helib::Ptxt<helib::CKKS> result(*context);
        secretKey->Decrypt(result, ctxt);
    }, iterations);
}

BenchmarkResults::OperationTiming StandardCKKSBenchmark::benchmarkAddition(size_t iterations) {
    cout << "Benchmarking Standard CKKS Addition..." << endl;
    
    helib::Ptxt<helib::CKKS> ptxt1(*context), ptxt2(*context);
    ptxt1.random();
    ptxt2.random();
    
    helib::Ctxt ctxt1(*publicKey), ctxt2(*publicKey);
    publicKey->Encrypt(ctxt1, ptxt1);
    publicKey->Encrypt(ctxt2, ptxt2);
    
    return measureOperation([&ctxt1, &ctxt2]() {
        auto copy = ctxt1;
        copy += ctxt2;
    }, iterations);
}

BenchmarkResults::OperationTiming StandardCKKSBenchmark::benchmarkMultiplication(size_t iterations) {
    cout << "Benchmarking Standard CKKS Multiplication..." << endl;
    
    helib::Ptxt<helib::CKKS> ptxt1(*context), ptxt2(*context);
    ptxt1.random();
    ptxt2.random();
    
    helib::Ctxt ctxt1(*publicKey), ctxt2(*publicKey);
    publicKey->Encrypt(ctxt1, ptxt1);
    publicKey->Encrypt(ctxt2, ptxt2);
    
    return measureOperation([&ctxt1, &ctxt2]() {
        auto copy = ctxt1;
        copy.multiplyBy(ctxt2);
    }, iterations);
}

BenchmarkResults::OperationTiming StandardCKKSBenchmark::benchmarkRotation(size_t iterations) {
    cout << "Benchmarking Standard CKKS Rotation..." << endl;
    
    helib::Ptxt<helib::CKKS> ptxt(*context);
    ptxt.random();
    helib::Ctxt ctxt(*publicKey);
    publicKey->Encrypt(ctxt, ptxt);
    
    return measureOperation([this, &ctxt]() {
        auto copy = ctxt;
        ea->rotate(copy, 1);
    }, iterations);
}

// ===================== Binary CKKS Implementation =====================

BinaryCKKSBenchmark::BinaryCKKSBenchmark(long security, long ring_dim) : keys_generated(false) {
    cout << "Initializing Binary CKKS with security=" << security << ", ring_dim=" << ring_dim << endl;
    scheme = make_unique<SimpleBinaryCKKS>(security, ring_dim);
    cout << "Binary CKKS initialization complete" << endl;
}

BenchmarkResults::OperationTiming BinaryCKKSBenchmark::measureOperation(
    function<void()> operation, size_t iterations) {
    
    vector<double> times;
    times.reserve(iterations);
    
    // Warmup
    for (size_t i = 0; i < min(iterations/10, 5UL); i++) {
        operation();
    }
    
    // Actual measurement
    for (size_t i = 0; i < iterations; i++) {
        auto start = high_resolution_clock::now();
        operation();
        auto end = high_resolution_clock::now();
        
        auto duration = duration_cast<microseconds>(end - start);
        times.push_back(duration.count());
    }
    
    // Calculate statistics
    double mean = 0;
    for (double time : times) {
        mean += time;
    }
    mean /= times.size();
    
    double variance = 0;
    for (double time : times) {
        variance += (time - mean) * (time - mean);
    }
    variance /= times.size();
    double stddev = sqrt(variance);
    
    return BenchmarkResults::OperationTiming(mean, stddev, iterations);
}

vector<long> BinaryCKKSBenchmark::generateTestData(size_t size) {
    vector<long> data(size);
    for (size_t i = 0; i < size; i++) {
        data[i] = rand() % 2;
    }
    return data;
}

BenchmarkResults::OperationTiming BinaryCKKSBenchmark::benchmarkKeyGeneration(size_t iterations) {
    cout << "Benchmarking Binary CKKS Key Generation..." << endl;
    
    return measureOperation([this]() {
        SimpleBinaryCKKSKeys temp_keys = scheme->keyGen();
    }, iterations);
}

BenchmarkResults::OperationTiming BinaryCKKSBenchmark::benchmarkEncryption(size_t iterations) {
    cout << "Benchmarking Binary CKKS Encryption..." << endl;
    
    if (!keys_generated) {
        keys = scheme->keyGen();
        keys_generated = true;
    }
    
    vector<long> data = generateTestData(16);
    SimpleBinaryPoly encoded = scheme->encode(data);
    
    return measureOperation([this, &encoded]() {
        SimpleBinaryCKKSCiphertext ctxt = scheme->encrypt(encoded, keys);
    }, iterations);
}

BenchmarkResults::OperationTiming BinaryCKKSBenchmark::benchmarkDecryption(size_t iterations) {
    cout << "Benchmarking Binary CKKS Decryption..." << endl;
    
    if (!keys_generated) {
        keys = scheme->keyGen();
        keys_generated = true;
    }
    
    vector<long> data = generateTestData(16);
    SimpleBinaryPoly encoded = scheme->encode(data);
    SimpleBinaryCKKSCiphertext ctxt = scheme->encrypt(encoded, keys);
    
    return measureOperation([this, &ctxt]() {
        SimpleBinaryPoly result = scheme->decrypt(ctxt, keys);
    }, iterations);
}

BenchmarkResults::OperationTiming BinaryCKKSBenchmark::benchmarkAddition(size_t iterations) {
    cout << "Benchmarking Binary CKKS Addition..." << endl;
    
    if (!keys_generated) {
        keys = scheme->keyGen();
        keys_generated = true;
    }
    
    vector<long> data1 = generateTestData(16);
    vector<long> data2 = generateTestData(16);
    
    SimpleBinaryPoly encoded1 = scheme->encode(data1);
    SimpleBinaryPoly encoded2 = scheme->encode(data2);
    
    SimpleBinaryCKKSCiphertext ctxt1 = scheme->encrypt(encoded1, keys);
    SimpleBinaryCKKSCiphertext ctxt2 = scheme->encrypt(encoded2, keys);
    
    return measureOperation([this, &ctxt1, &ctxt2]() {
        SimpleBinaryCKKSCiphertext result = scheme->add(ctxt1, ctxt2);
    }, iterations);
}

BenchmarkResults::OperationTiming BinaryCKKSBenchmark::benchmarkMultiplication(size_t iterations) {
    cout << "Benchmarking Binary CKKS Multiplication..." << endl;
    
    if (!keys_generated) {
        keys = scheme->keyGen();
        keys_generated = true;
    }
    
    vector<long> data1 = generateTestData(16);
    vector<long> data2 = generateTestData(16);
    
    SimpleBinaryPoly encoded1 = scheme->encode(data1);
    SimpleBinaryPoly encoded2 = scheme->encode(data2);
    
    SimpleBinaryCKKSCiphertext ctxt1 = scheme->encrypt(encoded1, keys);
    SimpleBinaryCKKSCiphertext ctxt2 = scheme->encrypt(encoded2, keys);
    
    return measureOperation([this, &ctxt1, &ctxt2]() {
        SimpleBinaryCKKSCiphertext result = scheme->multiply(ctxt1, ctxt2, keys);
    }, iterations);
}

// ===================== Results Implementation =====================

void BenchmarkResults::printResults() const {
    cout << "\n=== CKKS Performance Comparison Results ===" << endl;
    cout << setprecision(2) << fixed;
    
    cout << "\n" << setw(20) << "Operation" 
         << setw(18) << "Standard CKKS" 
         << setw(18) << "Binary CKKS" 
         << setw(15) << "Speedup"
         << setw(15) << "Efficiency" << endl;
    cout << string(86, '-') << endl;
    
    vector<string> operations = {"KeyGeneration", "Encryption", "Decryption", "Addition", "Multiplication"};
    
    for (const string& op : operations) {
        auto std_it = standard_ckks.find(op);
        auto bin_it = binary_ckks.find(op);
        
        if (std_it != standard_ckks.end() && bin_it != binary_ckks.end()) {
            double std_time = std_it->second.mean_microseconds;
            double bin_time = bin_it->second.mean_microseconds;
            double speedup = std_time / bin_time;
            double efficiency = (speedup > 1.0) ? ((speedup - 1.0) / speedup) * 100 : 0.0;
            
            cout << setw(20) << op
                 << setw(12) << std_time << " μs"
                 << setw(12) << bin_time << " μs"
                 << setw(12) << speedup << "x"
                 << setw(12) << efficiency << "%" << endl;
        }
    }
    
    cout << "\n=== Detailed Statistics ===" << endl;
    cout << "\nStandard CKKS:" << endl;
    for (const auto& [op, timing] : standard_ckks) {
        cout << "  " << op << ": " << timing.mean_microseconds 
             << " ± " << timing.stddev_microseconds << " μs (" << timing.iterations << " iterations)" << endl;
    }
    
    cout << "\nBinary CKKS:" << endl;
    for (const auto& [op, timing] : binary_ckks) {
        cout << "  " << op << ": " << timing.mean_microseconds 
             << " ± " << timing.stddev_microseconds << " μs (" << timing.iterations << " iterations)" << endl;
    }
}

void BenchmarkResults::saveToCSV(const string& filename) const {
    ofstream file(filename);
    file << "Operation,Standard_CKKS_Mean,Standard_CKKS_Stddev,Standard_CKKS_Iterations,";
    file << "Binary_CKKS_Mean,Binary_CKKS_Stddev,Binary_CKKS_Iterations,Speedup\n";
    
    vector<string> operations = {"KeyGeneration", "Encryption", "Decryption", "Addition", "Multiplication"};
    
    for (const string& op : operations) {
        auto std_it = standard_ckks.find(op);
        auto bin_it = binary_ckks.find(op);
        
        if (std_it != standard_ckks.end() && bin_it != binary_ckks.end()) {
            double speedup = std_it->second.mean_microseconds / bin_it->second.mean_microseconds;
            
            file << op << ","
                 << std_it->second.mean_microseconds << ","
                 << std_it->second.stddev_microseconds << ","
                 << std_it->second.iterations << ","
                 << bin_it->second.mean_microseconds << ","
                 << bin_it->second.stddev_microseconds << ","
                 << bin_it->second.iterations << ","
                 << speedup << "\n";
        }
    }
    
    file.close();
    cout << "Results saved to " << filename << endl;
}

// ===================== Main Benchmark Function =====================

void runComprehensiveBenchmark(const vector<pair<long, long>>& parameter_sets) {
    cout << "=== Comprehensive CKKS Performance Comparison ===" << endl;
    cout << "Comparing Standard CKKS vs Binary CKKS variants" << endl;
    cout << "Parameter sets: " << parameter_sets.size() << endl << endl;
    
    for (size_t i = 0; i < parameter_sets.size(); i++) {
        long ring_dim = parameter_sets[i].first;
        long security = parameter_sets[i].second;
        
        cout << "\n=== Parameter Set " << (i+1) << "/" << parameter_sets.size() 
             << " (Ring Dim: " << ring_dim << ", Security: " << security << ") ===" << endl;
        
        BenchmarkResults results;
        
        try {
            // Standard CKKS Benchmarks
            cout << "\n--- Standard CKKS Benchmarks ---" << endl;
            StandardCKKSBenchmark std_benchmark(ring_dim, 1, 360);
            
            results.standard_ckks["KeyGeneration"] = std_benchmark.benchmarkKeyGeneration(5);
            results.standard_ckks["Encryption"] = std_benchmark.benchmarkEncryption(50);
            results.standard_ckks["Decryption"] = std_benchmark.benchmarkDecryption(50);
            results.standard_ckks["Addition"] = std_benchmark.benchmarkAddition(200);
            results.standard_ckks["Multiplication"] = std_benchmark.benchmarkMultiplication(20);
            
            // Binary CKKS Benchmarks
            cout << "\n--- Binary CKKS Benchmarks ---" << endl;
            BinaryCKKSBenchmark bin_benchmark(security, ring_dim);
            
            results.binary_ckks["KeyGeneration"] = bin_benchmark.benchmarkKeyGeneration(10);
            results.binary_ckks["Encryption"] = bin_benchmark.benchmarkEncryption(100);
            results.binary_ckks["Decryption"] = bin_benchmark.benchmarkDecryption(100);
            results.binary_ckks["Addition"] = bin_benchmark.benchmarkAddition(500);
            results.binary_ckks["Multiplication"] = bin_benchmark.benchmarkMultiplication(50);
            
            // Print and save results
            results.printResults();
            
            string filename = "ckks_comparison_" + to_string(ring_dim) + "_" + to_string(security) + ".csv";
            results.saveToCSV(filename);
            
        } catch (const exception& e) {
            cerr << "Error in parameter set " << (i+1) << ": " << e.what() << endl;
        }
    }
}

int main() {
    cout << "CKKS Standard vs Binary Variant Performance Comparison" << endl;
    cout << "=======================================================" << endl;
    
    // Define parameter sets for comprehensive testing
    vector<pair<long, long>> parameter_sets = {
        {1024, 128},   // Small parameters
        {2048, 128},   // Medium parameters
        {4096, 128},   // Large parameters (if memory permits)
    };
    
    runComprehensiveBenchmark(parameter_sets);
    
    cout << "\n=== Benchmark Complete ===" << endl;
    cout << "Results saved to CSV files for analysis" << endl;
    cout << "Use the CSV data to generate plots for your conference paper" << endl;
    
    return 0;
}