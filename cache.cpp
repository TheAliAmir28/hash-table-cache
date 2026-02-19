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
    m_currentTable = new Person*[m_currentCap];
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
int Cache::locateInsertionSlot(Person& p, Person** table, int capacity, prob_t policy) {
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
// personExists: check if the person exists in the table
bool Cache::personExists(Person& p, Person** table, int capacity, prob_t policy) {
    // Compute hash
    int hashedKey = m_hash(p.getKey());
    // Loop
    for (int i = 0; i < capacity; i++) {
        // Get index
        int index = probeIndex(hashedKey, i, capacity, policy);
        // If index location is nullptr, then person cannot be anywhere beyond
        if (table[index] == nullptr) {
            return false;
        // If a person exists at the index
        } else if (table[index] != nullptr && table[index]->m_used == true) {
            // Compare person's ID and Key and return true if match
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
    m_currentTable = new Person*[m_currentCap];
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
// reinsertFromOld: Grab the person from old table and insert into current table
void Cache::reinsertFromOld(Person* p) {
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
        // If the index contains a person
        if (m_oldTable[i] != nullptr && m_oldTable[i]->m_used == true) {
            // Store the person and insert into the new table
            Person* p = m_oldTable[i];
            // Remove person from old table
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
bool Cache::insert(Person person){
    // Insert causes the transfer
    if (m_oldTable != nullptr) {
        transferPartOfTable();
    }
    // Validate ID
    if (person.getID() < MINID || person.getID() > MAXID) {
        return false;
    }    
    // Check is the person already exists in the current and old tables
    if (personExists(person, m_currentTable, m_currentCap, m_currProbing) == true) {
        return false;
    } else if (m_oldTable != nullptr && personExists(person, m_oldTable, m_oldCap, m_oldProbing) == true) {
        return false;
    }
    // Get the insertion spot
    int insertionSpot = locateInsertionSlot(person, m_currentTable, m_currentCap, m_currProbing);
    // Validate insertion spot
    if (insertionSpot == -1) {
        return false;
    }
    // If the spot was previously used, then decrement the number deleted since the spot is now goin to be occupied again.
    if (m_currentTable[insertionSpot] != nullptr && m_currentTable[insertionSpot]->m_used == false) {
        m_currNumDeleted--;
    }
    // Allocate new person
    Person* newPerson = new Person(person);
    // Insert to the spot and set values
    m_currentTable[insertionSpot] = newPerson;
    newPerson->m_used = true;
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
// findPersonIndex: Get the exact index where the Person lives
int Cache::findPersonIndex(Person& p, Person** table, int capacity, prob_t policy) const {
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
        // If person exists in that index
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
bool Cache::remove(Person person){
    // Remove causes the transfer
    if (m_oldTable != nullptr) {
        transferPartOfTable();
    }

    bool removedFromCurrTable = false;
    // Get index of the person
    int personIndex = findPersonIndex(person, m_currentTable, m_currentCap, m_currProbing);
    // Validate index
    if (personIndex >= 0) {
        // If index is valid, then set the person's m_used to false and decrement counter
        m_currentTable[personIndex]->m_used = false;
        m_currNumDeleted++;
        removedFromCurrTable = true;
    }
    // Check old table
    if (removedFromCurrTable == false && m_oldTable != nullptr) {
        int oldTableIndex = findPersonIndex(person, m_oldTable, m_oldCap, m_oldProbing);
        // If person exists in old table, then set the person's m_used to false and decrement counter
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
// getPerson: Looks for the Person object with the sequence and the ID in the database
const Person Cache::getPerson(string key, int ID) const{
    // Create temporary person to function call
    Person tempPerson;
    tempPerson.m_key = key;
    tempPerson.m_id = ID;
    tempPerson.m_used = true;
    // Get index
    int personIndex = findPersonIndex(tempPerson, m_currentTable, m_currentCap, m_currProbing);
    // Validate index
    if (personIndex >= 0) {
        // Return person
        return *m_currentTable[personIndex];
    }
    // Check old table
    if (m_oldTable != nullptr) {
        
        int personOldIndex = findPersonIndex(tempPerson, m_oldTable, m_oldCap, m_oldProbing);
        // Validate index
        if (personOldIndex >= 0) {
            // Return person
            return *m_oldTable[personOldIndex];
        }
    }
    // Return empty person if not found
    return Person("", 0, false);
}
// updateID: Looks for the Person object in the database
bool Cache::updateID(Person person, int ID){
    if (ID < MINID || ID > MAXID) {
        return false;
    }

    Person tempPerson;
    tempPerson.m_key = person.getKey();
    tempPerson.m_id = person.getID();
    tempPerson.m_used = true;

    int index = findPersonIndex(tempPerson, m_currentTable, m_currentCap, m_currProbing);

    if (index >= 0) {
        m_currentTable[index]->m_id = ID;
        return true;
    } else {
        if (m_oldTable != nullptr) {
            int oldIndex = findPersonIndex(tempPerson, m_oldTable, m_oldCap, m_oldProbing);

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
