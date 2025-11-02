#include <functional>

#include "base.h"

SUDOKU_NAMESPACE::utils {
    template <class T>
    inline void combineHash(std::size_t& seed, const T& v) {
        // Taken from the old implementation used by Boost...
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}
