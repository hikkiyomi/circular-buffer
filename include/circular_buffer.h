#pragma once

#include <algorithm>
#include <cinttypes>
#include <iostream>

template<typename Buffer>
class BufferIterator {
public:
    using value_type = typename Buffer::value_type;
    using reference = value_type&;
    using pointer = value_type*;
public:
    BufferIterator(pointer ptr, pointer start, size_t capacity)
        : ptr_(ptr)
        , start_(start)
        , capacity_(capacity)
    {}

    BufferIterator& operator++() {
        ++ptr_;

        size_t distance = ptr_ - start_;

        if (distance == capacity_ + 1) {
            ptr_ = start_;
        }

        return *this;
    }

    BufferIterator& operator++(int) {
        BufferIterator iterator = *this;
        ++(*this);

        return iterator;
    }

    BufferIterator& operator--() {
        --ptr_;

        int64_t distance = ptr_ - start_;

        if (distance == -1) {
            ptr_ = start_ + capacity_;
        }

        return *this;
    }

    BufferIterator& operator--(int) {
        BufferIterator iterator = *this;
        --(*this);

        return iterator;
    }

    reference operator*() {
        return *ptr_;
    }

    pointer operator->() {
        return ptr_;
    }

    bool operator==(const BufferIterator& other) const {
        return ptr_ == other.ptr_;
    }

    bool operator!=(const BufferIterator& other) const {
        return !(*this == other);
    }

    BufferIterator& operator+(size_t index) const {

    }

    reference operator[](size_t index) {
        return *(ptr_ + index);
    }
private:
    pointer ptr_;
    pointer start_;
    size_t capacity_;
    size_t position_;
};

template<
    typename T,
    typename Allocator = std::allocator<T>
>
class CircularBuffer {
private:
    size_t capacity_;
    size_t size_;
    T* data_;
    Allocator alloc_;
public:
    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using iterator        = BufferIterator<CircularBuffer<T>>;
    using const_iterator  = BufferIterator<CircularBuffer<const T>>;
    using difference_type = int32_t;
    using size_type       = size_t;
public:
    CircularBuffer() = default;

    CircularBuffer(size_t capacity)
        : capacity_(capacity)
        , size_(capacity)
    {
        data_ = alloc_.allocate(capacity_ + 1);

        for (size_t i = 0; i < capacity_ + 1; ++i) {
            data_[i] = 5 - i;
        }
    }

    CircularBuffer(const CircularBuffer<T, Allocator>& other)
        : capacity_(other.capacity_)
        , size_(other.size_)
    {
        data_ = alloc_.allocate(capacity_ + 1);
        
        for (size_t i = 0; i < capacity_ + 1; ++i) {
            data_[i] = other.data_[i];
        }
    }

    CircularBuffer& operator=(const CircularBuffer<T, Allocator>& other) {
        if (this == &other) {
            return *this;
        }

        alloc_.deallocate(data_, capacity_ + 1);

        capacity_ = other.capacity_;
        size_ = other.size_;
        data_ = alloc_.allocate(capacity_ + 1);

        for (size_t i = 0; i < capacity_ + 1; ++i) {
            data_[i] = other.data_[i];
        }

        return *this;
    }

    ~CircularBuffer() {
        alloc_.deallocate(data_, capacity_ + 1);
    }
public:
    iterator begin() {
        return iterator(data_, data_, capacity_);
    }

    iterator end() {
        return iterator(data_ + size_, data_, capacity_);
    }

    const_iterator cbegin() const {
        return const_iterator(data_, data_, capacity_);
    }

    const_iterator cend() const {
        return const_iterator(data_ + size_, data_, capacity_);
    }

    bool operator==(const CircularBuffer& other) const {
        return size_ == other.size_ && std::equal(this->begin(), this->end(), other.begin());
    }

    bool operator!=(const CircularBuffer& other) const {
        return !(*this == other);
    }

    void swap(CircularBuffer& other) {
        swap(*this, other);
    }

    size_type size() const {
        return size_;
    }

    size_type max_size() const {
        return capacity_;
    }

    size_type capacity() const {
        return capacity_;
    }

    bool empty() const {
        return size_ == 0;
    }
};
