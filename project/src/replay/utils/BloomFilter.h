#ifndef __BLOOM_FILTER_H_
#define __BLOOM_FILTER_H_

#include <vector>
#include <string>
#include <cstddef>

class BloomFilter {
private:
    std::vector<bool> bitArray;
    size_t m;
    size_t k;
    size_t insertCount;

    size_t hash1(const std::string& key) const;
    size_t hash2(const std::string& key) const;
    size_t hashK(const std::string& key, size_t i) const;

public:
    BloomFilter(size_t size, size_t numHashes);
    void insert(const std::string& key);
    bool contains(const std::string& key) const;
    void clear();
    double getEstimatedFPR() const;
    size_t getInsertCount() const { return insertCount; }
};

#endif
