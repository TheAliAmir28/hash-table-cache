// Naive Cache Implementation - Full Rehashing Version
#include "naive_cache.h"

using std::string;

// Constructor
NaiveCache::NaiveCache(int size, hash_fn hash, prob_t probing) {
    m_hash = hash;
    m_currProbing = probing;
    
    // Set capacity to a prime number
    if (size < MINPRIME) {
        m_currentCap = MINPRIME;
    } else if (size > MAXPRIME) {
        m_currentCap = MAXPRIME;
    } else if (!isPrime(size)) {
        m_currentCap = findNextPrime(size);
    } else {
        m_currentCap = size;
    }
    
    // Allocate table
    m_currentTable = new CacheEntry*[m_currentCap];
    for (int i = 0; i < m_currentCap; i++) {
        m_currentTable[i] = nullptr;
    }
    
    m_currentSize = 0;
    m_currNumDeleted = 0;
}

// Destructor
NaiveCache::~NaiveCache() {
    if (m_currentTable != nullptr) {
        for (int i = 0; i < m_currentCap; i++) {
            if (m_currentTable[i] != nullptr) {
                delete m_currentTable[i];
            }
        }
        delete[] m_currentTable;
    }
}

// FULL REHASHING - This is where the latency spike happens!
void NaiveCache::fullRehash() {
    // Calculate new size
    int liveCount = m_currentSize - m_currNumDeleted;
    int newCapacity = findNextPrime(liveCount * 4);
    
    // Allocate new table
    CacheEntry** newTable = new CacheEntry*[newCapacity];
    for (int i = 0; i < newCapacity; i++) {
        newTable[i] = nullptr;
    }
    
    // TRANSFER EVERYTHING AT ONCE (this causes the spike!)
    for (int i = 0; i < m_currentCap; i++) {
        if (m_currentTable[i] != nullptr && m_currentTable[i]->getUsed()) {
            CacheEntry* p = m_currentTable[i];
            
            // Find slot in new table using current probing policy
            unsigned int hashedKey = m_hash(p->getKey());
            for (int j = 0; j < newCapacity; j++) {
                int index = probeIndex(hashedKey, j, newCapacity, m_currProbing);
                if (newTable[index] == nullptr) {
                    newTable[index] = p;
                    break;
                }
            }
        } else if (m_currentTable[i] != nullptr) {
            // Delete tombstones
            delete m_currentTable[i];
        }
    }
    
    // Delete old table
    delete[] m_currentTable;
    
    // Update to new table
    m_currentTable = newTable;
    m_currentCap = newCapacity;
    m_currentSize = liveCount;
    m_currNumDeleted = 0;
}

// Insert operation
bool NaiveCache::insert(const CacheEntry& entry) {
    // Validate ID
    if (entry.getID() < MINID || entry.getID() > MAXID) {
        return false;
    }
    
    // Check if already exists
    if (entryExists(entry, m_currentTable, m_currentCap, m_currProbing)) {
        return false;
    }
    
    // Find insertion slot
    int insertionSpot = locateInsertionSlot(entry, m_currentTable, m_currentCap, m_currProbing);
    if (insertionSpot == -1) {
        return false;
    }
    
    // Reuse deleted slot if applicable
    if (m_currentTable[insertionSpot] != nullptr && !m_currentTable[insertionSpot]->getUsed()) {
        m_currNumDeleted--;
    }
    
    // Insert new entry
    CacheEntry* newEntry = new CacheEntry(entry);
    m_currentTable[insertionSpot] = newEntry;
    newEntry->setUsed(true);
    m_currentSize++;
    
    // Check if rehash needed
    float loadFactor = lambda();
    float delRatio = deletedRatio();
    
    if (loadFactor > 0.5 || delRatio > 0.8) {
        fullRehash();  // THIS CAUSES THE LATENCY SPIKE!
    }
    
    return true;
}

// Remove operation
bool NaiveCache::remove(const CacheEntry& entry) {
    int entryIndex = findEntryIndex(entry, m_currentTable, m_currentCap, m_currProbing);
    
    if (entryIndex >= 0) {
        m_currentTable[entryIndex]->setUsed(false);
        m_currNumDeleted++;
        
        // Check if rehash needed due to deleted ratio
        float loadFactor = lambda();
        float delRatio = deletedRatio();
        
        if (loadFactor > 0.5 || delRatio > 0.8) {
            fullRehash();
        }
        
        return true;
    }
    
    return false;
}

// Get entry operation
CacheEntry NaiveCache::getCacheEntry(string key, int ID) const {
    CacheEntry tempEntry;
    tempEntry.setKey(key);
    tempEntry.setID(ID);
    tempEntry.setUsed(true);
    
    int entryIndex = findEntryIndex(tempEntry, m_currentTable, m_currentCap, m_currProbing);
    
    if (entryIndex >= 0) {
        return *m_currentTable[entryIndex];
    }
    
    // Return empty entry if not found
    return CacheEntry("", 0, false);
}

// Calculate load factor
float NaiveCache::lambda() const {
    return float(m_currentSize) / m_currentCap;
}

// Calculate deleted ratio
float NaiveCache::deletedRatio() const {
    if (m_currentSize == 0) {
        return 0.0;
    }
    return float(m_currNumDeleted) / m_currentSize;
}

// Probe index calculation (same as Cache)
int NaiveCache::probeIndex(unsigned int hashedKey, int i, int capacity, prob_t policy) const {
    int value = 0;
    int base = static_cast<int>(hashedKey % capacity);
    
    if (policy == LINEAR) {
        value = (base + i) % capacity;
    } else if (policy == QUADRATIC) {
        value = static_cast<int>((base + 1LL * i * i) % capacity);
    } else if (policy == DOUBLEHASH) {
        int dbValue = 11 - static_cast<int>(hashedKey % 11);
        if (dbValue == 0) {
            dbValue = 1;
        }
        value = static_cast<int>((base + 1LL * i * dbValue) % capacity);
    }
    
    return value;
}

// Locate insertion slot
int NaiveCache::locateInsertionSlot(const CacheEntry& p, CacheEntry** table, int capacity, prob_t policy) {
    unsigned int hashedKey = m_hash(p.getKey());
    int index = 0;
    
    for (int i = 0; i < capacity; i++) {
        index = probeIndex(hashedKey, i, capacity, policy);
        
        if (table[index] == nullptr) {
            return index;
        } else if (table[index]->getUsed() == false) {
            return index;
        }
    }
    
    return -1;
}

// Check if entry exists
bool NaiveCache::entryExists(const CacheEntry& p, CacheEntry** table, int capacity, prob_t policy) {
    unsigned int hashedKey = m_hash(p.getKey());
    
    for (int i = 0; i < capacity; i++) {
        int index = probeIndex(hashedKey, i, capacity, policy);
        
        if (table[index] == nullptr) {
            return false;
        } else if (table[index] != nullptr && table[index]->getUsed() == true) {
            if (table[index]->getID() == p.getID() && table[index]->getKey() == p.getKey()) {
                return true;
            }
        }
    }
    
    return false;
}

// Find entry index
int NaiveCache::findEntryIndex(const CacheEntry& p, CacheEntry** table, int capacity, prob_t policy) const {
    unsigned int hashedKey = m_hash(p.getKey());
    
    for (int i = 0; i < capacity; i++) {
        int index = probeIndex(hashedKey, i, capacity, policy);
        
        if (table[index] == nullptr) {
            return -1;
        } else if (table[index] != nullptr && table[index]->getUsed() == false) {
            continue;
        } else if (table[index] != nullptr && table[index]->getUsed() == true) {
            if (table[index]->getID() == p.getID() && table[index]->getKey() == p.getKey()) {
                return index;
            }
        }
    }
    
    return -1;
}

// Check if number is prime
bool NaiveCache::isPrime(int number) {
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

// Find next prime number
int NaiveCache::findNextPrime(int current) {
    if (current < MINPRIME) current = MINPRIME - 1;
    
    for (int i = current; i < MAXPRIME; i++) { 
        for (int j = 2; j * j <= i; j++) {
            if (i % j == 0) 
                break;
            else if (j + 1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    
    return MAXPRIME;
}
