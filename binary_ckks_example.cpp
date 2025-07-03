#include "binary_ckks.h"
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

// Helper function to print complex vectors
void printComplexVector(const vector<complex<double>>& vec, const string& name) {
    cout << name << ": [";
    for (size_t i = 0; i < min(vec.size(), (size_t)8); i++) {
        cout << "(" << setprecision(3) << vec[i].real() << "," << vec[i].imag() << ")";
        if (i < min(vec.size(), (size_t)8) - 1) cout << ", ";
    }
    if (vec.size() > 8) cout << ", ...";
    cout << "]" << endl;
}

// Helper function to generate test data
vector<complex<double>> generateTestData(size_t size) {
    vector<complex<double>> data(size);
    for (size_t i = 0; i < size; i++) {
        data[i] = complex<double>(i + 1, 0); // Simple real numbers
    }
    return data;
}

// Helper function to measure execution time
template<typename Func>
double measureTime(Func&& func) {
    auto start = high_resolution_clock::now();
    func();
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    return duration.count() / 1000.0; // Convert to milliseconds
}

int main() {
    cout << "=====================================\n";
    cout << "Binary CKKS Homomorphic Encryption Demo\n";
    cout << "=====================================\n\n";
    
    try {
        // Initialize Binary CKKS scheme
        cout << "1. Initializing Binary CKKS scheme...\n";
        BinaryCKKS scheme(128); // 128-bit security
        scheme.printParameters();
        cout << "\n";
        
        // Key Generation
        cout << "2. Generating keys...\n";
        BinaryCKKSKeys keys;
        double keygen_time = measureTime([&]() {
            keys = scheme.keyGen();
        });
        cout << "Key generation completed in " << keygen_time << " ms\n";
        cout << "\n";
        
        // Prepare test data
        cout << "3. Preparing test data...\n";
        size_t num_slots = min((long)8, scheme.getSlots()); // Use fewer slots for demo
        vector<complex<double>> data1 = generateTestData(num_slots);
        vector<complex<double>> data2 = generateTestData(num_slots);
        
        // Modify data2 for variety
        for (size_t i = 0; i < data2.size(); i++) {
            data2[i] = complex<double>(i + 10, 0);
        }
        
        printComplexVector(data1, "Data 1");
        printComplexVector(data2, "Data 2");
        cout << "\n";
        
        // Encoding
        cout << "4. Encoding data...\n";
        double Delta = 64.0; // Scaling factor
        BinaryPoly encoded1, encoded2;
        double encode_time = measureTime([&]() {
            encoded1 = scheme.encode(data1, Delta);
            encoded2 = scheme.encode(data2, Delta);
        });
        cout << "Encoding completed in " << encode_time << " ms\n";
        cout << "Encoded polynomial 1: ";
        encoded1.print();
        cout << "Encoded polynomial 2: ";
        encoded2.print();
        cout << "\n";
        
        // Encryption
        cout << "5. Encrypting data...\n";
        BinaryCKKSCiphertext ct1, ct2;
        double encrypt_time = measureTime([&]() {
            ct1 = scheme.encrypt(encoded1, keys);
            ct2 = scheme.encrypt(encoded2, keys);
        });
        cout << "Encryption completed in " << encrypt_time << " ms\n";
        cout << "Ciphertext 1 noise estimate: " << ct1.noise_estimate << "\n";
        cout << "Ciphertext 2 noise estimate: " << ct2.noise_estimate << "\n";
        cout << "\n";
        
        // Homomorphic Addition
        cout << "6. Performing homomorphic addition...\n";
        BinaryCKKSCiphertext ct_add;
        double add_time = measureTime([&]() {
            ct_add = scheme.add(ct1, ct2);
        });
        cout << "Homomorphic addition completed in " << add_time << " ms\n";
        cout << "Addition result noise estimate: " << ct_add.noise_estimate << "\n";
        cout << "\n";
        
        // Homomorphic Multiplication
        cout << "7. Performing homomorphic multiplication...\n";
        BinaryCKKSCiphertext ct_mult;
        double mult_time = measureTime([&]() {
            ct_mult = scheme.multiply(ct1, ct2, keys);
        });
        cout << "Homomorphic multiplication completed in " << mult_time << " ms\n";
        cout << "Multiplication result noise estimate: " << ct_mult.noise_estimate << "\n";
        cout << "\n";
        
        // Decryption and Decoding
        cout << "8. Decrypting and decoding results...\n";
        
        // Decrypt addition result
        BinaryPoly decrypted_add;
        vector<complex<double>> result_add;
        double decrypt_time = measureTime([&]() {
            decrypted_add = scheme.decrypt(ct_add, keys);
            result_add = scheme.decode(decrypted_add, Delta);
        });
        
        // Decrypt multiplication result
        BinaryPoly decrypted_mult = scheme.decrypt(ct_mult, keys);
        vector<complex<double>> result_mult = scheme.decode(decrypted_mult, Delta);
        
        cout << "Decryption and decoding completed in " << decrypt_time << " ms\n";
        cout << "\n";
        
        // Display results
        cout << "9. Results comparison...\n";
        printComplexVector(data1, "Original Data 1");
        printComplexVector(data2, "Original Data 2");
        cout << "\n";
        
        // Expected results (computed in plaintext)
        vector<complex<double>> expected_add(num_slots), expected_mult(num_slots);
        for (size_t i = 0; i < num_slots; i++) {
            expected_add[i] = data1[i] + data2[i];
            expected_mult[i] = data1[i] * data2[i];
        }
        
        printComplexVector(expected_add, "Expected Addition");
        printComplexVector(result_add, "HE Addition Result");
        cout << "\n";
        
        printComplexVector(expected_mult, "Expected Multiplication");
        printComplexVector(result_mult, "HE Multiplication Result");
        cout << "\n";
        
        // Calculate and display errors
        cout << "10. Error analysis...\n";
        double add_error = 0.0, mult_error = 0.0;
        for (size_t i = 0; i < min(num_slots, result_add.size()); i++) {
            add_error += abs(expected_add[i] - result_add[i]);
        }
        for (size_t i = 0; i < min(num_slots, result_mult.size()); i++) {
            mult_error += abs(expected_mult[i] - result_mult[i]);
        }
        
        cout << "Average addition error: " << add_error / num_slots << "\n";
        cout << "Average multiplication error: " << mult_error / num_slots << "\n";
        cout << "\n";
        
        // Test threshold function
        cout << "11. Testing noise threshold...\n";
        double B_max = 100.0;
        bool add_needs_refresh = scheme.threshold(B_max, ct_add.noise_estimate);
        bool mult_needs_refresh = scheme.threshold(B_max, ct_mult.noise_estimate);
        
        cout << "Addition result needs refresh: " << (add_needs_refresh ? "Yes" : "No") << "\n";
        cout << "Multiplication result needs refresh: " << (mult_needs_refresh ? "Yes" : "No") << "\n";
        cout << "\n";
        
        // Demonstrate refresh operation
        if (mult_needs_refresh) {
            cout << "12. Demonstrating refresh operation...\n";
            
            // Generate new keys for refresh
            BinaryCKKSKeys new_keys;
            double new_keygen_time = measureTime([&]() {
                new_keys = scheme.keyGen();
            });
            
            // Refresh the multiplication ciphertext
            BinaryCKKSCiphertext ct_refreshed;
            double refresh_time = measureTime([&]() {
                ct_refreshed = scheme.refresh(ct_mult, keys, new_keys, Delta);
            });
            
            cout << "New key generation time: " << new_keygen_time << " ms\n";
            cout << "Refresh operation time: " << refresh_time << " ms\n";
            cout << "Original noise estimate: " << ct_mult.noise_estimate << "\n";
            cout << "Refreshed noise estimate: " << ct_refreshed.noise_estimate << "\n";
            
            // Verify refresh correctness
            BinaryPoly decrypted_refreshed = scheme.decrypt(ct_refreshed, new_keys);
            vector<complex<double>> result_refreshed = scheme.decode(decrypted_refreshed, Delta);
            
            double refresh_error = 0.0;
            for (size_t i = 0; i < min(num_slots, result_refreshed.size()); i++) {
                refresh_error += abs(expected_mult[i] - result_refreshed[i]);
            }
            cout << "Refresh operation error: " << refresh_error / num_slots << "\n";
            cout << "\n";
        }
        
        // Performance summary
        cout << "13. Performance Summary...\n";
        cout << "========================================\n";
        cout << "Key Generation:    " << setw(10) << keygen_time << " ms\n";
        cout << "Encoding:          " << setw(10) << encode_time << " ms\n";
        cout << "Encryption:        " << setw(10) << encrypt_time << " ms\n";
        cout << "HE Addition:       " << setw(10) << add_time << " ms\n";
        cout << "HE Multiplication: " << setw(10) << mult_time << " ms\n";
        cout << "Decryption:        " << setw(10) << decrypt_time << " ms\n";
        cout << "========================================\n";
        cout << "\n";
        
        // Additional tests
        cout << "14. Additional functionality tests...\n";
        
        // Test multiple additions
        BinaryCKKSCiphertext ct_chain = ct1;
        for (int i = 0; i < 3; i++) {
            ct_chain = scheme.add(ct_chain, ct2);
        }
        cout << "Chain of 3 additions noise estimate: " << ct_chain.noise_estimate << "\n";
        
        // Test multiple multiplications (if noise permits)
        if (ct1.noise_estimate * ct2.noise_estimate < 50.0) {
            BinaryCKKSCiphertext ct_mult_chain = scheme.multiply(ct1, ct2, keys);
            ct_mult_chain = scheme.multiply(ct_mult_chain, ct1, keys);
            cout << "Chain of 2 multiplications noise estimate: " << ct_mult_chain.noise_estimate << "\n";
        } else {
            cout << "Skipping multiplication chain due to high noise\n";
        }
        
        cout << "\n=== Binary CKKS Demo Complete ===\n";
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}

// Additional utility functions for advanced testing
void demonstrateAdvancedOperations() {
    cout << "\n=== Advanced Operations Demo ===\n";
    
    BinaryCKKS scheme(128);
    BinaryCKKSKeys keys = scheme.keyGen();
    
    // Test with different data types
    vector<complex<double>> gaussian_integers;
    for (int i = 0; i < 8; i++) {
        gaussian_integers.push_back(complex<double>(i, i));
    }
    
    cout << "Testing with Gaussian integers:\n";
    printComplexVector(gaussian_integers, "Gaussian Integers");
    
    double Delta = 128.0;
    BinaryPoly encoded = scheme.encode(gaussian_integers, Delta);
    BinaryCKKSCiphertext ct = scheme.encrypt(encoded, keys);
    
    BinaryPoly decrypted = scheme.decrypt(ct, keys);
    vector<complex<double>> decoded = scheme.decode(decrypted, Delta);
    
    printComplexVector(decoded, "Recovered Gaussian Integers");
}

void benchmarkOperations() {
    cout << "\n=== Performance Benchmarking ===\n";
    
    BinaryCKKS scheme(128);
    BinaryCKKSKeys keys = scheme.keyGen();
    
    // Generate larger test data
    size_t num_tests = 100;
    vector<double> add_times, mult_times;
    
    for (size_t test = 0; test < num_tests; test++) {
        vector<complex<double>> data1 = generateTestData(4);
        vector<complex<double>> data2 = generateTestData(4);
        
        BinaryPoly enc1 = scheme.encode(data1, 64.0);
        BinaryPoly enc2 = scheme.encode(data2, 64.0);
        
        BinaryCKKSCiphertext ct1 = scheme.encrypt(enc1, keys);
        BinaryCKKSCiphertext ct2 = scheme.encrypt(enc2, keys);
        
        // Benchmark addition
        auto start = high_resolution_clock::now();
        BinaryCKKSCiphertext ct_add = scheme.add(ct1, ct2);
        auto end = high_resolution_clock::now();
        add_times.push_back(duration_cast<microseconds>(end - start).count() / 1000.0);
        
        // Benchmark multiplication
        start = high_resolution_clock::now();
        BinaryCKKSCiphertext ct_mult = scheme.multiply(ct1, ct2, keys);
        end = high_resolution_clock::now();
        mult_times.push_back(duration_cast<microseconds>(end - start).count() / 1000.0);
    }
    
    // Calculate statistics
    double avg_add = 0, avg_mult = 0;
    for (double t : add_times) avg_add += t;
    for (double t : mult_times) avg_mult += t;
    avg_add /= num_tests;
    avg_mult /= num_tests;
    
    cout << "Average addition time over " << num_tests << " tests: " << avg_add << " ms\n";
    cout << "Average multiplication time over " << num_tests << " tests: " << avg_mult << " ms\n";
}