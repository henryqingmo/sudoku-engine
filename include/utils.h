#pragma once

#include <functional>
#include <list>
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
        explicit ArrayVector(std::size_t capacity)
            : m_capacity(capacity), m_size(0) {
            void* const raw = ::operator new[](
                capacity * sizeof(T), std::align_val_t(alignof(T))
            );
            this->m_buffer.reset(reinterpret_cast<std::byte*>(raw));
        }

        ArrayVector(const ArrayVector&) = delete;
        ArrayVector(ArrayVector&&) noexcept = default;

        ArrayVector& operator=(const ArrayVector&) = delete;
        ArrayVector& operator=(ArrayVector&&) noexcept = default;

        T* begin() {
            return this->buffer();
        }

        T* end() {
            return &this->buffer()[this->size()];
        }

        const T* begin() const {
            return this->buffer();
        }

        const T* end() const {
            return &this->buffer()[this->size()];
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

        template <typename... Args>
        void emplace(Args&&... args) {
            this->appendCheck();
            ::new (&this->buffer()[this->m_size++])
                T(std::forward<Args>(args)...);
        }

        void append(const T& element) {
            this->appendCheck();
            ::new (&this->buffer()[this->m_size++]) T(element);
        }

        void append(T&& element) {
            this->appendCheck();
            ::new (&this->buffer()[this->m_size++]) T(std::move(element));
        }

        ~ArrayVector() noexcept {
            if (this->m_buffer) {
                for (std::size_t i = 0; i < this->m_size; i++) {
                    std::destroy_at(&this->buffer()[i]);
                }
            }
        }

    private:
        void appendCheck() {
            if (this->m_size >= this->capacity()) {
                throw std::length_error("Array vector is already full");
            }
        }

        [[nodiscard]]
        const T* buffer() const {
            return std::launder(
                reinterpret_cast<const T*>(this->m_buffer.get())
            );
        }

        [[nodiscard]]
        T* buffer() {
            return std::launder(reinterpret_cast<T*>(this->m_buffer.get()));
        }
    };

    // A simple adapter for std::list...
    template <class Container, class Helper>
    class VectorAdapter {
    private:
        using T = Container::value_type;

        Container list;

    public:
        explicit VectorAdapter(std::size_t capacity) {
            Helper{}.reserve(list, capacity);
        }

        auto begin() {
            return this->list.begin();
        }

        auto end() {
            return this->list.end();
        }

        auto rbegin() const {
            return this->list.rbegin();
        }

        auto rend() const {
            return this->list.rend();
        }

        std::size_t size() const {
            return this->list.size();
        }

        void append(const T& element) {
            this->list.push_back(element);
        }

        void append(T&& element) {
            this->list.push_back(std::move(element));
        }

        void append(VectorAdapter&& tail) {
            Helper{}.append(this->list, std::move(tail.list));
        }
    };
}
