// Benchmark Utilities for Cache Performance Testing
#ifndef BENCHMARK_UTILS_H
#define BENCHMARK_UTILS_H

#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <random>
#include "cache.h"

using namespace std;
using namespace std::chrono;

// ===================================================================
// Timer class for measuring operation latency
// ===================================================================
class Timer {
private:
    high_resolution_clock::time_point m_start;
    
public:
    // Start the timer
    void start() {
        m_start = high_resolution_clock::now();
    }
    
    // Returns elapsed time in microseconds (μs)
    // 1 second = 1,000,000 microseconds
    double elapsed() {
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - m_start);
        return duration.count();
    }
};

// ===================================================================
// Statistics calculator for latency measurements
// ===================================================================
class LatencyStats {
private:
    vector<double> m_latencies; // stores all latencies in microseconds
    
public:
    // Record a single latency measurement
    void record(double latency) {
        m_latencies.push_back(latency);
    }
    
    // Clear all recorded data
    void clear() {
        m_latencies.clear();
    }
    
    // Calculate average latency
    double getAverage() const {
        if (m_latencies.empty()) return 0.0;
        double sum = accumulate(m_latencies.begin(), m_latencies.end(), 0.0);
        return sum / m_latencies.size();
    }
    
    // Get a specific percentile
    // percentile should be between 0.0 and 1.0
    // Example: 0.99 for P99 (99th percentile)
    double getPercentile(double percentile) const {
        if (m_latencies.empty()) return 0.0;
        
        // Make a sorted copy of the latencies
        vector<double> sorted = m_latencies;
        sort(sorted.begin(), sorted.end());
        
        // Calculate the index for this percentile
        size_t index = (size_t)(percentile * sorted.size());
        if (index >= sorted.size()) index = sorted.size() - 1;
        
        return sorted[index];
    }
    
    // Convenience functions for common percentiles
    double getP50() const { return getPercentile(0.50); }  // Median
    double getP95() const { return getPercentile(0.95); }  // 95th percentile
    double getP99() const { return getPercentile(0.99); }  // 99th percentile
    double getMax() const { return getPercentile(1.0); }   // Maximum value
    
    // Get the number of recorded operations
    size_t count() const { return m_latencies.size(); }
    
    // Print a summary of statistics to console
    void printSummary(const string& name) const {
        cout << "\n=== " << name << " ===" << endl;
        cout << "Operations: " << count() << endl;
        cout << fixed << setprecision(2);
        cout << "Average:    " << getAverage() << " μs" << endl;
        cout << "P50:        " << getP50() << " μs" << endl;
        cout << "P95:        " << getP95() << " μs" << endl;
        cout << "P99:        " << getP99() << " μs" << endl;
        cout << "Max:        " << getMax() << " μs" << endl;
    }
    
    // Save statistics to CSV file for later graphing
    void saveToCSV(const string& filename, const string& label) const {
        ofstream file(filename, ios::app); // append mode
        file << label << ","
             << count() << ","
             << fixed << setprecision(2)
             << getAverage() << ","
             << getP50() << ","
             << getP95() << ","
             << getP99() << ","
             << getMax() << endl;
        file.close();
    }
};

// ===================================================================
// Test Data Generator
// ===================================================================
class TestDataGenerator {
private:
    mt19937 m_generator;
    uniform_int_distribution<int> m_idDist;
    uniform_int_distribution<int> m_keyDist;
    
    // Sample search strings (same as in driver.cpp)
    const vector<string> searchStr = {"c++", "python", "java", "scheme", 
                                      "prolog", "c#", "c", "js"};
    
public:
    // Constructor with optional seed (default 42 for reproducibility)
    TestDataGenerator(int seed = 42) 
        : m_generator(seed), 
          m_idDist(MINID, MAXID),
          m_keyDist(0, 7) {
    }
    
    // Generate a single random Person
    Person generatePerson(int index) {
        string key = searchStr[m_keyDist(m_generator)];
        int id = m_idDist(m_generator);
        return Person(key, id, true);
    }
    
    // Generate a batch of random Persons
    vector<Person> generateBatch(int count) {
        vector<Person> batch;
        batch.reserve(count); // pre-allocate for efficiency
        for (int i = 0; i < count; i++) {
            batch.push_back(generatePerson(i));
        }
        return batch;
    }
};

#endif // BENCHMARK_UTILS_H
