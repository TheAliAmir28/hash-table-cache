// Comprehensive Benchmark Suite for Cache Performance Testing
#include "cache.h"
#include "naive_cache.h"
#include "benchmark_utils.h"
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unordered_map>

using namespace std;

// Hash function (same as your project)
unsigned int hashCode(const string str) {
    unsigned int val = 0;
    const unsigned int thirtyThree = 33;
    for (int i = 0; i < (int)(str.length()); i++)
        val = val * thirtyThree + str[i];
    return val;
}

double median(vector<double> values) {
    if (values.empty()) {
        return 0.0;
    }

    sort(values.begin(), values.end());
    return values[values.size() / 2];
}

template <typename TrialRunner>
double runThroughputTrials(const string& label, int trials, TrialRunner runTrial) {
    vector<double> results;
    results.reserve(trials);

    for (int trial = 0; trial < trials; trial++) {
        double opsPerSec = runTrial(trial);
        results.push_back(opsPerSec);
        cout << "  Trial " << (trial + 1) << "/" << trials << ": "
             << fixed << setprecision(0) << opsPerSec << " ops/sec" << endl;
    }

    double medianOps = median(results);
    cout << "  Median " << label << " throughput: "
         << fixed << setprecision(0) << medianOps << " ops/sec" << endl;
    return medianOps;
}

// ===================================================================
// BENCHMARK 1: Insertion Latency During Rehashing
// ===================================================================
void benchmarkInsertionLatency() {
    cout << "\n========================================" << endl;
    cout << "BENCHMARK 1: Insertion Latency" << endl;
    cout << "========================================" << endl;
    cout << "Testing 10,000 insertions on each implementation..." << endl;
    
    const int NUM_OPERATIONS = 10000;
    TestDataGenerator dataGen;
    
    // Test your incremental rehashing
    {
        cout << "\n[1/3] Testing: Incremental Rehashing Cache..." << endl;
        Cache cache(MINPRIME, hashCode, DOUBLEHASH);
        LatencyStats stats;
        Timer timer;
        
        for (int i = 0; i < NUM_OPERATIONS; i++) {
            CacheEntry p = dataGen.generateEntry(i);
            
            timer.start();
            cache.insert(p);
            double latency = timer.elapsed();
            
            stats.record(latency);
            
            // Print progress every 2000 operations
            if ((i + 1) % 2000 == 0) {
                cout << "  Progress: " << (i + 1) << "/" << NUM_OPERATIONS 
                     << " (P99: " << fixed << setprecision(1) << stats.getP99() << " μs)" << endl;
            }
        }
        
        stats.printSummary("Incremental Rehashing");
        stats.saveToCSV("results/latency.csv", "Incremental");
    }
    
    // Test naive full rehashing
    {
        cout << "\n[2/3] Testing: Naive Full Rehashing..." << endl;
        NaiveCache cache(MINPRIME, hashCode, DOUBLEHASH);
        LatencyStats stats;
        Timer timer;
        
        for (int i = 0; i < NUM_OPERATIONS; i++) {
            CacheEntry p = dataGen.generateEntry(i);
            
            timer.start();
            cache.insert(p);
            double latency = timer.elapsed();
            
            stats.record(latency);
            
            // Print progress every 2000 operations
            if ((i + 1) % 2000 == 0) {
                cout << "  Progress: " << (i + 1) << "/" << NUM_OPERATIONS 
                     << " (P99: " << fixed << setprecision(1) << stats.getP99() << " μs)" << endl;
            }
        }
        
        stats.printSummary("Naive Full Rehashing");
        stats.saveToCSV("results/latency.csv", "Naive");
    }
    
    // Test std::unordered_map (baseline)
    {
        cout << "\n[3/3] Testing: std::unordered_map (baseline)..." << endl;
        unordered_map<string, CacheEntry> stdMap;
        LatencyStats stats;
        Timer timer;
        
        for (int i = 0; i < NUM_OPERATIONS; i++) {
            CacheEntry p = dataGen.generateEntry(i);
            string key = p.getKey() + to_string(p.getID());
            
            timer.start();
            stdMap[key] = p;
            double latency = timer.elapsed();
            
            stats.record(latency);
            
            // Print progress every 2000 operations
            if ((i + 1) % 2000 == 0) {
                cout << "  Progress: " << (i + 1) << "/" << NUM_OPERATIONS 
                     << " (P99: " << fixed << setprecision(1) << stats.getP99() << " μs)" << endl;
            }
        }
        
        stats.printSummary("std::unordered_map");
        stats.saveToCSV("results/latency.csv", "std_unordered_map");
    }
    
    cout << "\n✓ Latency benchmark complete!" << endl;
}

// ===================================================================
// BENCHMARK 2: Throughput Test
// ===================================================================
void benchmarkThroughput() {
    cout << "\n========================================" << endl;
    cout << "BENCHMARK 2: Throughput (ops/sec)" << endl;
    cout << "========================================" << endl;
    cout << "Measuring median operations per second across repeated trials..." << endl;
    
    const int NUM_OPERATIONS = 50000;
    const int WARMUP = 1000;
    const int TRIALS = 5;
    
    // Incremental rehashing
    {
        cout << "\n[1/3] Testing Incremental Rehashing..." << endl;
        double opsPerSec = runThroughputTrials("Incremental", TRIALS, [&](int trial) {
            TestDataGenerator dataGen(100 + trial);
            Cache cache(MINPRIME, hashCode, DOUBLEHASH);

            for (int i = 0; i < WARMUP; i++) {
                cache.insert(dataGen.generateEntry(i));
            }

            auto start = steady_clock::now();
            for (int i = 0; i < NUM_OPERATIONS; i++) {
                cache.insert(dataGen.generateEntry(i + WARMUP));
            }
            auto end = steady_clock::now();

            auto duration = max<long long>(duration_cast<microseconds>(end - start).count(), 1);
            return (NUM_OPERATIONS * 1000000.0) / duration;
        });
        
        // Save to CSV
        ofstream file("results/throughput.csv", ios::app);
        file << "Incremental," << opsPerSec << endl;
        file.close();
    }
    
    // Naive rehashing
    {
        cout << "\n[2/3] Testing Naive Full Rehashing..." << endl;
        double opsPerSec = runThroughputTrials("Naive", TRIALS, [&](int trial) {
            TestDataGenerator dataGen(100 + trial);
            NaiveCache cache(MINPRIME, hashCode, DOUBLEHASH);

            for (int i = 0; i < WARMUP; i++) {
                cache.insert(dataGen.generateEntry(i));
            }

            auto start = steady_clock::now();
            for (int i = 0; i < NUM_OPERATIONS; i++) {
                cache.insert(dataGen.generateEntry(i + WARMUP));
            }
            auto end = steady_clock::now();

            auto duration = max<long long>(duration_cast<microseconds>(end - start).count(), 1);
            return (NUM_OPERATIONS * 1000000.0) / duration;
        });
        
        // Save to CSV
        ofstream file("results/throughput.csv", ios::app);
        file << "Naive," << opsPerSec << endl;
        file.close();
    }
    
    // std::unordered_map
    {
        cout << "\n[3/3] Testing std::unordered_map..." << endl;
        double opsPerSec = runThroughputTrials("std::unordered_map", TRIALS, [&](int trial) {
            TestDataGenerator dataGen(100 + trial);
            unordered_map<string, CacheEntry> stdMap;

            for (int i = 0; i < WARMUP; i++) {
                CacheEntry p = dataGen.generateEntry(i);
                stdMap[p.getKey() + to_string(p.getID())] = p;
            }

            auto start = steady_clock::now();
            for (int i = 0; i < NUM_OPERATIONS; i++) {
                CacheEntry p = dataGen.generateEntry(i + WARMUP);
                stdMap[p.getKey() + to_string(p.getID())] = p;
            }
            auto end = steady_clock::now();

            auto duration = max<long long>(duration_cast<microseconds>(end - start).count(), 1);
            return (NUM_OPERATIONS * 1000000.0) / duration;
        });
        
        // Save to CSV
        ofstream file("results/throughput.csv", ios::app);
        file << "std_unordered_map," << opsPerSec << endl;
        file.close();
    }
    
    cout << "\n✓ Throughput benchmark complete!" << endl;
}

// ===================================================================
// BENCHMARK 3: Rehashing Spike Detection
// ===================================================================
void benchmarkRehashingSpikes() {
    cout << "\n========================================" << endl;
    cout << "BENCHMARK 3: Rehashing Spike Detection" << endl;
    cout << "========================================" << endl;
    cout << "This measures latency specifically during rehashing.\n" << endl;
    
    const int OPS_BEFORE_REHASH = 45; // Close to 50% load factor
    const int OPS_DURING_REHASH = 100;
    TestDataGenerator dataGen;
    
    // Incremental
    {
        cout << "[1/2] Testing Incremental Rehashing..." << endl;
        Cache cache(MINPRIME, hashCode, DOUBLEHASH);
        LatencyStats beforeStats, duringStats;
        Timer timer;
        
        // Insert until close to rehash
        for (int i = 0; i < OPS_BEFORE_REHASH; i++) {
            timer.start();
            cache.insert(dataGen.generateEntry(i));
            beforeStats.record(timer.elapsed());
        }
        
        // Insert during rehashing period
        for (int i = 0; i < OPS_DURING_REHASH; i++) {
            timer.start();
            cache.insert(dataGen.generateEntry(i + OPS_BEFORE_REHASH));
            duringStats.record(timer.elapsed());
        }
        
        cout << "  Before Rehash:" << endl;
        cout << "    Average: " << fixed << setprecision(2) << beforeStats.getAverage() << " μs" << endl;
        cout << "    P99:     " << beforeStats.getP99() << " μs" << endl;
        cout << "    Max:     " << beforeStats.getMax() << " μs" << endl;
        
        cout << "  During Rehash:" << endl;
        cout << "    Average: " << duringStats.getAverage() << " μs" << endl;
        cout << "    P99:     " << duringStats.getP99() << " μs" << endl;
        cout << "    Max:     " << duringStats.getMax() << " μs" << endl;
        
        double spikeRatio = duringStats.getMax() / beforeStats.getMax();
        cout << "  Spike Factor: " << fixed << setprecision(2) << spikeRatio << "x" << endl;
        
        // Save to CSV
        ofstream file("results/spikes.csv", ios::app);
        file << "Incremental," << beforeStats.getMax() << "," << duringStats.getMax() << "," << spikeRatio << endl;
        file.close();
    }
    
    // Naive
    {
        cout << "\n[2/2] Testing Naive Full Rehashing..." << endl;
        NaiveCache cache(MINPRIME, hashCode, DOUBLEHASH);
        LatencyStats beforeStats, duringStats;
        Timer timer;
        
        // Insert until close to rehash
        for (int i = 0; i < OPS_BEFORE_REHASH; i++) {
            timer.start();
            cache.insert(dataGen.generateEntry(i));
            beforeStats.record(timer.elapsed());
        }
        
        // Insert during rehashing period
        for (int i = 0; i < OPS_DURING_REHASH; i++) {
            timer.start();
            cache.insert(dataGen.generateEntry(i + OPS_BEFORE_REHASH));
            duringStats.record(timer.elapsed());
        }
        
        cout << "  Before Rehash:" << endl;
        cout << "    Average: " << fixed << setprecision(2) << beforeStats.getAverage() << " μs" << endl;
        cout << "    P99:     " << beforeStats.getP99() << " μs" << endl;
        cout << "    Max:     " << beforeStats.getMax() << " μs" << endl;
        
        cout << "  During Rehash:" << endl;
        cout << "    Average: " << duringStats.getAverage() << " μs" << endl;
        cout << "    P99:     " << duringStats.getP99() << " μs" << endl;
        cout << "    Max:     " << duringStats.getMax() << " μs" << endl;
        
        double spikeRatio = duringStats.getMax() / beforeStats.getMax();
        cout << "  Spike Factor: " << fixed << setprecision(2) << spikeRatio << "x" << endl;
        
        // Save to CSV
        ofstream file("results/spikes.csv", ios::app);
        file << "Naive," << beforeStats.getMax() << "," << duringStats.getMax() << "," << spikeRatio << endl;
        file.close();
    }
    
    cout << "\n✓ Spike detection complete!" << endl;
}

// ===================================================================
// Print Summary
// ===================================================================
void printSummary() {
    cout << "\n========================================" << endl;
    cout << "   BENCHMARK SUMMARY" << endl;
    cout << "========================================\n" << endl;
    
    // Read and display key metrics
    ifstream latencyFile("results/latency.csv");
    string line;
    getline(latencyFile, line); // skip header
    
    cout << "KEY FINDINGS:" << endl;
    cout << "-------------" << endl;
    
    double incP99 = 0, naiveP99 = 0, incMax = 0, naiveMax = 0;
    while (getline(latencyFile, line)) {
        vector<string> fields;
        string field;
        stringstream row(line);

        while (getline(row, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() >= 7 && fields[0] == "Incremental") {
            incP99 = stod(fields[5]);
            incMax = stod(fields[6]);
        }
        if (fields.size() >= 7 && fields[0] == "Naive") {
            naiveP99 = stod(fields[5]);
            naiveMax = stod(fields[6]);
        }
    }
    latencyFile.close();
    
    if (incP99 > 0 && naiveP99 > 0) {
        double improvement = ((naiveP99 - incP99) / naiveP99) * 100;
        cout << "1. P99 Latency Delta: " << fixed << setprecision(1)
             << improvement << "%" << endl;
        cout << "   - Incremental: " << incP99 << " μs" << endl;
        cout << "   - Naive:       " << naiveP99 << " μs" << endl;
    }

    if (incMax > 0 && naiveMax > 0) {
        double maxReduction = ((naiveMax - incMax) / naiveMax) * 100;
        cout << "\n2. Max Latency Reduction: " << fixed << setprecision(1)
             << maxReduction << "%" << endl;
        cout << "   - Incremental: " << incMax << " us" << endl;
        cout << "   - Naive:       " << naiveMax << " us" << endl;
    }
    
    cout << "\n3. All results saved to results/ directory:" << endl;
    cout << "   - latency.csv" << endl;
    cout << "   - throughput.csv" << endl;
    cout << "   - spikes.csv" << endl;
    
    cout << "\n4. Next steps:" << endl;
    cout << "   - Review CSV files for detailed data" << endl;
    cout << "   - Create graphs using plot_results.py (optional)" << endl;
    cout << "   - Add these results to your resume/portfolio!" << endl;
}

// ===================================================================
// MAIN
// ===================================================================
int main() {
    cout << "========================================" << endl;
    cout << "   HASH TABLE BENCHMARK SUITE" << endl;
    cout << "   Comparing Incremental vs Full Rehashing" << endl;
    cout << "========================================" << endl;
    cout << "\nThis will run 3 comprehensive benchmarks." << endl;
    cout << "Estimated time: 2-3 minutes\n" << endl;
    
    // Create results directory (cross-platform)
    filesystem::create_directories("results");
    
    // Initialize CSV files with headers
    ofstream latencyFile("results/latency.csv");
    latencyFile << "Implementation,Operations,Average,P50,P95,P99,Max" << endl;
    latencyFile.close();
    
    ofstream throughputFile("results/throughput.csv");
    throughputFile << "Implementation,OpsPerSecond" << endl;
    throughputFile.close();
    
    ofstream spikesFile("results/spikes.csv");
    spikesFile << "Implementation,BeforeMax,DuringMax,SpikeRatio" << endl;
    spikesFile.close();
    
    // Run benchmarks
    benchmarkInsertionLatency();
    benchmarkThroughput();
    benchmarkRehashingSpikes();
    
    // Print summary
    printSummary();
    
    cout << "\n========================================" << endl;
    cout << "   BENCHMARKS COMPLETE!" << endl;
    cout << "========================================" << endl;
    
    return 0;
}
