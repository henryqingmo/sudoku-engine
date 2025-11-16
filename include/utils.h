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
        struct Deleter {
            void operator()(std::byte* p) const noexcept {
                ::operator delete[](p, std::align_val_t(alignof(T)));
            }
        };

        std::unique_ptr<std::byte[], Deleter> m_buffer;
        std::size_t m_capacity;
        std::size_t m_size;

    public:
        ArrayVector() : ArrayVector(0) {}
        explicit ArrayVector(std::size_t capacity)
            : m_capacity(capacity), m_size(0) {
            if (m_capacity > 0) {
                void* const raw = ::operator new[](
                    capacity * sizeof(T), std::align_val_t(alignof(T))
                );
                this->m_buffer.reset(reinterpret_cast<std::byte*>(raw));
            }
        }

        ArrayVector(const ArrayVector&) = delete;
        ArrayVector(ArrayVector&& other) noexcept
            : m_buffer(std::move(other.m_buffer)), m_capacity(other.m_capacity),
              m_size(other.m_size) {
            other.m_capacity = 0;
            other.m_size = 0;
        }

        ArrayVector& operator=(const ArrayVector&) = delete;
        ArrayVector& operator=(ArrayVector&& other) noexcept {
            if (this != &other) {
                this->clear();

                this->m_buffer = std::move(other.m_buffer);
                this->m_capacity = other.m_capacity;
                this->m_size = other.m_size;

                other.m_capacity = 0;
                other.m_size = 0;
            }

            return *this;
        }

        std::span<T> data() {
            return std::span<T>(this->buffer(), this->m_size);
        }

        std::span<const T> data() const {
            return std::span<const T>(this->buffer(), this->m_size);
        }

        std::size_t size() const {
            return this->m_size;
        }

        std::size_t capacity() const {
            return this->m_capacity;
        }

        void append(const T& element) {
            this->appendCheck();
            ::new (&this->buffer()[this->m_size++]) T(element);
        }

        void append(T&& element) {
            this->appendCheck();
            ::new (&this->buffer()[this->m_size++]) T(std::move(element));
        }

        void clear() noexcept {
            for (std::size_t i = 0; i < this->m_size; i++) {
                std::destroy_at(&this->buffer()[i]);
            }

            this->m_size = 0;
        }

        ~ArrayVector() noexcept {
            this->clear();
        }

    private:
        void appendCheck() {
            if (this->m_size >= this->m_capacity) {
                throw std::length_error("Array vector is already full");
            }
        }

        [[nodiscard]]
        const T* buffer() const noexcept {
            return std::launder(
                reinterpret_cast<const T*>(this->m_buffer.get())
            );
        }

        [[nodiscard]]
        T* buffer() noexcept {
            return std::launder(reinterpret_cast<T*>(this->m_buffer.get()));
        }
    };
}
