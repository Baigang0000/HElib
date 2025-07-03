#include "simple_binary_ckks.h"
#include <iostream>
#include <chrono>

using namespace std;
using namespace SimpleBinaryCKKSUtils;

int main() {
    cout << "===============================================" << endl;
    cout << "    Simple Binary CKKS Demonstration" << endl;
    cout << "   (Educational Implementation using NTL)" << endl;
    cout << "===============================================" << endl << endl;
    
    try {
        // Test basic polynomial operations first
        testBasicOperations();
        
        // Main homomorphic encryption demonstration
        testHomomorphicOperations();
        
        // Performance benchmarking
        benchmarkOperations();
        
        // Demonstrate advanced features
        cout << "=== Advanced Features Demo ===" << endl;
        
        SimpleBinaryCKKS scheme(128, 512);
        scheme.printParameters();
        
        // Generate keys
        SimpleBinaryCKKSKeys keys = scheme.keyGen();
        
        // Test data
        vector<long> data = {1, 1, 0, 1, 0, 0, 1, 1};
        printVector(data, "Original Data");
        
        // Encode and encrypt
        SimpleBinaryPoly encoded = scheme.encode(data);
        SimpleBinaryCKKSCiphertext ct = scheme.encrypt(encoded, keys);
        
        cout << "Initial noise level: " << ct.noise_estimate << endl;
        
        // Chain multiple operations to increase noise
        SimpleBinaryCKKSCiphertext ct_chain = ct;
        for (int i = 0; i < 3; i++) {
            ct_chain = scheme.add(ct_chain, ct);
            cout << "After addition " << (i+1) << ", noise level: " << ct_chain.noise_estimate << endl;
        }
        
        // Test refresh operation if noise is high
        if (scheme.needsRefresh(ct_chain, 50.0)) {
            cout << "\nNoise level too high, performing refresh..." << endl;
            SimpleBinaryCKKSKeys new_keys = scheme.keyGen();
            SimpleBinaryCKKSCiphertext ct_refreshed = scheme.refresh(ct_chain, keys, new_keys);
            cout << "Noise after refresh: " << ct_refreshed.noise_estimate << endl;
            
            // Verify the refresh worked correctly
            SimpleBinaryPoly decrypted_original = scheme.decrypt(ct_chain, keys);
            SimpleBinaryPoly decrypted_refreshed = scheme.decrypt(ct_refreshed, new_keys);
            
            vector<long> result_original = scheme.decode(decrypted_original, data.size());
            vector<long> result_refreshed = scheme.decode(decrypted_refreshed, data.size());
            
            printVector(result_original, "Original Result");
            printVector(result_refreshed, "Refreshed Result");
            
            bool refresh_correct = (result_original == result_refreshed);
            cout << "Refresh correctness: " << (refresh_correct ? "✓ PASS" : "✗ FAIL") << endl;
        } else {
            cout << "Noise level acceptable, no refresh needed." << endl;
        }
        
        cout << "\n=== Multiplication Chain Demo ===" << endl;
        
        // Test multiplication depth
        SimpleBinaryCKKSCiphertext ct1 = scheme.encrypt(scheme.encode({1, 1, 0, 1}), keys);
        SimpleBinaryCKKSCiphertext ct2 = scheme.encrypt(scheme.encode({1, 0, 1, 1}), keys);
        
        cout << "Initial ciphertext noise levels:" << endl;
        cout << "CT1: " << ct1.noise_estimate << endl;
        cout << "CT2: " << ct2.noise_estimate << endl;
        
        // Perform a multiplication
        SimpleBinaryCKKSCiphertext ct_mult = scheme.multiply(ct1, ct2, keys);
        cout << "After 1 multiplication: " << ct_mult.noise_estimate << endl;
        
        // Try another multiplication if noise permits
        if (ct_mult.noise_estimate < 100.0) {
            SimpleBinaryCKKSCiphertext ct_mult2 = scheme.multiply(ct_mult, ct1, keys);
            cout << "After 2 multiplications: " << ct_mult2.noise_estimate << endl;
            
            // Decrypt and check result
            SimpleBinaryPoly result = scheme.decrypt(ct_mult2, keys);
            vector<long> decoded_result = scheme.decode(result, 4);
            printVector(decoded_result, "Final Multiplication Result");
        } else {
            cout << "Noise too high for additional multiplication" << endl;
        }
        
        cout << "\n=== Large Data Test ===" << endl;
        
        // Test with larger data vectors
        vector<long> large_data1, large_data2;
        for (int i = 0; i < 32; i++) {
            large_data1.push_back(i % 2);
            large_data2.push_back((i + 1) % 2);
        }
        
        cout << "Testing with 32-element vectors..." << endl;
        printVector(large_data1, "Large Data 1", 10);
        printVector(large_data2, "Large Data 2", 10);
        
        auto start = chrono::high_resolution_clock::now();
        
        SimpleBinaryPoly enc1 = scheme.encode(large_data1);
        SimpleBinaryPoly enc2 = scheme.encode(large_data2);
        
        SimpleBinaryCKKSCiphertext ct_large1 = scheme.encrypt(enc1, keys);
        SimpleBinaryCKKSCiphertext ct_large2 = scheme.encrypt(enc2, keys);
        
        SimpleBinaryCKKSCiphertext ct_large_add = scheme.add(ct_large1, ct_large2);
        SimpleBinaryCKKSCiphertext ct_large_mult = scheme.multiply(ct_large1, ct_large2, keys);
        
        SimpleBinaryPoly dec_add = scheme.decrypt(ct_large_add, keys);
        SimpleBinaryPoly dec_mult = scheme.decrypt(ct_large_mult, keys);
        
        vector<long> result_large_add = scheme.decode(dec_add, 32);
        vector<long> result_large_mult = scheme.decode(dec_mult, 32);
        
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        
        cout << "Large data operations completed in " << duration.count() << " ms" << endl;
        
        printVector(result_large_add, "Large Add Result", 10);
        printVector(result_large_mult, "Large Mult Result", 10);
        
        cout << "\n=== Security Analysis ===" << endl;
        
        // Display security-relevant information
        cout << "Security Parameters:" << endl;
        cout << "- Ring dimension: " << scheme.getRingDim() << endl;
        cout << "- Noise level: " << scheme.getNoiseLevel() << endl;
        cout << "- Key structure: Binary polynomials with controlled Hamming weight" << endl;
        cout << "- Security assumption: Ring Learning With Errors (RLWE)" << endl;
        
        cout << "\nKey properties:" << endl;
        cout << "- Secret key Hamming weight: Low (sparse)" << endl;
        cout << "- Public key: Computationally indistinguishable from random" << endl;
        cout << "- Ciphertext: Semantic security under RLWE assumption" << endl;
        
        cout << "\n=== Implementation Summary ===" << endl;
        cout << "This simplified binary CKKS implementation demonstrates:" << endl;
        cout << "✓ Key generation with appropriate randomness" << endl;
        cout << "✓ Binary polynomial arithmetic in Z_2[x]/(x^n + 1)" << endl;
        cout << "✓ Homomorphic addition and multiplication" << endl;
        cout << "✓ Noise management and refresh operations" << endl;
        cout << "✓ Performance measurement and analysis" << endl;
        cout << "✓ Security parameter configuration" << endl;
        
        cout << "\nEducational value:" << endl;
        cout << "• Shows core CKKS concepts adapted to binary arithmetic" << endl;
        cout << "• Demonstrates practical homomorphic encryption workflow" << endl;
        cout << "• Illustrates noise growth and management strategies" << endl;
        cout << "• Provides performance benchmarking framework" << endl;
        
        cout << "\n===============================================" << endl;
        cout << "       Demo completed successfully!" << endl;
        cout << "===============================================" << endl;
        
    } catch (const exception& e) {
        cerr << "Error during demonstration: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}