#include "cache.h"

#include <iostream>
#include <string>

using namespace std;

unsigned int hashCode(const string str) {
    unsigned int val = 0;
    const unsigned int thirtyThree = 33;
    for (int i = 0; i < static_cast<int>(str.length()); i++) {
        val = val * thirtyThree + str[i];
    }
    return val;
}

CacheEntry makeEntry(int index) {
    return CacheEntry("stats_key_" + to_string(index), MINID + index, true);
}

bool expect(bool condition, const string& message) {
    if (!condition) {
        cerr << "FAILED: " << message << endl;
        return false;
    }
    return true;
}

bool testInitialStats() {
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);
    CacheStats stats = cache.getStats();

    return expect(stats.currentCapacity == MINPRIME, "initial capacity should be MINPRIME") &&
           expect(stats.currentSize == 0, "initial current size should be zero") &&
           expect(stats.liveEntries == 0, "initial live entries should be zero") &&
           expect(!stats.rehashInProgress, "rehash should not be active initially") &&
           expect(stats.rehashProgress == 1.0F, "completed rehash progress should be 1.0");
}

bool testRehashStats() {
    Cache cache(MINPRIME, hashCode, DOUBLEHASH);

    for (int i = 0; i < 51; i++) {
        if (!cache.insert(makeEntry(i))) {
            return expect(false, "setup insert failed before rehash");
        }
    }

    CacheStats stats = cache.getStats();
    if (!expect(stats.rehashInProgress, "rehash should begin after load factor threshold")) {
        return false;
    }
    if (!expect(stats.oldCapacity == MINPRIME, "old capacity should preserve previous table size")) {
        return false;
    }
    if (!expect(stats.liveEntries == 51, "live entries should include old table during rehash")) {
        return false;
    }
    if (!expect(stats.rehashProgress == 0.0F, "rehash progress should start at zero")) {
        return false;
    }

    if (!cache.insert(makeEntry(51))) {
        return expect(false, "insert during rehash failed");
    }

    CacheStats progressed = cache.getStats();
    return expect(progressed.rehashInProgress, "rehash should still be in progress after one transfer") &&
           expect(progressed.rehashProgress > 0.0F, "rehash progress should increase after transfer") &&
           expect(progressed.liveEntries == 52, "live entries should include new insert during rehash");
}

bool testDeletedRatioStats() {
    Cache cache(MINPRIME, hashCode, LINEAR);

    for (int i = 0; i < 10; i++) {
        if (!cache.insert(makeEntry(i))) {
            return expect(false, "setup insert failed before removal");
        }
    }

    for (int i = 0; i < 3; i++) {
        if (!cache.remove(makeEntry(i))) {
            return expect(false, "setup remove failed");
        }
    }

    CacheStats stats = cache.getStats();
    return expect(stats.currentSize == 10, "current size should include tombstones") &&
           expect(stats.currentDeleted == 3, "deleted count should reflect tombstones") &&
           expect(stats.liveEntries == 7, "live entries should exclude tombstones") &&
           expect(stats.deletedRatio > 0.29F && stats.deletedRatio < 0.31F,
                  "deleted ratio should be approximately 0.3");
}

int main() {
    if (!testInitialStats()) {
        return 1;
    }
    cout << "testInitialStats PASSED" << endl;

    if (!testRehashStats()) {
        return 1;
    }
    cout << "testRehashStats PASSED" << endl;

    if (!testDeletedRatioStats()) {
        return 1;
    }
    cout << "testDeletedRatioStats PASSED" << endl;

    cout << "All cache stats tests passed." << endl;
    return 0;
}
