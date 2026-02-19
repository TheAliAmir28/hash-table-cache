// Naive Cache Implementation - Full Rehashing Version
#include "naive_cache.h"

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
    m_currentTable = new Person*[m_currentCap];
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
    Person** newTable = new Person*[newCapacity];
    for (int i = 0; i < newCapacity; i++) {
        newTable[i] = nullptr;
    }
    
    // TRANSFER EVERYTHING AT ONCE (this causes the spike!)
    for (int i = 0; i < m_currentCap; i++) {
        if (m_currentTable[i] != nullptr && m_currentTable[i]->getUsed()) {
            Person* p = m_currentTable[i];
            
            // Find slot in new table using current probing policy
            int hashedKey = m_hash(p->getKey());
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
bool NaiveCache::insert(Person person) {
    // Validate ID
    if (person.getID() < MINID || person.getID() > MAXID) {
        return false;
    }
    
    // Check if already exists
    if (personExists(person, m_currentTable, m_currentCap, m_currProbing)) {
        return false;
    }
    
    // Find insertion slot
    int insertionSpot = locateInsertionSlot(person, m_currentTable, m_currentCap, m_currProbing);
    if (insertionSpot == -1) {
        return false;
    }
    
    // Reuse deleted slot if applicable
    if (m_currentTable[insertionSpot] != nullptr && !m_currentTable[insertionSpot]->getUsed()) {
        m_currNumDeleted--;
    }
    
    // Insert new person
    Person* newPerson = new Person(person);
    m_currentTable[insertionSpot] = newPerson;
    newPerson->setUsed(true);
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
bool NaiveCache::remove(Person person) {
    int personIndex = findPersonIndex(person, m_currentTable, m_currentCap, m_currProbing);
    
    if (personIndex >= 0) {
        m_currentTable[personIndex]->setUsed(false);
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

// Get person operation
const Person NaiveCache::getPerson(string key, int ID) const {
    Person tempPerson;
    tempPerson.setKey(key);
    tempPerson.setID(ID);
    tempPerson.setUsed(true);
    
    int personIndex = findPersonIndex(tempPerson, m_currentTable, m_currentCap, m_currProbing);
    
    if (personIndex >= 0) {
        return *m_currentTable[personIndex];
    }
    
    // Return empty person if not found
    return Person("", 0, false);
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
int NaiveCache::probeIndex(int hashedKey, int i, int capacity, prob_t policy) const {
    int value = 0;
    
    if (policy == LINEAR) {
        value = (hashedKey + i) % capacity;
    } else if (policy == QUADRATIC) {
        value = (hashedKey + i * i) % capacity;
    } else if (policy == DOUBLEHASH) {
        int dbValue = 11 - (hashedKey % 11);
        if (dbValue == 0) {
            dbValue = 1;
        }
        value = (hashedKey + i * dbValue) % capacity;
    }
    
    return value;
}

// Locate insertion slot
int NaiveCache::locateInsertionSlot(Person& p, Person** table, int capacity, prob_t policy) {
    int hashedKey = m_hash(p.getKey());
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

// Check if person exists
bool NaiveCache::personExists(Person& p, Person** table, int capacity, prob_t policy) {
    int hashedKey = m_hash(p.getKey());
    
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

// Find person index
int NaiveCache::findPersonIndex(Person& p, Person** table, int capacity, prob_t policy) const {
    int hashedKey = m_hash(p.getKey());
    
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