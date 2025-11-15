#pragma once

#include <functional>
#include <memory>
#include <span>
#include <stdexcept>

#include "base.h"

SUDOKU_NAMESPACE::utils {
    template <class T>
    inline void combineHash(std::size_t& seed, const T& v) {
        // Taken from the old implementation used by Boost...
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <class T>
    class ArrayVector {
    private:
        std::unique_ptr<T[]> m_buffer;
        std::size_t m_capacity;
        std::size_t m_size;

    public:
        ArrayVector(std::size_t capacity) : m_capacity(capacity), m_size(0) {
            this->m_buffer = std::make_unique<T[]>(capacity);
        }

        std::span<T> data() {
            return std::span<T>(this->m_buffer.get(), this->m_size);
        }

        std::span<const T> data() const {
            return this->m_buffer;
        }

        std::size_t size() const {
            return this->m_size;
        }

        std::size_t capacity() const {
            return this->m_capacity;
        }

        void append(const T& element) {
            this->m_buffer[this->m_size++] = element;
        }

        void append(T&& element) {
            this->m_buffer[this->m_size++] = std::move(element);
        }

    private:
        void appendCheck() {
            if (this->m_size >= this->capacity) {
                throw std::length_error("Array vector is already full");
            }
        }
    };
}
