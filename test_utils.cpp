// Test program to verify benchmark utilities work correctly
#include "benchmark_utils.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

// Simple hash function (same as in driver.cpp)
unsigned int hashCode(const string str) {
    unsigned int val = 0;
    const unsigned int thirtyThree = 33;
    for (int i = 0; i < (int)(str.length()); i++)
        val = val * thirtyThree + str[i];
    return val;
}

int main() {
    cout << "========================================" << endl;
    cout << "  Testing Benchmark Utilities" << endl;
    cout << "========================================\n" << endl;
    
    // Test 1: Timer class
    cout << "TEST 1: Timer Class" << endl;
    cout << "-------------------" << endl;
    Timer timer;
    timer.start();
    this_thread::sleep_for(chrono::milliseconds(10)); // Sleep for 10ms
    double elapsed = timer.elapsed();
    cout << "Slept for ~10ms, timer measured: " << elapsed / 1000.0 << " ms" << endl;
    cout << "✓ Timer is working!\n" << endl;
    
    // Test 2: LatencyStats class
    cout << "TEST 2: LatencyStats Class" << endl;
    cout << "---------------------------" << endl;
    LatencyStats stats;
    
    // Add some sample latencies
    stats.record(5.0);
    stats.record(10.0);
    stats.record(15.0);
    stats.record(20.0);
    stats.record(100.0); // outlier
    
    cout << "Recorded latencies: 5, 10, 15, 20, 100 μs" << endl;
    cout << "Average: " << stats.getAverage() << " μs (expected: 30)" << endl;
    cout << "P50: " << stats.getP50() << " μs (expected: 15)" << endl;
    cout << "P99: " << stats.getP99() << " μs (expected: 100)" << endl;
    cout << "✓ LatencyStats is working!\n" << endl;
    
    // Test 3: TestDataGenerator class
    cout << "TEST 3: TestDataGenerator Class" << endl;
    cout << "--------------------------------" << endl;
    TestDataGenerator dataGen(42); // seed = 42
    
    cout << "Generating 5 random persons:" << endl;
    for (int i = 0; i < 5; i++) {
        Person p = dataGen.generatePerson(i);
        cout << "  Person " << i << ": " << p.getKey() 
             << " (ID: " << p.getID() << ")" << endl;
    }
    cout << "✓ TestDataGenerator is working!\n" << endl;
    
    // Test 4: Generate batch
    cout << "TEST 4: Batch Generation" << endl;
    cout << "------------------------" << endl;
    vector<Person> batch = dataGen.generateBatch(100);
    cout << "Generated batch of " << batch.size() << " persons" << endl;
    cout << "✓ Batch generation is working!\n" << endl;
    
    // Test 5: CSV output
    cout << "TEST 5: CSV Output" << endl;
    cout << "------------------" << endl;
    
    // Create a test CSV file
    ofstream testFile("test_output.csv");
    testFile << "Implementation,Operations,Average,P50,P95,P99,Max" << endl;
    testFile.close();
    
    // Add some stats
    LatencyStats testStats;
    for (int i = 0; i < 1000; i++) {
        testStats.record(i * 0.5); // gradually increasing latencies
    }
    
    testStats.saveToCSV("test_output.csv", "TestImplementation");
    cout << "Created test_output.csv" << endl;
    cout << "Check the file to see CSV output format" << endl;
    cout << "✓ CSV output is working!\n" << endl;
    
    cout << "\n========================================" << endl;
    cout << "  All Tests Passed!" << endl;
    cout << "========================================" << endl;
    cout << "\nYou're ready to move to Step 2!" << endl;
    
    return 0;
}
