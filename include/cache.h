#ifndef CACHE_H
#define CACHE_H
#include <iostream>
#include <string>
#include <cmath>
class Tester;   // forward declaration, will be used for testing
class CacheEntry;   // forward declaration
class Cache;    // forward declaration
const int MINPRIME = 101;   // Min size for hash table
const int MAXPRIME = 99991; // Max size for hash table
const int MINID = 100000;
const int MAXID = 999999;
typedef unsigned int (*hash_fn)(std::string); // declaration of hash function
enum prob_t {QUADRATIC, DOUBLEHASH, LINEAR}; // types of collision handling policy
#define DEFPOLCY QUADRATIC

struct CacheStats {
    int currentCapacity = 0;
    int currentSize = 0;
    int currentDeleted = 0;
    int oldCapacity = 0;
    int oldSize = 0;
    int oldDeleted = 0;
    int liveEntries = 0;
    float loadFactor = 0.0F;
    float deletedRatio = 0.0F;
    bool rehashInProgress = false;
    float rehashProgress = 1.0F;
};

class CacheEntry{
    public:
    friend class Tester;
    friend class Cache;
    CacheEntry(std::string key="", int id=0, bool used=false){
        m_key = key; m_id = id; m_used=used;
    }
    CacheEntry(const CacheEntry& rhs) = default;
    CacheEntry& operator=(const CacheEntry& rhs) = default;
    std::string getKey() const {return m_key;}
    int getID() const {return m_id;}
    void setKey(std::string key){m_key=key;}
    void setID(int id){m_id=id;}
    bool getUsed() const {return m_used;}
    void setUsed(bool used) {m_used=used;}
    // the following function is a friend function
    friend std::ostream& operator<<(std::ostream& sout, const CacheEntry *entry ){
        if ((entry != nullptr) && !(entry->getKey().empty()))
            sout << entry->getKey() << " (" << entry->getID() << ", "<< entry->getUsed() <<  ")";
        else
            sout << "";
        return sout;
    }
    // the following function is a friend function
    friend bool operator==(const CacheEntry& lhs, const CacheEntry& rhs){
        // since the uniqueness of an object is defined by sequence and ID
        // the equality operator considers only those two criteria
        return ((lhs.getKey() == rhs.getKey()) && (lhs.getID() == rhs.getID()));
    }
    // the following function is a class function
    bool operator==(const CacheEntry* rhs) const{
        // since the uniqueness of an object is defined by sequence and ID
        // the equality operator considers only those two criteria
        return ((getKey() == rhs->getKey()) && (getID() == rhs->getID()));
    }
    private:
    std::string m_key;   // the search string used as key in the hash table
    int m_id;       // a unique ID number identifying the object
    // the following variable is used for lazy delete scheme in hash table
    // if it is set to false, it means the bucket in the hash table is free for insert
    // if it is set to true, it means the bucket contains live data, and we cannot overwrite it
    bool m_used;
};
class Cache{
    public:
    friend class Grader;
    friend class Tester;
    Cache(int size, hash_fn hash, prob_t probing);
    ~Cache();
    // Returns Load factor of the new table
    float lambda() const;
    // Returns the ratio of deleted slots in the new table
    float deletedRatio() const;
    // insert only happens in the new table
    bool insert(const CacheEntry& entry);
    // remove can happen from either table
    bool remove(const CacheEntry& entry);
    // find can happen in either table
    CacheEntry getCacheEntry(std::string key, int id) const;
    // update the information
    bool updateID(const CacheEntry& entry, int ID);
    CacheStats getStats() const;
    void changeProbPolicy(prob_t policy);
    void dump() const;
    private:
    hash_fn    m_hash;          // hash function
    prob_t     m_newPolicy;     // stores the change of policy request

    CacheEntry**   m_currentTable;  // hash table
    int        m_currentCap;    // hash table size (capacity)
    int        m_currentSize;   // current number of entries
                                // m_currentSize includes deleted entries 
    int        m_currNumDeleted;// number of deleted entries
    prob_t     m_currProbing;   // collision handling policy

    CacheEntry**   m_oldTable;      // hash table
    int        m_oldCap;        // hash table size (capacity)
    int        m_oldSize;       // current number of entries
                                // m_oldSize includes deleted entries
    int        m_oldNumDeleted; // number of deleted entries
    prob_t     m_oldProbing;    // collision handling policy

    int        m_transferIndex; // this can be used as a temporary place holder
                                // during incremental transfer to scanning the table

    //private helper functions
    bool isPrime(int number);
    int findNextPrime(int current);

    /******************************************
    * Private function declarations go here! *
    ******************************************/
    int probeIndex(unsigned int hashedKey, int i, int capacity, prob_t policy) const;
    int locateInsertionSlot(const CacheEntry& entry, CacheEntry** table, int capacity, prob_t policy);
    bool entryExists(const CacheEntry& entry, CacheEntry** table, int capacity, prob_t policy);
    int findEntryIndex(const CacheEntry& entry, CacheEntry** table, int capacity, prob_t policy) const;
    void reinsertFromOld(CacheEntry* entry);
    void startNewRehash();
    void transferPartOfTable();
};
#endif
