// Test program to verify NaiveCache works correctly
#include "naive_cache.h"
#include "benchmark_utils.h"
#include <iostream>

using namespace std;

// Hash function (same as driver.cpp)
unsigned int hashCode(const string str) {
    unsigned int val = 0;
    const unsigned int thirtyThree = 33;
    for (int i = 0; i < (int)(str.length()); i++)
        val = val * thirtyThree + str[i];
    return val;
}

int main() {
    cout << "========================================" << endl;
    cout << "  Testing NaiveCache Implementation" << endl;
    cout << "========================================\n" << endl;
    
    TestDataGenerator dataGen(42);
    NaiveCache cache(MINPRIME, hashCode, DOUBLEHASH);
    
    // Test 1: Basic insertion
    cout << "TEST 1: Basic Insertion" << endl;
    cout << "------------------------" << endl;
    
    vector<Person> testData;
    for (int i = 0; i < 10; i++) {
        Person p = dataGen.generatePerson(i);
        testData.push_back(p);
        
        if (cache.insert(p)) {
            cout << "✓ Inserted: " << p.getKey() << " (ID: " << p.getID() << ")" << endl;
        } else {
            cout << "✗ Failed to insert!" << endl;
            return 1;
        }
    }
    
    cout << "\nCurrent size: " << cache.getCurrentSize() << " / " << cache.getCurrentCap() << endl;
    cout << "Load factor: " << cache.lambda() << endl;
    
    // Test 2: Retrieval
    cout << "\nTEST 2: Retrieval" << endl;
    cout << "-----------------" << endl;
    
    bool allFound = true;
    for (int i = 0; i < 10; i++) {
        Person found = cache.getPerson(testData[i].getKey(), testData[i].getID());
        if (found.getKey() == testData[i].getKey() && found.getID() == testData[i].getID()) {
            cout << "✓ Found: " << found.getKey() << " (ID: " << found.getID() << ")" << endl;
        } else {
            cout << "✗ Not found!" << endl;
            allFound = false;
        }
    }
    
    if (!allFound) {
        cout << "\n✗ Some items not found!" << endl;
        return 1;
    }
    
    // Test 3: Trigger rehashing
    cout << "\nTEST 3: Trigger Rehashing" << endl;
    cout << "--------------------------" << endl;
    
    int oldCapacity = cache.getCurrentCap();
    cout << "Initial capacity: " << oldCapacity << endl;
    cout << "Inserting more items to trigger rehash..." << endl;
    
    // Insert enough to trigger rehash
    for (int i = 10; i < 100; i++) {
        Person p = dataGen.generatePerson(i);
        cache.insert(p);
        
        if (cache.getCurrentCap() != oldCapacity) {
            cout << "✓ Rehash triggered!" << endl;
            cout << "  Old capacity: " << oldCapacity << endl;
            cout << "  New capacity: " << cache.getCurrentCap() << endl;
            break;
        }
    }
    
    // Test 4: Verify data after rehash
    cout << "\nTEST 4: Verify Data After Rehash" << endl;
    cout << "----------------------------------" << endl;
    
    bool allStillThere = true;
    for (int i = 0; i < 10; i++) {
        Person found = cache.getPerson(testData[i].getKey(), testData[i].getID());
        if (found.getKey() != testData[i].getKey() || found.getID() != testData[i].getID()) {
            cout << "✗ Lost data during rehash!" << endl;
            allStillThere = false;
            break;
        }
    }
    
    if (allStillThere) {
        cout << "✓ All data preserved after rehash!" << endl;
    } else {
        return 1;
    }
    
    // Test 5: Removal
    cout << "\nTEST 5: Removal" << endl;
    cout << "---------------" << endl;
    
    if (cache.remove(testData[0])) {
        cout << "✓ Removed: " << testData[0].getKey() << endl;
        
        Person found = cache.getPerson(testData[0].getKey(), testData[0].getID());
        if (found.getKey() == "") {
            cout << "✓ Confirmed removal" << endl;
        } else {
            cout << "✗ Item still found after removal!" << endl;
            return 1;
        }
    } else {
        cout << "✗ Failed to remove!" << endl;
        return 1;
    }
    
    cout << "\n========================================" << endl;
    cout << "  All Tests Passed!" << endl;
    cout << "========================================" << endl;
    cout << "\nNaiveCache is working correctly!" << endl;
    cout << "Ready to compare with incremental rehashing.\n" << endl;
    
    return 0;
}
