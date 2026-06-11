#include "cache.h"

#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

unsigned int hashCode(const string str) {
    unsigned int val = 0;
    const unsigned int thirtyThree = 33;
    for (int i = 0; i < static_cast<int>(str.length()); i++) {
        val = val * thirtyThree + str[i];
    }
    return val;
}

string makeCompositeKey(const CacheEntry& entry) {
    return entry.getKey() + "#" + to_string(entry.getID());
}

CacheEntry makeEntry(int index) {
    return CacheEntry("key_" + to_string(index), MINID + index, true);
}

bool expectFound(Cache& cache, const CacheEntry& expected) {
    CacheEntry actual = cache.getCacheEntry(expected.getKey(), expected.getID());
    return actual == expected && actual.getUsed();
}

bool expectMissing(Cache& cache, const CacheEntry& entry) {
    CacheEntry actual = cache.getCacheEntry(entry.getKey(), entry.getID());
    return actual.getKey().empty() && actual.getID() == 0 && !actual.getUsed();
}

bool verifyAgainstReference(Cache& cache, const unordered_map<string, CacheEntry>& reference) {
    for (const auto& item : reference) {
        if (!expectFound(cache, item.second)) {
            cerr << "Missing live entry from cache: " << item.first << endl;
            return false;
        }
    }
    return true;
}

bool runRandomizedWorkload(prob_t policy) {
    constexpr int kCandidateCount = 2000;
    constexpr int kOperations = 8000;

    Cache cache(MINPRIME, hashCode, policy);
    unordered_map<string, CacheEntry> reference;
    vector<CacheEntry> candidates;
    candidates.reserve(kCandidateCount);

    for (int i = 0; i < kCandidateCount; i++) {
        candidates.push_back(makeEntry(i));
    }

    mt19937 rng(2026 + static_cast<int>(policy));
    uniform_int_distribution<int> indexDist(0, kCandidateCount - 1);
    uniform_int_distribution<int> operationDist(0, 99);

    for (int step = 0; step < kOperations; step++) {
        int candidateIndex = indexDist(rng);
        CacheEntry entry = candidates[candidateIndex];
        string key = makeCompositeKey(entry);
        int operation = operationDist(rng);

        if (operation < 45) {
            bool expectedInsert = reference.find(key) == reference.end();
            bool actualInsert = cache.insert(entry);

            if (actualInsert != expectedInsert) {
                cerr << "Insert mismatch at step " << step << " for " << key << endl;
                return false;
            }

            if (actualInsert) {
                reference.emplace(key, entry);
            }
        } else if (operation < 75) {
            bool expectedRemove = reference.erase(key) == 1;
            bool actualRemove = cache.remove(entry);

            if (actualRemove != expectedRemove) {
                cerr << "Remove mismatch at step " << step << " for " << key << endl;
                return false;
            }
        } else if (operation < 90) {
            bool shouldExist = reference.find(key) != reference.end();
            bool actualFound = expectFound(cache, entry);

            if (actualFound != shouldExist) {
                cerr << "Find mismatch at step " << step << " for " << key << endl;
                return false;
            }
        } else {
            bool shouldExist = reference.find(key) != reference.end();
            int newID = MAXID - candidateIndex;
            bool actualUpdate = cache.updateID(entry, newID);

            if (actualUpdate != shouldExist) {
                cerr << "Update mismatch at step " << step << " for " << key << endl;
                return false;
            }

            if (actualUpdate) {
                CacheEntry updated(entry.getKey(), newID, true);
                reference.erase(key);
                reference.emplace(makeCompositeKey(updated), updated);
                candidates[candidateIndex] = updated;
            }
        }

        if (step % 500 == 0 && !verifyAgainstReference(cache, reference)) {
            cerr << "Reference verification failed at step " << step << endl;
            return false;
        }
    }

    if (!verifyAgainstReference(cache, reference)) {
        return false;
    }

    for (const CacheEntry& candidate : candidates) {
        if (reference.find(makeCompositeKey(candidate)) == reference.end() &&
            !expectMissing(cache, candidate)) {
            cerr << "Deleted/missing candidate was still visible: "
                 << makeCompositeKey(candidate) << endl;
            return false;
        }
    }

    return true;
}

int main() {
    struct TestCase {
        prob_t policy;
        string name;
    };

    vector<TestCase> testCases = {
        {LINEAR, "linear probing"},
        {QUADRATIC, "quadratic probing"},
        {DOUBLEHASH, "double hashing"},
    };

    for (const TestCase& testCase : testCases) {
        cout << "Running randomized reference test with " << testCase.name << "..." << endl;
        if (!runRandomizedWorkload(testCase.policy)) {
            cerr << "FAILED: " << testCase.name << endl;
            return 1;
        }
        cout << "PASSED: " << testCase.name << endl;
    }

    cout << "All randomized reference tests passed." << endl;
    return 0;
}
