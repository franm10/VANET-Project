#include "BloomFilter.h"
#include <cstddef>
#include <string>
#include <cmath>
#include <functional>
#include <algorithm>

BloomFilter::BloomFilter(size_t size, size_t numHashes)
    : m(size), k(numHashes), insertCount(0)
{
    bitArray.resize(m, false);
}

size_t BloomFilter::hash1(const std::string& key) const {
    return std::hash<std::string>{}(key) % m;
}

size_t BloomFilter::hash2(const std::string& key) const {
    const uint64_t FNV_PRIME = 1099511628211ULL;
    const uint64_t FNV_OFFSET = 14695981039346656037ULL;

    uint64_t hash = FNV_OFFSET;
    for (char c : key) {
        hash ^= static_cast<uint64_t>(c);
        hash *= FNV_PRIME;
    }
    return hash % m;
}

size_t BloomFilter::hashK(const std::string& key, size_t i) const {
    return (hash1(key) + i * hash2(key)) % m;
}

void BloomFilter::insert(const std::string& key) {
    for (size_t i = 0; i < k; ++i) {
        size_t pos = hashK(key, i);
        bitArray[pos] = true;
    }
    insertCount++;
}

bool BloomFilter::contains(const std::string& key) const {
    for (size_t i = 0; i < k; ++i) {
        size_t pos = hashK(key, i);
        if (!bitArray[pos]) return false;
    }
    return true;
}

void BloomFilter::clear() {
    std::fill(bitArray.begin(), bitArray.end(), false);
    insertCount = 0;
}

double BloomFilter::getEstimatedFPR() const {
    if (insertCount == 0) return 0.0;
    double exponent = -(double)(k * insertCount) / m;
    return std::pow(1.0 - std::exp(exponent), k);
}
