// Naive Cache with Full Rehashing (for benchmarking comparison)
// This implementation rehashes ALL data at once instead of incrementally
#ifndef NAIVE_CACHE_H
#define NAIVE_CACHE_H

#include "cache.h"

// Same interface as Cache but with full rehashing
class NaiveCache {
public:
    NaiveCache(int size, hash_fn hash, prob_t probing = DEFPOLCY);
    ~NaiveCache();
    
    // Core operations (same as Cache)
    bool insert(Person person);
    bool remove(Person person);
    const Person getPerson(string key, int ID) const;
    
    // Expose for benchmarking
    int getCurrentSize() const { return m_currentSize; }
    int getCurrentCap() const { return m_currentCap; }
    float lambda() const;
    float deletedRatio() const;
    
private:
    // FULL REHASHING
    void fullRehash();
    
    // Helper functions (same as Cache)
    int probeIndex(int hashedKey, int i, int capacity, prob_t policy) const;
    int locateInsertionSlot(Person& p, Person** table, int capacity, prob_t policy);
    bool personExists(Person& p, Person** table, int capacity, prob_t policy);
    int findPersonIndex(Person& p, Person** table, int capacity, prob_t policy) const;
    
    // Utility functions
    bool isPrime(int number);
    int findNextPrime(int current);
    
    // Member variables (simpler than Cache - no old table!)
    Person**   m_currentTable;
    int        m_currentCap;
    int        m_currentSize;
    int        m_currNumDeleted;
    hash_fn    m_hash;
    prob_t     m_currProbing;
};

#endif // NAIVE_CACHE_H
