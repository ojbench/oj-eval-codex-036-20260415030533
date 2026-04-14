#pragma once
#include <vector>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <string>

namespace sjtu {

struct dynamic_bitset {
    using block_t = unsigned long long;
    static constexpr std::size_t BITS = 64;

    std::vector<block_t> blocks;
    std::size_t nbits = 0;

    dynamic_bitset() = default;
    ~dynamic_bitset() = default;
    dynamic_bitset(const dynamic_bitset &) = default;
    dynamic_bitset &operator=(const dynamic_bitset &) = default;

    dynamic_bitset(std::size_t n) : blocks(), nbits(n) {}

    dynamic_bitset(const std::string &str) {
        nbits = str.size();
        blocks.assign((nbits + BITS - 1) / BITS, 0);
        for (std::size_t i = 0; i < nbits; ++i) {
            if (str[i] == '1') set(i, true);
            else set(i, false);
        }
    }

    static std::size_t idx(std::size_t pos) { return pos / BITS; }
    static std::size_t off(std::size_t pos) { return pos % BITS; }

    void trim_to_size() {
        std::size_t need = (nbits + BITS - 1) / BITS;
        if (blocks.size() > need) blocks.resize(need);
        if (need && (nbits % BITS)) {
            block_t mask = (block_t(1) << (nbits % BITS)) - 1;
            blocks[need - 1] &= mask;
        }
    }

    bool operator[](std::size_t n) const {
        if (n >= nbits) return false;
        std::size_t bi = idx(n);
        if (bi >= blocks.size()) return false;
        return (blocks[bi] >> off(n)) & 1ULL;
    }

    dynamic_bitset &set(std::size_t n, bool val = true) {
        if (n >= nbits) return *this;
        std::size_t bi = idx(n);
        if (bi >= blocks.size()) {
            if (!val) return *this; // setting 0 on missing block is no-op
            blocks.resize(bi + 1, 0);
        }
        auto m = (block_t(1) << off(n));
        if (val) blocks[bi] |= m; else blocks[bi] &= ~m;
        return *this;
    }

    dynamic_bitset &push_back(bool val) {
        std::size_t newpos = nbits;
        if ((nbits % BITS) == 0) blocks.push_back(0);
        ++nbits;
        if (val) blocks[idx(newpos)] |= (block_t(1) << off(newpos));
        return *this;
    }

    bool none() const {
        std::size_t full_blocks = nbits / BITS;
        std::size_t exist = std::min(full_blocks, blocks.size());
        for (std::size_t i = 0; i < exist; ++i) if (blocks[i]) return false;
        std::size_t rem = nbits % BITS;
        if (rem && blocks.size() > full_blocks) {
            block_t mask = (block_t(1) << rem) - 1;
            if (blocks[full_blocks] & mask) return false;
        }
        return true;
    }

    bool all() const {
        if (nbits == 0) return true;
        std::size_t full_blocks = nbits / BITS;
        if (blocks.size() < full_blocks) return false;
        for (std::size_t i = 0; i < full_blocks; ++i) if (blocks[i] != ~block_t(0)) return false;
        std::size_t rem = nbits % BITS;
        if (rem) {
            if (blocks.size() <= full_blocks) return false;
            block_t mask = (block_t(1) << rem) - 1;
            if ((blocks[full_blocks] & mask) != mask) return false;
        }
        return true;
    }

    std::size_t size() const { return nbits; }

    dynamic_bitset &operator|=(const dynamic_bitset &other) {
        std::size_t common = std::min(nbits, other.nbits);
        std::size_t cb = (common + BITS - 1) / BITS;
        if (cb == 0) return *this;
        for (std::size_t i = 0; i < cb; ++i) {
            block_t ob = (i < other.blocks.size() ? other.blocks[i] : 0);
            if (i + 1 == cb && (common % BITS)) {
                block_t mask = (block_t(1) << (common % BITS)) - 1;
                block_t cur = (i < blocks.size() ? blocks[i] : 0);
                block_t res = (cur | (ob & mask)) & mask;
                if (res != 0 || i < blocks.size()) {
                    if (i >= blocks.size()) blocks.resize(i + 1, 0);
                    blocks[i] = (cur & ~mask) | res;
                }
            } else {
                if (ob != 0) {
                    if (i >= blocks.size()) blocks.resize(i + 1, 0);
                    blocks[i] |= ob;
                }
            }
        }
        return *this;
    }

    dynamic_bitset &operator&=(const dynamic_bitset &other) {
        std::size_t common = std::min(nbits, other.nbits);
        std::size_t cb = (common + BITS - 1) / BITS;
        if (blocks.empty()) return *this; // already zero
        for (std::size_t i = 0; i < std::min(cb, blocks.size()); ++i) {
            block_t ob = (i < other.blocks.size() ? other.blocks[i] : 0);
            if (i + 1 == cb && (common % BITS)) {
                block_t mask = (block_t(1) << (common % BITS)) - 1;
                block_t keep = blocks[i] & ~mask;
                blocks[i] = keep | ((blocks[i] & ob) & mask);
            } else {
                blocks[i] &= ob;
            }
        }
        // For i >= cb, overlapping part is 0; higher bits remain unchanged by rule
        return *this;
    }

    dynamic_bitset &operator^=(const dynamic_bitset &other) {
        std::size_t common = std::min(nbits, other.nbits);
        std::size_t cb = (common + BITS - 1) / BITS;
        if (cb == 0) return *this;
        for (std::size_t i = 0; i < cb; ++i) {
            block_t ob = (i < other.blocks.size() ? other.blocks[i] : 0);
            if (i + 1 == cb && (common % BITS)) {
                block_t mask = (block_t(1) << (common % BITS)) - 1;
                block_t cur = (i < blocks.size() ? blocks[i] : 0);
                block_t res = (cur ^ ob) & mask;
                if (res != 0 || i < blocks.size()) {
                    if (i >= blocks.size()) blocks.resize(i + 1, 0);
                    block_t keep = cur & ~mask;
                    blocks[i] = keep | res;
                }
            } else {
                if (ob != 0 || i < blocks.size()) {
                    if (i >= blocks.size()) blocks.resize(i + 1, 0);
                    blocks[i] ^= ob;
                }
            }
        }
        return *this;
    }

    dynamic_bitset &operator<<=(std::size_t n) {
        if (n == 0 || nbits == 0) return *this;
        // sanitize current last block so no garbage spills in
        trim_to_size();
        std::size_t new_n = nbits + n;
        std::size_t new_blocks = (new_n + BITS - 1) / BITS;
        std::vector<block_t> nb(new_blocks, 0);
        std::size_t word_shift = n / BITS;
        std::size_t bit_shift = n % BITS;
        for (std::size_t i = 0; i < blocks.size(); ++i) {
            std::size_t to = i + word_shift;
            block_t cur = blocks[i];
            if (bit_shift == 0) {
                if (to < new_blocks) nb[to] |= cur;
            } else {
                if (to < new_blocks) nb[to] |= (cur << bit_shift);
                if (to + 1 < new_blocks) nb[to + 1] |= (cur >> (BITS - bit_shift));
            }
        }
        blocks.swap(nb);
        nbits = new_n;
        // mask high unused bits in last block
        if (new_blocks && (new_n % BITS)) {
            block_t mask = (block_t(1) << (new_n % BITS)) - 1;
            blocks.back() &= mask;
        }
        return *this;
    }

    dynamic_bitset &operator>>=(std::size_t n) {
        if (n == 0) return *this;
        if (n >= nbits) { blocks.clear(); nbits = 0; return *this; }
        // sanitize current last block
        trim_to_size();
        std::size_t old_blocks = blocks.size();
        std::size_t word_shift = n / BITS;
        std::size_t bit_shift = n % BITS;
        std::vector<block_t> nb(old_blocks, 0);
        for (std::size_t i = word_shift; i < old_blocks; ++i) {
            block_t cur = blocks[i];
            std::size_t to = i - word_shift;
            if (bit_shift == 0) nb[to] |= cur;
            else {
                nb[to] |= (cur >> bit_shift);
                if (i + 1 < old_blocks) nb[to] |= (blocks[i + 1] << (BITS - bit_shift));
            }
        }
        nbits -= n;
        nb.resize((nbits + BITS - 1) / BITS);
        if (!nb.empty() && (nbits % BITS)) {
            block_t mask = (block_t(1) << (nbits % BITS)) - 1;
            nb.back() &= mask;
        }
        blocks.swap(nb);
        return *this;
    }

    dynamic_bitset &set() {
        if (nbits == 0) return *this;
        std::size_t full_blocks = nbits / BITS;
        std::size_t need = (nbits + BITS - 1) / BITS;
        if (blocks.size() < need) blocks.resize(need, 0);
        for (std::size_t i = 0; i < full_blocks; ++i) blocks[i] = ~block_t(0);
        std::size_t rem = nbits % BITS;
        if (rem) blocks[full_blocks] = (block_t(1) << rem) - 1;
        return *this;
    }

    dynamic_bitset &flip() {
        if (nbits == 0) return *this;
        std::size_t full_blocks = nbits / BITS;
        std::size_t need = (nbits + BITS - 1) / BITS;
        if (blocks.size() < need) blocks.resize(need, 0);
        for (std::size_t i = 0; i < full_blocks; ++i) blocks[i] = ~blocks[i];
        std::size_t rem = nbits % BITS;
        if (rem) {
            block_t mask = (block_t(1) << rem) - 1;
            blocks[full_blocks] = (~blocks[full_blocks]) & mask;
        }
        return *this;
    }

    dynamic_bitset &reset() {
        std::fill(blocks.begin(), blocks.end(), 0);
        return *this;
    }
};

} // namespace sjtu
