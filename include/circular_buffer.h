#pragma once

#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <iterator>

template<typename Buffer>
class BufferIterator {
public:
    using value_type        = typename Buffer::value_type;
    using reference         = value_type&;
    using pointer           = value_type*;
    using size_type         = typename Buffer::size_type;
    using difference_type   = typename Buffer::difference_type;
    using iterator_category = std::random_access_iterator_tag;
public:
    BufferIterator(pointer ptr, pointer start, size_type size)
        : ptr_(ptr)
        , start_(start)
        , size_(size)
        , position_(ptr - start)
    {}
public:
    reference operator*() {
        return *ptr_;
    }

    pointer operator->() {
        return ptr_;
    }

    reference operator[](size_type index) {
        return *(ptr_ + index);
    }

    BufferIterator& operator+=(const size_type& n) {
        *this = *this + n;
        return *this;
    }

    BufferIterator& operator-=(const size_type& n) {
        *this = *this - n;
        return *this;
    }
public:
    BufferIterator& operator++() {
        ++ptr_;
        ++position_;

        size_type distance = ptr_ - start_;

        if (distance == size_ + 1) {
            ptr_ = start_;
            position_ = 0;
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
        --position_;

        int64_t distance = ptr_ - start_;

        if (distance == -1) {
            ptr_ = start_ + size_;
            position_ = size_;
        }

        return *this;
    }

    BufferIterator& operator--(int) {
        BufferIterator iterator = *this;
        --(*this);

        return iterator;
    }

    BufferIterator operator+(size_type n) {
        n %= size_;

        if (size_ - position_ >= n) {
            return BufferIterator(ptr_ + n, start_, size_);
        }

        n -= size_ - position_;

        return BufferIterator(start_ + n - 1, start_, size_);
    }

    BufferIterator operator-(size_type n) {
        n %= size_;

        return (*this + (size_ - n));
    }

    size_type operator-(const BufferIterator& rhs) {
        // ! changes needed
        return ptr_ - rhs.ptr_;
    }

    friend BufferIterator& operator+(size_type lhs, const BufferIterator& rhs) {
        return rhs + lhs;
    }
public:
    bool operator==(const BufferIterator& other) const {
        return ptr_ == other.ptr_;
    }

    bool operator!=(const BufferIterator& other) const {
        return !(*this == other);
    }

    bool operator>(const BufferIterator& other) const {
        // ! changes needed
        return ptr_ > other.ptr_;
    }

    bool operator<(const BufferIterator& other) const {
        // ! changes needed
        return ptr_ < other.ptr_;
    }

    bool operator>=(const BufferIterator& other) const {
        // ! changes needed
        return ptr_ >= other.ptr_;
    }

    bool operator<=(const BufferIterator& other) const {
        // ! changes needed
        return ptr_ <= other.ptr_;
    }
private:
    pointer ptr_;
    pointer start_;
    size_type size_;
    size_type position_;
};

template<
    typename T,
    typename Allocator = std::allocator<T>
>
class CircularBuffer {
public:
    using value_type       = typename Allocator::value_type;
    using reference        = value_type&;
    using const_reference  = const value_type&;
    using iterator         = BufferIterator<CircularBuffer<T>>;
    using const_iterator   = BufferIterator<CircularBuffer<const T>>;
    using difference_type  = typename Allocator::difference_type;
    using size_type        = typename Allocator::size_type;
public:
    CircularBuffer()
        : capacity_(0)
        , size_(0)
        , start_pos_(0)
        , end_pos_(0)
    {
        data_ = alloc_.allocate(1);
    }

    CircularBuffer(size_type capacity)
        : capacity_(capacity)
        , size_(0)
        , start_pos_(0)
        , end_pos_(0)
    {
        data_ = alloc_.allocate(capacity_ + 1);
    }

    CircularBuffer(size_type size, const_reference fill_with)
        : capacity_(size)
        , size_(size)
        , start_pos_(0)
        , end_pos_(size)
    {
        data_ = alloc_.allocate(capacity_ + 1);

        for (size_type i = 0; i < size; ++i) {
            data_[i] = fill_with;
        }
    }

    template<typename InputIterator>
    CircularBuffer(InputIterator first, InputIterator last) {
        InputIterator temp_first = first;
        size_ = 0;

        while (temp_first != last) {
            ++size_;
            ++temp_first;
        }

        capacity_ = size_;
        start_pos_ = 0;
        end_pos_ = size_;
        data_ = alloc_.allocate(capacity_ + 1);

        size_type current_index = start_pos_;

        while (first != last) {
            data_[current_index] = *first;
            ++first;
            ++current_index;
        }
    }

    CircularBuffer(const std::initializer_list<value_type>& init_list)
        : capacity_(init_list.end() - init_list.begin())
        , size_(init_list.end() - init_list.begin())
        , start_pos_(0)
        , end_pos_(init_list.end() - init_list.begin())
    {
        data_ = alloc_.allocate(capacity_ + 1);
        
        auto current = init_list.begin();

        for (size_type i = 0; i < size_; ++i) {
            data_[i] = *current;
            ++current;
        }
    }

    CircularBuffer(const CircularBuffer<value_type, Allocator>& other)
        : capacity_(other.capacity_)
        , size_(other.size_)
        , start_pos_(other.start_pos_)
        , end_pos_(other.end_pos_)
    {
        data_ = alloc_.allocate(capacity_ + 1);
        
        for (size_type i = 0; i < capacity_ + 1; ++i) {
            data_[i] = other.data_[i];
        }
    }

    CircularBuffer& operator=(const CircularBuffer<value_type, Allocator>& other) {
        if (this == &other) {
            return *this;
        }

        alloc_.deallocate(data_, capacity_ + 1);

        capacity_ = other.capacity_;
        size_ = other.size_;
        data_ = alloc_.allocate(capacity_ + 1);
        start_pos_ = other.start_pos_;
        end_pos_ = other.end_pos_;

        for (size_type i = 0; i < capacity_ + 1; ++i) {
            data_[i] = other.data_[i];
        }

        return *this;
    }

    CircularBuffer& operator=(const std::initializer_list<value_type>& other) {
        alloc_.deallocate(data_, capacity_ + 1);

        capacity_ = other.end() - other.begin();
        size_ = capacity_;
        data_ = alloc_.allocate(capacity_ + 1);
        start_pos_ = 0;
        end_pos_ = size_;

        auto current = other.begin();

        for (size_type i = 0; i < size_; ++i) {
            data_[i] = *current;
            ++current;
        }
    }

    ~CircularBuffer() {
        alloc_.deallocate(data_, capacity_ + 1);
    }
public:
    iterator begin() {
        // ! changes needed
        return iterator(data_ + start_pos_, data_, capacity_);
    }

    iterator end() {
        // ! changes needed
        return iterator(data_ + end_pos_, data_, capacity_);
    }

    const_iterator cbegin() const {
        // ! changes needed
        return const_iterator(data_ + start_pos_, data_, capacity_);
    }

    const_iterator cend() const {
        // ! changes needed
        return const_iterator(data_ + end_pos_, data_, capacity_);
    }

    bool operator==(const CircularBuffer& other) const {
        return size_ == other.size_ && std::equal(begin(), end(), other.begin());
    }

    bool operator!=(const CircularBuffer& other) const {
        return !(*this == other);
    }

    void swap(CircularBuffer& other) {
        std::swap(*this, other);
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

    void push_back(const_reference element) {
        if (size_ < capacity_) {
            data_[size_] = element;
            end_pos_ = GetNextPosition(end_pos_);
            ++size_;

            return;
        }

        std::swap(data_[start_pos_], data_[end_pos_]);

        data_[end_pos_] = element;
        start_pos_ = GetNextPosition(start_pos_);
        end_pos_ = GetNextPosition(end_pos_);
    }
private:
    size_type capacity_;
    size_type size_;
    value_type* data_;
    Allocator alloc_;
private:
    size_type start_pos_;
    size_type end_pos_;
private:
    size_type GetNextPosition(size_type pos) {
        return (pos + 1) % (capacity_ + 1);
    }
};
