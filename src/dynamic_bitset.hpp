// Implement dynamic_bitset per assignment, using 64-bit blocks
#include <vector>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <string>

struct dynamic_bitset {
    using block_t = unsigned long long; // 64-bit block
    static constexpr std::size_t BITS = 64;

    std::vector<block_t> blocks;
    std::size_t nbits = 0; // logical size

    dynamic_bitset() = default;
    ~dynamic_bitset() = default;
    dynamic_bitset(const dynamic_bitset &) = default;
    dynamic_bitset &operator=(const dynamic_bitset &) = default;

    dynamic_bitset(std::size_t n) : blocks((n + BITS - 1) / BITS, 0), nbits(n) {}

    dynamic_bitset(const std::string &str) {
        nbits = str.size();
        blocks.assign((nbits + BITS - 1) / BITS, 0);
        for (std::size_t i = 0; i < nbits; ++i) {
            char c = str[i];
            if (c == '1') set(i, true);
            else set(i, false);
        }
    }

    static std::size_t idx(std::size_t pos) { return pos / BITS; }
    static std::size_t off(std::size_t pos) { return pos % BITS; }
    static block_t mask1(std::size_t pos) { return block_t(1) << off(pos); }

    bool operator[](std::size_t n) const {
        if (n >= nbits) return false;
        return (blocks[idx(n)] >> off(n)) & 1ULL;
    }

    dynamic_bitset &set(std::size_t n, bool val = true) {
        if (n >= nbits) return *this; // ignore out of range
        block_t &b = blocks[idx(n)];
        block_t m = mask1(n);
        if (val) b |= m; else b &= ~m;
        return *this;
    }

    dynamic_bitset &push_back(bool val) {
        // Append high bit (most significant position)
        std::size_t newpos = nbits; // new index at end
        if ((nbits % BITS) == 0) blocks.push_back(0);
        nbits++;
        if (val) {
            block_t &b = blocks[idx(newpos)];
            b |= (block_t(1) << off(newpos));
        }
        return *this;
    }

    bool none() const {
        std::size_t full_blocks = nbits / BITS;
        for (std::size_t i = 0; i < full_blocks; ++i) if (blocks[i]) return false;
        if (nbits % BITS) {
            block_t last = blocks[full_blocks];
            block_t mask = (nbits % BITS == 64) ? ~block_t(0) : ((block_t(1) << (nbits % BITS)) - 1);
            if (last & mask) return false;
        }
        return true;
    }

    bool all() const {
        if (nbits == 0) return true; // vacuously all true?
        std::size_t full_blocks = nbits / BITS;
        for (std::size_t i = 0; i < full_blocks; ++i) if (blocks[i] != ~block_t(0)) return false;
        std::size_t rem = nbits % BITS;
        if (rem) {
            block_t mask = (block_t(1) << rem) - 1;
            if ((blocks[full_blocks] & mask) != mask) return false;
        }
        return true;
    }

    std::size_t size() const { return nbits; }

    // bitwise operations on overlapping part only
    dynamic_bitset &operator|=(const dynamic_bitset &other) {
        std::size_t common = std::min(nbits, other.nbits);
        std::size_t cb = (common + BITS - 1) / BITS;
        for (std::size_t i = 0; i < cb; ++i) {
            block_t ob = (i < other.blocks.size() ? other.blocks[i] : 0);
            if (i + 1 == cb && (common % BITS)) {
                block_t mask = (block_t(1) << (common % BITS)) - 1;
                blocks[i] = (blocks[i] & ~mask) | ((blocks[i] | (ob & mask)) & mask);
            } else {
                blocks[i] |= ob;
            }
        }
        return *this;
    }

    dynamic_bitset &operator&=(const dynamic_bitset &other) {
        std::size_t common = std::min(nbits, other.nbits);
        std::size_t cb = (common + BITS - 1) / BITS;
        for (std::size_t i = 0; i < cb; ++i) {
            block_t ob = (i < other.blocks.size() ? other.blocks[i] : 0);
            if (i + 1 == cb && (common % BITS)) {
                block_t mask = (block_t(1) << (common % BITS)) - 1;
                block_t keep = blocks[i] & ~mask; // higher bits unchanged
                blocks[i] = keep | ((blocks[i] & ob) & mask);
            } else {
                blocks[i] &= ob;
            }
        }
        return *this;
    }

    dynamic_bitset &operator^=(const dynamic_bitset &other) {
        std::size_t common = std::min(nbits, other.nbits);
        std::size_t cb = (common + BITS - 1) / BITS;
        for (std::size_t i = 0; i < cb; ++i) {
            block_t ob = (i < other.blocks.size() ? other.blocks[i] : 0);
            if (i + 1 == cb && (common % BITS)) {
                block_t mask = (block_t(1) << (common % BITS)) - 1;
                block_t keep = blocks[i] & ~mask;
                blocks[i] = keep | ((blocks[i] ^ ob) & mask);
            } else {
                blocks[i] ^= ob;
            }
        }
        return *this;
    }

    dynamic_bitset &operator<<=(std::size_t n) {
        if (n == 0 || nbits == 0) return *this;
        // Increase size by n, shifting existing bits up; low bits filled with 0
        std::size_t old_n = nbits;
        std::size_t old_blocks = blocks.size();
        std::size_t new_n = nbits + n;
        std::size_t new_blocks = (new_n + BITS - 1) / BITS;
        std::vector<block_t> nb(new_blocks, 0);

        std::size_t word_shift = n / BITS;
        std::size_t bit_shift = n % BITS;
        for (std::size_t i = 0; i < old_blocks; ++i) {
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
        return *this;
    }

    dynamic_bitset &operator>>=(std::size_t n) {
        if (n == 0) return *this;
        if (n >= nbits) { blocks.clear(); nbits = 0; return *this; }
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
        // adjust logical size
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
        std::size_t full_blocks = nbits / BITS;
        for (std::size_t i = 0; i < full_blocks; ++i) blocks[i] = ~block_t(0);
        std::size_t rem = nbits % BITS;
        if (rem) {
            blocks[full_blocks] = (block_t(1) << rem) - 1;
        }
        return *this;
    }

    dynamic_bitset &flip() {
        std::size_t full_blocks = nbits / BITS;
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

