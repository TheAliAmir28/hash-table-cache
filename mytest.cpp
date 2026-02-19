#include "cache.h"
#include <math.h>
#include <algorithm>
#include <random>
#include <vector>

using namespace std;

const int MINSEARCH = 0;
const int MAXSEARCH = 7;
// the following array defines sample search strings for testing
string searchStr[MAXSEARCH+1]={"c++","python","java","scheme","prolog","c#","c","js"};
enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE};
class Random {
public:
    Random(){}
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor 
            m_normdist = normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = mt19937(10);// 10 is the fixed seed value
            m_unidist = uniform_int_distribution<>(min,max);
        }
        else if (type == UNIFORMREAL) { //the case of UNIFORMREAL to generate real numbers
            m_generator = mt19937(10);// 10 is the fixed seed value
            m_uniReal = uniform_real_distribution<double>((double)min,(double)max);
        }
        else { //the case of SHUFFLE to generate every number only once
            m_generator = mt19937(m_device());
        }
    }
    void setSeed(int seedNum){
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = mt19937(seedNum);
    }
    void init(int min, int max){
        m_min = min;
        m_max = max;
        m_type = UNIFORMINT;
        m_generator = mt19937(10);// 10 is the fixed seed value
        m_unidist = uniform_int_distribution<>(min,max);
    }
    void getShuffle(vector<int> & array){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the user program creates the vector param and passes here
        // here we populate the vector using m_min and m_max
        for (int i = m_min; i<=m_max; i++){
            array.push_back(i);
        }
        shuffle(array.begin(),array.end(),m_generator);
    }

    void getShuffle(int array[]){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the param array must be of the size (m_max-m_min+1)
        // the user program creates the array and pass it here
        vector<int> temp;
        for (int i = m_min; i<=m_max; i++){
            temp.push_back(i);
        }
        shuffle(temp.begin(), temp.end(), m_generator);
        vector<int>::iterator it;
        int i = 0;
        for (it=temp.begin(); it != temp.end(); it++){
            array[i] = *it;
            i++;
        }
    }

    int getRandNum(){
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum(){
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = floor(result*100.0)/100.0;
        return result;
    }

    string getRandString(int size){
        // the parameter size specifies the length of string we ask for
        // to use ASCII char the number range in constructor must be set to 97 - 122
        // and the Random type must be UNIFORMINT (it is default in constructor)
        string output = "";
        for (int i=0;i<size;i++){
            output = output + (char)getRandNum();
        }
        return output;
    }
    
    int getMin(){return m_min;}
    int getMax(){return m_max;}
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    random_device m_device;
    mt19937 m_generator;
    normal_distribution<> m_normdist;//normal distribution
    uniform_int_distribution<> m_unidist;//integer uniform distribution
    uniform_real_distribution<double> m_uniReal;//real uniform distribution
};

unsigned int hashCode(const string str);

unsigned int hashCode(const string str) {
    unsigned int val = 0 ;
    const unsigned int thirtyThree = 33 ;  // magic number from textbook
    for ( int i = 0 ; i < (int)(str.length()); i++)
       val = val * thirtyThree + str[i] ;
    return val ;
}

class Tester {
public:
    // testInsertNoCollide: Test the insertion operation in the hash table with a descent number of data points
    bool testInsertNoCollide() {
        // Create cache with default probing
        Cache c(MINPRIME, hashCode, LINEAR);

        Random rID(MINID, MAXID);
        // Insert 50 data points (persons)
        for (int i = 0; i < 50; i++) {
            string key = "key" + to_string(i);
            int ID = rID.getRandNum();
            // Create person
            Person p(key, ID, true);

            // Hash the key and get the index
            int hashedKey = hashCode(key);
            int index = hashedKey % c.m_currentCap;

            // Insert should succeed
            if (c.insert(p) == false) {
                return false;
            }

            // Check table size
            if (c.m_currentSize != i + 1) {
                return false;
            }

            // Check that the person is in the correct table spot
            if (c.m_currentTable[index] == nullptr) {
                return false;
            }
            // Make sure that the person is there and the keys match
            Person foundPerson = c.getPerson(key, ID);
            if (foundPerson.getKey() != key || foundPerson.getID() != ID) {
                return false;
            }
        }
        return true;
    }
    // testInsertHalfCollide: Test the insertion operation in the hash table with a descent number of data points with half of them using colliding keys, e.g. 50 nodes.
    bool testInsertHalfCollide() {
        Random rID(MINID, MAXID);
        Cache c(MINPRIME, hashCode, LINEAR);
        // Inserting 25 persons that do not collide
        for (int i = 0; i < 25; i++) {
            string key = "key" + to_string(i);
            int ID = rID.getRandNum();
            Person p(key, ID, true);
            // Insert the person
            if (c.insert(p) == false) {
                return false;
            }

            int newSize = i + 1;
            if (c.m_currentSize != newSize) {
                return false;
            }
            
            // Retrieve person from table
            Person foundPerson = c.getPerson(key, ID);
            // Check if it's the matching person
            if (foundPerson.getKey() != key || foundPerson.getID() != ID) {
                return false;
            }
        }
        // Insert 25 persons that do collide
        for (int i = 25; i < 50; i++) {
            string key = "A" + to_string(i);
            int ID = rID.getRandNum();
            Person p(key, ID, true);
            // Insert the person
            if (c.insert(p) == false) {
                return false;
            }

            // Check size
            int newSize = i + 1;
            if (c.m_currentSize != newSize) {
                return false;
            }
        // Retrieve person from table
            Person foundPerson = c.getPerson(key, ID);
            // Check if it's the matching person           
            if (foundPerson.getKey() != key || foundPerson.getID() != ID) {
                return false;
            }
        }
        return true;
    }
    // testFindMissingPerson: Test the find operation (getPerson(...) function) for an error case, the Person object does not exist in the database.
    bool testFindMissingPerson() {
        Random rID(MINID, MAXID);
        Cache c(MINPRIME, hashCode, LINEAR);

        // Insert persons
        for (int i = 0; i < 50; i++) {
            string key = "key" + to_string(i);
            int ID = rID.getRandNum();
            // Create person and insert
            Person p(key, ID, true);

            if (c.insert(p) == false) {
                return false;
            }
        }

        // Test a missing key
        Person missingPerson = c.getPerson("Missing Person", 123456);

        // Make sure it's empty
        if (missingPerson.getKey() != "") {
            return false;
        }
        if (missingPerson.getID() != 0) {
            return false;
        }
        if (missingPerson.getUsed() != false) {
            return false;
        }
        return true;        
    }
    // testFindWithCollidingKeys: Test the find operation with several non-colliding keys. 
    bool testFindNoCollidingKeys() {
        Random rID(MINID, MAXID);
        Cache c(MINPRIME, hashCode, LINEAR);

        vector<Person> inserted;

        // Insert 50 non-colliding keys
        for (int i = 0; i < 50; i++) {
            string key = "key" + to_string(i);
            int ID = rID.getRandNum();
            Person p(key, ID, true);

            if (c.insert(p) == false) {
                return false;
            }
            // Push the insertion person to the vector
            inserted.push_back(p);

            // Make sure size matches
            if (c.m_currentSize != (i + 1)) {
                return false;
            }
        }

        // Test retrieval for each key
        for (unsigned int i = 0; i < inserted.size(); i++) {
            Person found = c.getPerson(inserted[i].getKey(), inserted[i].getID());
            // Check match
            if (found.getKey() != inserted[i].getKey()) {
                return false;
            }
            if (found.getID()  != inserted[i].getID()) {
                return false;
            }
        }
        return true;
    }
    // testFindColliding: Test the find operation with several colliding keys without triggering a rehash. This also tests whether the insertion works correctly with colliding data.
    bool testFindColliding() {
        Random rID(MINID, MAXID);
        Cache c(MINPRIME, hashCode, LINEAR);

        vector<Person> inserted;

        // Insert keys that collide
        for (int i = 0; i < 20; i++) {
            
            string key = "A" + to_string(i);
            int ID = rID.getRandNum();
            Person p(key, ID, true);
            // Insert
            if (c.insert(p) == false) {
                return false;
            }
            // Push back into the inserted vector
            inserted.push_back(p);

            // No rehash start
            if (c.m_oldTable != nullptr) {
                return false;
            }
        }

        // Test retrieval for all keys
        for (unsigned int i = 0; i < inserted.size(); i++) {
            Person found = c.getPerson(inserted[i].getKey(), inserted[i].getID());
            // Check match
            if (found.getKey() != inserted[i].getKey()) {
                return false;
            }
            if (found.getID()  != inserted[i].getID()) {
                return false;
            }
        }
        return true;
    }
    // testRemoveNoColliding: Test the remove operation with a few non-colliding keys.
    bool testRemoveNoColliding() {
        Random rID(MINID, MAXID);
        Cache c(MINPRIME, hashCode, LINEAR);
        // Data structure to hold check for deleted and remaining Persons later
        vector<Person> inserted;

        // Insert 50 keys that do not collide
        for (int i = 0; i < 50; i++) {
            string key = "key" + to_string(i);
            int ID = rID.getRandNum();
            Person p(key, ID, true);

            if (c.insert(p) == false) {
                return false;
            }

            inserted.push_back(p);
            // Check size
            if (c.m_currentSize != (i + 1)) {
                return false;
            }
        }

        // Remove the 10 first
        for (int i = 0; i < 10; i++) {
            if (c.remove(inserted[i]) == false) {
                return false;
            }

            // Make sure that they are deleted
            Person foundPerson = c.getPerson(inserted[i].getKey(), inserted[i].getID());

            if (foundPerson.getKey() != "" || foundPerson.getID() != 0) {
                return false;
            }
        }

        // Make sure that the remaining are still present
        for (int i = 10; i < 50; i++) {
            Person foundPerson = c.getPerson(inserted[i].getKey(), inserted[i].getID());

            if (foundPerson.getKey() != inserted[i].getKey()) {
                return false;
            }
            if (foundPerson.getID() != inserted[i].getID()) {
                return false;
            }
        }

        return true;
    }
    // testRemoveCollidingNoRehash: Test the remove operation with a number of colliding keys without triggering a rehash.
    bool testRemoveCollidingNoRehash() {
        Random rID(MINID, MAXID);
        Cache c(MINPRIME, hashCode, LINEAR);
        // Data structure to hold check for deleted and remaining Persons later
        vector<Person> inserted;

        // Insert 20 keys that collide
        for (int i = 0; i < 20; i++) {
            string key = "A" + to_string(i);
            int ID = rID.getRandNum();
            Person p(key, ID, true);
            // Insert person
            if (c.insert(p) == false) {
                return false;
            }
            // Append to the vector for later
            inserted.push_back(p);

            // Make sure rehash hasn't started
            if (c.m_oldTable != nullptr) {
                return false;
            }
        }

        // Remove Half
        for (int i = 0; i < 10; i++) {
            if (c.remove(inserted[i]) == false) {
                return false;
            }

            // The keys should be gone so check that
            Person foundPerson = c.getPerson(inserted[i].getKey(), inserted[i].getID());

            if (foundPerson.getKey() != "" || foundPerson.getID() != 0) {
                return false;
            }
            
        }

        // Make sure that the remaining are still present
        for (int i = 10; i < 20; i++) {
            Person found = c.getPerson(inserted[i].getKey(), inserted[i].getID());

            if (found.getKey() != inserted[i].getKey()) {
                return false;
            }
            if (found.getID() != inserted[i].getID()) {
                return false;
            }
        }

        return true;
    }
    // testRehashTriggerInsert: Test the rehashing is triggered after a descent number of data insertion.
    bool testRehashTriggerInsert() {
        Random rID(MINID, MAXID);
        Cache c(MINPRIME, hashCode, QUADRATIC);

        vector<Person> inserted;

        // Insert enough nodes to trigger a rehash
        for (int i = 0; i < 80; i++) {
            string key = "key" + to_string(i);
            int ID = rID.getRandNum();
            Person p(key, ID, true);
            // Insert
            if (c.insert(p) == false) {
                return false;
            }

            inserted.push_back(p);

            // Check if rehash triggered
            if (c.lambda() > 0.5 && c.m_oldTable == nullptr) {
                return false;
            }
            // hash triggered so return true
            if (c.m_oldTable != nullptr) {
                return true;
            }
        }
        return false;
    }
    // testRehashLoadFactor: Test the rehash completion after triggering rehash due to load factor, i.e. all live data is transferred to the new table and the old table is removed.
    bool testRehashLoadFactor() {
        Random rID(MINID, MAXID);
        Cache c(MINPRIME, hashCode, QUADRATIC);
        // bool to keep track of rehash
        bool rehashStarted = false;

        // Insert enough keys to trigger rehash
        for (int i = 0; i < 80; i++) {
            string key = "key" + to_string(i);
            int ID = rID.getRandNum();
            Person p(key, ID, true);
            // Insert
            if (c.insert(p) == false) {
                return false;
            }
            // Rehash has started when old table is no longer nullptr
            if (c.m_oldTable != nullptr) {
                rehashStarted = true;
            }
        }
        // If rehash never started, then fail
        if (rehashStarted == false) {
            return false;
        }

        // Keep inserting keys until rehash is completely done
        for (int i = 80; i < 180; i++) {
            string key = "more" + to_string(i);
            int ID = rID.getRandNum();
            Person p(key, ID, true);
            
            if (c.insert(p) == false) {
                return false;
            }
            // Rehash is fully completed when m_oldTable becomes nullptr again
            if (c.m_oldTable == nullptr) {
                return true;
            }
        }
        // If old table never cleared, fail
        return false;
    }

    // testRehashRemoval: Test the rehashing is triggered after a descent number of data removal.
    bool testRehashRemoval() {
        Random rID(MINID, MAXID);
        Cache c(MINPRIME, hashCode, QUADRATIC);

        vector<Person> inserted;

        // Insert 50 keys that do not collide
        for (int i = 0; i < 50; i++) {
            string key = "key" + to_string(i);
            int ID = rID.getRandNum();
            Person p(key, ID, true);

            // Insert
            if (c.insert(p) == false) {
                return false;
            }

            inserted.push_back(p);
        }
        // Delete to force the deleted ratio to exceed 0.8
        for (int i = 0; i < 45; i++) {
            // Remove till rehash is triggered
            if (c.remove(inserted[i]) == false) {
                return false;
            }

            // Rehash is triggered
            if (c.m_oldTable != nullptr) {
                return true;
            }
        }
        return false;
    }
    // testRehashCompletionRemoval: Test the rehash completion after triggering rehash due to delete ratio, i.e. all live data is transferred to the new table and the old table is removed.
    bool testRehashCompletionRemoval() {
        Random rID(MINID, MAXID);
        Cache c(MINPRIME, hashCode, QUADRATIC);

        vector<Person> inserted;

        // Insert 50 keys that do not collide
        for (int i = 0; i < 50; i++) {
            string key = "key" + to_string(i);
            int ID = rID.getRandNum();
            Person p(key, ID, true);

            if (c.insert(p) == false) {
                return false;
            }

            inserted.push_back(p);
        }
        bool rehashStarted = false;
        
        // Remove some to trigger rehash 
        for (int i = 0; i < 45; i++) {
            if (c.remove(inserted[i]) == false) {
                return false;
            }

            // rehash has started
            if (c.m_oldTable != nullptr) {
                rehashStarted = true;
            }
        }

        // if rehash didn't happen, return false
        if (rehashStarted == false) {
            return false;
        }

        // Keep doing operations until transfer completes
        for (int i = 0; i < 120; i++) {
            string key = "more" + to_string(i);
            int ID = rID.getRandNum();
            Person p(key, ID, true);
            
            if (c.insert(p) == false) {
                return false;
            }        
            // If old table becomes nullptr, rehash is fully completed
            if (c.m_oldTable == nullptr) {
                return true;
            }
        }
        return false;
    }

};

int main() {
    Tester t;
    // Run all tests
    cout << (t.testInsertNoCollide() == true ? "testInsertNoCollide PASSED" : "testInsertNoCollide FAILED") << endl;
    cout << (t.testInsertHalfCollide() == true ? "testInsertHalfCollide PASSED" : "testInsertHalfCollide FAILED") << endl;
    cout << (t.testFindMissingPerson() == true ? "testFindMissingPerson PASSED" : "testFindMissingPerson FAILED") << endl;
    cout << (t.testFindNoCollidingKeys() == true ? "testFindNoCollidingKeys PASSED" : "testFindNoCollidingKeys FAILED") << endl;
    cout << (t.testFindColliding() == true ? "testFindColliding PASSED" : "testFindColliding FAILED") << endl;
    cout << (t.testRemoveNoColliding() == true ? "testRemoveNoColliding PASSED" : "testRemoveNoColliding FAILED") << endl;
    cout << (t.testRemoveCollidingNoRehash() == true ? "testRemoveCollidingNoRehash PASSED" : "testRemoveCollidingNoRehash FAILED") << endl;
    cout << (t.testRehashTriggerInsert() == true ? "testRehashTriggerInsert PASSED" : "testRehashTriggerInsert FAILED") << endl;
    cout << (t.testRehashLoadFactor() == true ? "testRehashLoadFactor PASSED" : "testRehashLoadFactor FAILED") << endl;
    cout << (t.testRehashRemoval() == true ? "testRehashRemoval PASSED" : "testRehashRemoval FAILED") << endl;
    cout << (t.testRehashCompletionRemoval() == true ? "testRehashCompletionRemoval PASSED" : "testRehashCompletionRemoval FAILED") << endl;

    return 0;
}