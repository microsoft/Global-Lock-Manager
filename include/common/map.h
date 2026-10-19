#pragma once

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace common {

/* A hash table implementation with a fixed number of buckets.
   TODOs:
   1. Use dinobase::common::List instead of the shared_mutex per bucket.
   2. methods that do not update Values should be set as const. */
template<typename Key, typename Value, typename Hash = std::hash<Key>>
class Map {
public:
    struct ValueWithLock {
        Value value;
        std::unique_lock<std::shared_mutex> lock;

        ValueWithLock(std::unique_lock<std::shared_mutex> lock) :
            lock{std::move(lock)} {}
    };

private:
    class Bucket {
    private:
        typedef std::pair<Key, Value> BucketValue;
        typedef std::list<BucketValue> BucketData;
        typedef typename BucketData::iterator bucketIterator;
        BucketData data;
        mutable std::shared_mutex mtx;

        bucketIterator findEntryFor(Key const& key) /*const*/ {
            return find_if(data.begin(), data.end(),
                [&](BucketValue const& item) { return item.first == key; });
        }

    public:
        Value valueFor(Key const& key, Value const& defaultValue) /*const*/ {
            std::shared_lock<std::shared_mutex> lock(mtx);
            auto /*const*/ entry = findEntryFor(key);
            return (entry == data.end()) ? defaultValue : entry->second;
        }

        ValueWithLock lockAndValueFor(Key const& key,
            Value const& defaultValue) /*const*/ {
            std::unique_lock<std::shared_mutex> lock(mtx);
            ValueWithLock valueWithLock(std::move(lock));
            auto /*const*/ entry = findEntryFor(key);
            valueWithLock.value = (entry == data.end()) ?
                defaultValue : entry->second;
            return valueWithLock;
        }

        Value valueForElseAddMapping(Key const& key, Value& defaultValue) {
            std::unique_lock<std::shared_mutex> lock(mtx);
            auto /*const*/ entry = findEntryFor(key);
            if (entry == data.end()) {
                data.push_back(BucketValue(key, defaultValue));
                return defaultValue;
            } else {
                return entry->second;
            }
        }

        void addOrUpdateMapping(Key const& key, Value const& value) {
            std::unique_lock<std::shared_mutex> lock(mtx);
            auto /*const*/ entry = findEntryFor(key);
            if (entry == data.end()) {
                data.push_back(BucketValue(key, value));
            } else {
                entry->second = value;
            }
        }

        void addOrUpdateMapping(Key const& key, Value const& value,
            std::unique_lock<std::shared_mutex> lock) {
            // lock owns the mutex of the bucket.
            auto /*const*/ entry = findEntryFor(key);
            if (entry == data.end()) {
                data.push_back(BucketValue(key, value));
            } else {
                entry->second = value;
            }
        }

        void removeMapping(Key const& key) {
            std::unique_lock<std::shared_mutex> lock(mtx);
            auto /*const*/ entry = findEntryFor(key);
            if (entry != data.end()) {
                data.erase(entry);
            }
        }
    };

public:
    Map(uint64_t numBuckets, Hash const& hash = Hash()) :
            buckets(numBuckets), hash(hash) {
        for(auto i = 0u; i < numBuckets; i++) {
            buckets[i].reset(new Bucket);
        }
    }

    Map(Map const& other) = delete;
    Map& operator=(Map const& other) = delete;

    Value get(Key const& key, Value const& defaultValue) /*const*/ {
        return getBucket(key).valueFor(key, defaultValue);
    }

    ValueWithLock getAndLock(Key const& key, Value const& defaultValue) /*const*/ {
        return getBucket(key).lockAndValueFor(key, defaultValue);
    }

    Value getElsePut(Key const& key, Value& value) {
        return getBucket(key).valueForElseAddMapping(key, value);
    }

    void put(Key const& key, Value const& value) {
        getBucket(key).addOrUpdateMapping(key, value);
    }

    void put(Key const& key, Value const& value,
        std::unique_lock<std::shared_mutex> lock) {
        getBucket(key).addOrUpdateMapping(key, value, std::move(lock));
    }

    void remove(Key const& key) {
        getBucket(key).removeMapping(key);
    }

private:
    Bucket& getBucket(Key const& key) const {
        // TODO: Change the % with an & bitMask (faster).
        return *buckets[/* bucketIdx */ hash(key) % buckets.size()];
    }

private:
    std::vector<std::unique_ptr<Bucket>> buckets;
    Hash hash;
};

} // namespace common
