#include "cache.h"
// Constructor
Cache::Cache(int size, hash_fn hash, prob_t probing = DEFPOLCY){
    // Store hash
    m_hash = hash;
    m_currProbing = probing;
    m_newPolicy = m_currProbing;
    // Initialize table size accordingly
    if (size < MINPRIME) {
        m_currentCap = MINPRIME;
    } else if (size > MAXPRIME) {
        m_currentCap = MAXPRIME;
    } else if (isPrime(size) == false) {
        m_currentCap = findNextPrime(size);
    } else {
        m_currentCap = size;
    }
    // Allocate current table
    m_currentTable = new CacheEntry*[m_currentCap];
    for (int i = 0; i < m_currentCap; i++) {
        m_currentTable[i] = nullptr;
    }
    // Initialize table counters
    m_currentSize = 0;
    m_currNumDeleted = 0;
    // Initialize old fields
    m_oldTable = nullptr;
    m_oldCap = 0;
    m_oldSize = 0;
    m_oldNumDeleted = 0;
    m_oldProbing = m_currProbing;
    // Initialize transfer index
    m_transferIndex = 0;
}
// Destructor: Deallocates the memory
Cache::~Cache(){
    if (m_currentTable != nullptr) {
        // Loop through array and deallocate all the memory
        for (int i = 0; i < m_currentCap; i++) {
            if (m_currentTable[i] != nullptr) {
                delete m_currentTable[i];
                m_currentTable[i] = nullptr;
            }
        }
        // Delete array and set to nullptr
        delete [] m_currentTable;
        m_currentTable = nullptr;
    }

    m_currentCap = 0;
    m_currentSize = 0;
    m_currNumDeleted = 0;

    if (m_oldTable != nullptr) {
        // Loop through old array
        for (int i = 0; i < m_oldCap; i++) {
            if (m_oldTable[i] != nullptr) {
                delete m_oldTable[i];
                m_oldTable[i] = nullptr;
            }
        }
        // Delete old array and set to nullptr
        delete [] m_oldTable;
        m_oldTable = nullptr;  
    }
    
    m_oldCap = 0;
    m_oldSize = 0;
    m_oldNumDeleted = 0;
}
// changeProbPolicy: Change the collision handling policy of the hash table at the runtime
void Cache::changeProbPolicy(prob_t policy){
    // Store new policy for next rehash
    m_newPolicy = policy;
}
//probeIndex: Get the right index
int Cache::probeIndex(int hashedKey, int i, int capacity, prob_t policy) const {
    int value = 0;
    // Linear probing formula
    if (policy == LINEAR) {
        value = (hashedKey + i) % capacity;
    // Quadratic probing formula
    } else if (policy == QUADRATIC) {
        value = (hashedKey + i * i) % capacity;
    // Double hashing formula
    } else if (policy == DOUBLEHASH) {
        // if secondary hash function is 0, then set it to 1 to prevent infinite loop
        int dbValue = 11 - (hashedKey % 11);
        if (dbValue == 0) {
            dbValue = 1;
        }
        // Compute double hashing index
        value = (hashedKey + i * dbValue) % capacity;
    }
    return value;
}
// locateInesrtionSlot: Probe table and get valid empty slot index
int Cache::locateInsertionSlot(CacheEntry& p, CacheEntry** table, int capacity, prob_t policy) {
    // Compute hash
    int hashedKey = m_hash(p.getKey());
    int index = 0;
    // Loop through table to locate appropriate spot
    for (int i = 0; i < capacity; i++) {
        // Get index based on probing method
        index = probeIndex(hashedKey, i, capacity, policy);
        // Check the index location in the table
        if (table[index] == nullptr) {
            return index;
        } else if (table[index]->m_used == false) {
            return index;
        }
    }
    return -1;
}
// entryExists: check if the entry exists in the table
bool Cache::entryExists(CacheEntry& p, CacheEntry** table, int capacity, prob_t policy) {
    // Compute hash
    int hashedKey = m_hash(p.getKey());
    // Loop
    for (int i = 0; i < capacity; i++) {
        // Get index
        int index = probeIndex(hashedKey, i, capacity, policy);
        // If index location is nullptr, then entry cannot be anywhere beyond
        if (table[index] == nullptr) {
            return false;
        // If an entry exists at the index
        } else if (table[index] != nullptr && table[index]->m_used == true) {
            // Compare entry's ID and Key and return true if match
            if (table[index]->getID() == p.getID() && table[index]->getKey() == p.getKey()) {
                return true;
            }
        } else if (table[index] != nullptr && table[index]->m_used == false) {
            continue;
        }
    }
    return false;
}

// startNewHash: Initiate new hash when load factor exceeds 0.5 or deleted ratio exceeds 0.8
void Cache::startNewRehash() {
    // Move current table into old table
    m_oldTable = m_currentTable;
    m_oldCap = m_currentCap;
    m_oldSize = m_currentSize;
    m_oldNumDeleted = m_currNumDeleted;
    m_oldProbing = m_currProbing;
    // Compute live elements
    int liveCount = m_currentSize - m_currNumDeleted;
    int value = liveCount * 4;
    // Get new capacity
    int newCapacity = findNextPrime(value);
    // Allocate new table
    m_currentCap = newCapacity;
    m_currentTable = new CacheEntry*[m_currentCap];
    for (int i = 0; i < m_currentCap; i++) {
        m_currentTable[i] = nullptr;
    }
    // Reset new table values
    m_currentSize = 0;
    m_currNumDeleted = 0;
    // Apply new probing functionality
    m_currProbing = m_newPolicy;
    // Rehashing moves 25% of old table per opertion so reset transfer progress
    m_transferIndex = 0;
    
}
// reinsertFromOld: Grab the entry from old table and insert into current table
void Cache::reinsertFromOld(CacheEntry* p) {
    int hashedKey = m_hash(p->getKey());

    for (int i = 0; i < m_currentCap; i++) {
        int index = probeIndex(hashedKey, i, m_currentCap, m_currProbing);

        if (m_currentTable[index] == nullptr) {
            m_currentTable[index] = p;
            m_currentTable[index]->m_used = true;
            m_currentSize++;
            return;
        } else if (m_currentTable[index] != nullptr && m_currentTable[index]->m_used == false) {
            m_currentTable[index] = p;
            m_currentTable[index]->m_used = true;
            m_currentSize++;
            return;
        } else if (m_currentTable[index] != nullptr && m_currentTable[index]->m_used == true) {
            continue;
        }
    }
}
// transferPartOfTable: Moves 25% of old table into the new current table
void Cache::transferPartOfTable() {
    // If old table is null, then there is nothing to transfer
    if (m_oldTable == nullptr) {
        return;
    }
    // Calculate the 25% that needs to be transferred
    int partToTransfer = floor(m_oldCap * 0.25);
    // Loop throught the old table
    for (int i = m_transferIndex; i < (m_transferIndex + partToTransfer) && i < m_oldCap; i++) {
        // If the index contains an entry
        if (m_oldTable[i] != nullptr && m_oldTable[i]->m_used == true) {
            // Store the entry and insert into the new table
            CacheEntry* p = m_oldTable[i];
            // Remove entry from old table
            m_oldTable[i] = nullptr;
            m_oldSize--;
            // Insert into new table
            reinsertFromOld(p);
        } else if (m_oldTable[i] != nullptr && m_oldTable[i]->m_used == false) {
            delete m_oldTable[i];
            m_oldTable[i] = nullptr;
            m_oldSize--;
            m_oldNumDeleted--;
        }
    }
    // Update transfer index for next transfers
    m_transferIndex += partToTransfer;
    // Check if transfer is finished. If so, then old table can be deleted entirely
    if (m_transferIndex >= m_oldCap) {
        for (int i = 0; i < m_oldCap; i++) {
            if (m_oldTable[i] != nullptr) {
                delete m_oldTable[i];
                m_oldTable[i] = nullptr;
            }
        }
        delete [] m_oldTable;

        m_oldTable = nullptr;
        m_oldCap = 0;
        m_oldSize = 0;
        m_oldNumDeleted = 0;
    }
}

// insert: Inserts an object into the current hash table
bool Cache::insert(CacheEntry entry){
    // Insert causes the transfer
    if (m_oldTable != nullptr) {
        transferPartOfTable();
    }
    // Validate ID
    if (entry.getID() < MINID || entry.getID() > MAXID) {
        return false;
    }    
    // Check if the entry already exists in the current and old tables
    if (entryExists(entry, m_currentTable, m_currentCap, m_currProbing) == true) {
        return false;
    } else if (m_oldTable != nullptr && entryExists(entry, m_oldTable, m_oldCap, m_oldProbing) == true) {
        return false;
    }
    // Get the insertion spot
    int insertionSpot = locateInsertionSlot(entry, m_currentTable, m_currentCap, m_currProbing);
    // Validate insertion spot
    if (insertionSpot == -1) {
        return false;
    }
    // If the spot was previously used, then decrement the number deleted since the spot is now goin to be occupied again.
    if (m_currentTable[insertionSpot] != nullptr && m_currentTable[insertionSpot]->m_used == false) {
        m_currNumDeleted--;
    }
    // Allocate new entry
    CacheEntry* newEntry = new CacheEntry(entry);
    // Insert to the spot and set values
    m_currentTable[insertionSpot] = newEntry;
    newEntry->m_used = true;
    m_currentSize++;

    if (m_oldTable == nullptr) {
        // Calculate load factor
        float loadFactor = lambda();
        float delRatio = deletedRatio();
        // If load factor exceeds 0.5 or delete ratio exceeds 0.8, then we initiate a new rehash
        if (loadFactor > 0.5 || delRatio > 0.8) {
            startNewRehash();
        }
    }
    return true;
}
// findEntryIndex: Get the exact index where the CacheEntry lives
int Cache::findEntryIndex(CacheEntry& p, CacheEntry** table, int capacity, prob_t policy) const {
    // Compute hash
    int hashedKey = m_hash(p.getKey());
    // Loop through the table
    for (int i = 0; i < capacity; i++) {
        // Get index
        int index = probeIndex(hashedKey, i, capacity, policy);
        // Check index location of the table
        if (table[index] == nullptr) {
            return -1;
        } else if (table[index] != nullptr && table[index]->m_used == false) {
            continue;
        // If entry exists in that index
        } else if (table[index] != nullptr && table[index]->m_used == true) {
            // Compare key and ID and return true if match
            if (table[index]->getID() == p.getID() && table[index]->getKey() == p.getKey()) {
                return index;
            }
        }
    }
    return -1;
}
// remove: Removes a data point from either the current hash table or the old hash table where the object is stored
bool Cache::remove(CacheEntry entry){
    // Remove causes the transfer
    if (m_oldTable != nullptr) {
        transferPartOfTable();
    }

    bool removedFromCurrTable = false;
    // Get index of the entry
    int entryIndex = findEntryIndex(entry, m_currentTable, m_currentCap, m_currProbing);
    // Validate index
    if (entryIndex >= 0) {
        // If index is valid, then set the entry's m_used to false and decrement counter
        m_currentTable[entryIndex]->m_used = false;
        m_currNumDeleted++;
        removedFromCurrTable = true;
    }
    // Check old table
    if (removedFromCurrTable == false && m_oldTable != nullptr) {
        int oldTableIndex = findEntryIndex(entry, m_oldTable, m_oldCap, m_oldProbing);
        // If entry exists in old table, then set the entry's m_used to false and decrement counter
        if (oldTableIndex >= 0) {
            m_oldTable[oldTableIndex]->m_used = false;
            m_oldNumDeleted++;
            return true;
        }
    }
    // Check for rehash and make sure no hash is in progress.
    if (removedFromCurrTable && m_oldTable == nullptr) {
        // Calculate delete ratio and load factor
        float loadFactor = lambda();
        float delRatio = deletedRatio();
        // If load factor exceeds 0.5 or delete ratio exceeds 0.8, then we initiate a new rehash
        if (loadFactor > 0.5 || delRatio > 0.8) {
            startNewRehash();
        }
    }
    return removedFromCurrTable;
}
// getCacheEntry: Looks for the CacheEntry object with the sequence and the ID in the database
const CacheEntry Cache::getCacheEntry(string key, int ID) const{
    // Create temporary entry to function call
    CacheEntry tempEntry;
    tempEntry.m_key = key;
    tempEntry.m_id = ID;
    tempEntry.m_used = true;
    // Get index
    int entryIndex = findEntryIndex(tempEntry, m_currentTable, m_currentCap, m_currProbing);
    // Validate index
    if (entryIndex >= 0) {
        // Return entry
        return *m_currentTable[entryIndex];
    }
    // Check old table
    if (m_oldTable != nullptr) {
        
        int entryOldIndex = findEntryIndex(tempEntry, m_oldTable, m_oldCap, m_oldProbing);
        // Validate index
        if (entryOldIndex >= 0) {
            // Return entry
            return *m_oldTable[entryOldIndex];
        }
    }
    // Return empty entry if not found
    return CacheEntry("", 0, false);
}
// updateID: Looks for the CacheEntry object in the database
bool Cache::updateID(CacheEntry entry, int ID){
    if (ID < MINID || ID > MAXID) {
        return false;
    }

    CacheEntry tempEntry;
    tempEntry.m_key = entry.getKey();
    tempEntry.m_id = entry.getID();
    tempEntry.m_used = true;

    int index = findEntryIndex(tempEntry, m_currentTable, m_currentCap, m_currProbing);

    if (index >= 0) {
        m_currentTable[index]->m_id = ID;
        return true;
    } else {
        if (m_oldTable != nullptr) {
            int oldIndex = findEntryIndex(tempEntry, m_oldTable, m_oldCap, m_oldProbing);

            if (oldIndex >= 0) {
                m_oldTable[oldIndex]->m_id = ID;
                return true;
            }
        }
    }

    return false;
    
}
// lambda: Returns the load factor of the current hash table
float Cache::lambda() const {
    return float(m_currentSize) / m_currentCap;
      
}
// deletedRatio: Returns the ratio of the deleted buckets to the total number of occupied buckets
float Cache::deletedRatio() const {
    // Cannot divide by 0
    if (m_currentSize == 0) {
        return 0.0;
    }
    return float(m_currNumDeleted) / m_currentSize;
}

void Cache::dump() const {
    cout << "Dump for the current table: " << endl;
    if (m_currentTable != nullptr)
        for (int i = 0; i < m_currentCap; i++) {
            cout << "[" << i << "] : " << m_currentTable[i] << endl;
        }
    cout << "Dump for the old table: " << endl;
    if (m_oldTable != nullptr)
        for (int i = 0; i < m_oldCap; i++) {
            cout << "[" << i << "] : " << m_oldTable[i] << endl;
        }
}

bool Cache::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int Cache::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}
