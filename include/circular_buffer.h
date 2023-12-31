#pragma once

#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>

template<typename Buffer>
class BufferIterator {
public:
    using value_type        = typename Buffer::value_type;
    using reference         = value_type&;
    using size_type         = typename Buffer::size_type;
    using pointer           = value_type*;
    using difference_type   = typename Buffer::difference_type;
    using iterator_category = std::random_access_iterator_tag;
public:
    BufferIterator(pointer ptr, pointer data_base, pointer data_begin, size_type size)
        : ptr_(ptr)
        , data_base_(data_base)
        , data_begin_(data_begin)
        , size_(size)
        , position_(ptr - data_base)
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

        size_type distance = ptr_ - data_base_;

        if (distance == size_) {
            ptr_ = data_base_;
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

        int64_t distance = ptr_ - data_base_;

        if (distance == -1) {
            ptr_ = data_base_ + size_;
            position_ = size_;
        }

        return *this;
    }

    BufferIterator& operator--(int) {
        BufferIterator iterator = *this;
        --(*this);

        return iterator;
    }

    BufferIterator operator+(size_type n) const {
        n %= size_;

        if (size_ - position_ - 1 >= n) {
            return BufferIterator(ptr_ + n, data_base_, data_begin_, size_);
        }

        n -= size_ - position_;

        return BufferIterator(data_base_ + n, data_base_, data_begin_, size_);
    }

    BufferIterator operator-(size_type n) const {
        n %= size_;

        return (*this + (size_ - n));
    }

    int64_t operator-(const BufferIterator& rhs) const {
        int64_t distance = ptr_ - rhs.ptr_;

        if (
        (data_begin_ <= ptr_ && data_begin_ <= rhs.ptr_) ||
        (data_begin_ > ptr_ && data_begin_ > rhs.ptr_)
        ) {
            return distance;
        }

        int64_t abstract_distance = std::max(ptr_, rhs.ptr_) - std::min(ptr_, rhs.ptr_);
        int sign = (ptr_ < rhs.ptr_ ? 1 : -1);

        return sign * (size_ - abstract_distance);
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
        return (*this - other) > 0;
    }

    bool operator<(const BufferIterator& other) const {
        return (*this - other) < 0;
    }

    bool operator>=(const BufferIterator& other) const {
        return (*this - other) >= 0;
    }

    bool operator<=(const BufferIterator& other) const {
        return (*this - other) <= 0;
    }
private:
    pointer ptr_;
    pointer data_base_;
    pointer data_begin_;
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
    using const_iterator   = BufferIterator<const CircularBuffer<T>>;
    using difference_type  = typename Allocator::difference_type;
    using size_type        = typename Allocator::size_type;
public:
    CircularBuffer()
        : capacity_(0)
        , real_capacity_(1)
        , size_(0)
        , begin_pos_(0)
        , end_pos_(0)
    {
        data_ = alloc_.allocate(real_capacity_);
        alloc_.construct(data_, value_type{});
    }

    CircularBuffer(size_type size)
        : capacity_(size)
        , real_capacity_(size + 1)
        , size_(size)
        , begin_pos_(0)
        , end_pos_(size)
    {
        data_ = alloc_.allocate(real_capacity_);

        for (size_type i = 0; i < real_capacity_; ++i) {
            alloc_.construct(data_ + i, value_type{});
        }
    }

    CircularBuffer(size_type size, const_reference fill_with)
        : capacity_(size)
        , real_capacity_(size + 1)
        , size_(size)
        , begin_pos_(0)
        , end_pos_(size)
    {
        data_ = alloc_.allocate(real_capacity_);

        for (size_type i = 0; i < real_capacity_; ++i) {
            alloc_.construct(data_ + i, fill_with);
        }
    }

    template<
        typename InputIterator,
        typename = std::_RequireInputIter<InputIterator>
    >
    CircularBuffer(InputIterator first, InputIterator last) {
        InputIterator temp_first = first;
        size_ = 0;

        while (temp_first != last) {
            ++size_;
            ++temp_first;
        }

        capacity_ = size_;
        real_capacity_ = size_ + 1;
        begin_pos_ = 0;
        end_pos_ = size_;
        data_ = alloc_.allocate(real_capacity_);

        size_type current_index = 0;

        while (first != last) {
            alloc_.construct(data_ + current_index, *first);
            ++first;
            ++current_index;
        }

        for (; current_index < real_capacity_; ++current_index) {
            alloc_.construct(data_ + current_index, *(first - 1));
        }
    }

    CircularBuffer(const std::initializer_list<value_type>& init_list)
        : capacity_(init_list.end() - init_list.begin())
        , real_capacity_(init_list.end() - init_list.begin() + 1)
        , size_(init_list.end() - init_list.begin())
        , begin_pos_(0)
        , end_pos_(init_list.end() - init_list.begin())
    {
        data_ = alloc_.allocate(real_capacity_);
        
        auto current = init_list.begin();

        for (size_type i = 0; i < size_; ++i) {
            alloc_.construct(data_ + i, *current);
            ++current;
        }

        for (size_type i = size_; i < real_capacity_; ++i) {
            alloc_.construct(data_ + i, *(current - 1));
        }
    }

    CircularBuffer(const CircularBuffer<value_type, Allocator>& other)
        : capacity_(other.capacity_)
        , real_capacity_(other.real_capacity_)
        , size_(other.size_)
        , begin_pos_(other.begin_pos_)
        , end_pos_(other.end_pos_)
    {
        data_ = alloc_.allocate(real_capacity_);
        
        for (size_type i = 0; i < real_capacity_; ++i) {
            alloc_.construct(data_ + i, other.data_[i]);
        }
    }

    CircularBuffer& operator=(const CircularBuffer<value_type, Allocator>& other) {
        if (this == &other) {
            return *this;
        }

        for (size_type i = 0; i < real_capacity_; ++i) {
            alloc_.destroy(data_ + i);
        }

        alloc_.deallocate(data_, real_capacity_);

        capacity_ = other.capacity_;
        real_capacity_ = other.real_capacity_;
        size_ = other.size_;
        data_ = alloc_.allocate(real_capacity_);
        begin_pos_ = other.begin_pos_;
        end_pos_ = other.end_pos_;

        for (size_type i = 0; i < real_capacity_; ++i) {
            alloc_.construct(data_ + i, other.data_[i]);
        }

        return *this;
    }

    CircularBuffer& operator=(const std::initializer_list<value_type>& other) {
        alloc_.deallocate(data_, real_capacity_);

        capacity_ = other.end() - other.begin();
        real_capacity_ = capacity_ + 1;
        size_ = capacity_;
        data_ = alloc_.allocate(real_capacity_);
        begin_pos_ = 0;
        end_pos_ = size_;

        auto current = other.begin();

        for (size_type i = 0; i < size_; ++i) {
            alloc_.construct(data_ + i, *current);
            ++current;
        }

        for (size_type i = size_; i < real_capacity_; ++i) {
            alloc_.construct(data_ + i, *(current - 1));
        }

        return *this;
    }

    ~CircularBuffer() {
        for (size_t i = 0; i < real_capacity_; ++i) {
            alloc_.destroy(data_ + i);
        }

        alloc_.deallocate(data_, real_capacity_);
    }
public:
    iterator begin() {
        return iterator(
            data_ + begin_pos_,
            data_,
            data_ + begin_pos_,
            real_capacity_
        );
    }

    iterator end() {
        return iterator(
            data_ + end_pos_,
            data_,
            data_ + begin_pos_,
            real_capacity_
        );
    }

    const_iterator begin() const {
        return const_iterator(
            data_ + begin_pos_,
            data_,
            data_ + begin_pos_,
            real_capacity_
        );
    }

    const_iterator end() const {
        return const_iterator(
            data_ + end_pos_,
            data_,
            data_ + begin_pos_,
            real_capacity_
        );
    }

    const_iterator cbegin() const {
        return const_iterator(
            data_ + begin_pos_,
            data_,
            data_ + begin_pos_,
            real_capacity_
        );
    }

    const_iterator cend() const {
        return const_iterator(
            data_ + end_pos_,
            data_,
            data_ + begin_pos_,
            real_capacity_
        );
    }

    template<typename Container>
    bool operator==(const Container& other) const {
        return size_ == other.size_ && std::equal(begin(), end(), other.begin());
    }

    bool operator==(const std::initializer_list<value_type>& other) const {
        return size_ == other.size_ && std::equal(begin(), end(), other.begin());
    }

    template<typename Container>
    bool operator!=(const Container& other) const {
        return !(*this == other);
    }

    bool operator!=(const std::initializer_list<value_type>& other) const {
        return !(*this == other);
    }

    void swap(CircularBuffer& other) {
        std::swap(*this, other);
    }

    friend void swap(CircularBuffer& lhs, CircularBuffer& rhs) {
        lhs.swap(rhs);
    }

    size_type size() const {
        return size_;
    }

    size_type max_size() const {
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }

    size_type capacity() const {
        return capacity_;
    }

    bool empty() const {
        return size_ == 0;
    }

    reference front() {
        if (empty()) {
            throw std::runtime_error("Cannot access empty container.");
        }

        return *begin();
    }

    const_reference front() const {
        if (empty()) {
            throw std::runtime_error("Cannot access empty container.");
        }

        return *begin();
    }

    reference back() {
        if (empty()) {
            throw std::runtime_error("Cannot access empty container.");
        }

        auto it = end();
        
        return *(--it);
    }

    const_reference back() const {
        if (empty()) {
            throw std::runtime_error("Cannot access empty container.");
        }

        auto it = end();
        
        return *(--it);
    }
public:
    iterator insert(iterator p, const_reference t) {
        for (auto it = end() - 1; it != (p - 1); --it) {
            *(it + 1) = *it;
        }

        *p = t;
        
        if (size_ < capacity_) {
            ++size_;
            end_pos_ = GetNextPosition(end_pos_);
        }

        return p;
    }

    iterator insert(iterator p, size_type n, const_reference t) {
        for (auto it = end() - 1; it != (p + n - 2); --it) {
            *(it + 1) = *it;
        }

        for (size_type i = 0; i < n; ++i) {
            *(p + i) = t;
        }

        if (size_ + n <= capacity_) {
            size_ += n;
            end_pos_ = (end_pos_ + n) % real_capacity_;
        }

        return p;
    }
    
    template<
        typename InputIterator,
        typename = std::_RequireInputIter<InputIterator>
    >
    iterator insert(iterator p, InputIterator first, InputIterator last) {
        InputIterator temp_first = first;
        size_type n = 0;

        while (temp_first != last) {
            ++n;
            ++temp_first;
        }

        for (auto it = end() - 1; it != (p + n - 2); --it) {
            *(it + 1) = *it;
        }

        for (size_type i = 0; i < n; ++i) {
            *(p + i) = *first;
            ++first;
        }

        if (size_ + n <= capacity_) {
            size_ += n;
            end_pos_ = (end_pos_ + n) % real_capacity_;
        }

        return p;
    }

    iterator insert(iterator p, const std::initializer_list<value_type>& init_list) {
        return insert(p, init_list.begin(), init_list.end());
    }

    iterator erase(iterator q) {
        if (empty() || q >= end()) {
            throw std::runtime_error("Cannot erase non-existing element");
        }

        for (auto it = q; it != end(); ++it) {
            *it = *(it + 1);
        }

        --size_;
        end_pos_ = GetPrevPosition(end_pos_);

        return q;
    }

    iterator erase(iterator q1, iterator q2) {
        size_type removed = q2 - q1;

        if (empty() || size_ < removed || q1 >= end() || q2 >= end()) {
            throw std::runtime_error("Cannot erase non-existing element");
        }

        for (auto it = q1; it + removed != end(); ++it) {
            *it = *(it + removed);
        }

        size_ -= removed;
        end_pos_ = ((end_pos_ - removed) % real_capacity_ + real_capacity_) % real_capacity_;

        return q1;
    }

    void clear() {
        while (!empty()) {
            pop_front();
        }
    }

    void reserve(size_type n) {
        if (capacity_ >= n) {
            return;
        }

        value_type* ndata = alloc_.allocate(n + 1);

        for (size_type i = 0; i < real_capacity_; ++i) {
            alloc_.construct(ndata + i, data_[i]);
        }

        alloc_.deallocate(data_, real_capacity_);

        capacity_ = n;
        real_capacity_ = n + 1;
        data_ = ndata;
    }

    void resize(size_type n) {
        if (n <= size_) {
            auto to_move = size_ - n;
            
            size_ = n;
            end_pos_ = ((end_pos_ - to_move) % real_capacity_ + real_capacity_) % real_capacity_;

            return;
        }

        reserve(n);
        auto it = end();

        while (size_ < n) {
            *it = value_type{};
            ++it;
            ++size_;
            end_pos_ = GetNextPosition(end_pos_); 
        }
    }

    void assign(size_type n, const_reference t) {
        if (size_ <= n) {
            resize(n);
        } else {
            for (size_type i = 0; i < real_capacity_; ++i) {
                alloc_.destroy(data_ + i);
            }

            alloc_.deallocate(data_, real_capacity_);

            value_type* ndata = alloc_.allocate(n);

            begin_pos_ = 0;
            end_pos_ = n;
            size_ = n;
            capacity_ = n;
            real_capacity_ = n + 1;
            data_ = ndata;
        }

        for (size_type i = 0; i < real_capacity_; ++i) {
            alloc_.construct(data_ + i, t);
        }
    }

    template<
        typename InputIterator,
        typename = std::_RequireInputIter<InputIterator>
    >
    void assign(InputIterator first, InputIterator last) {
        InputIterator temp_first = first;
        size_type n = 0;

        while (temp_first != last) {
            ++n;
            ++temp_first;
        }

        if (size_ <= n) {
            reserve(n);
        } else {
            for (size_type i = 0; i < real_capacity_; ++i) {
                alloc_.destroy(data_ + i);
            }

            alloc_.deallocate(data_, real_capacity_);

            value_type* ndata = alloc_.allocate(n);

            begin_pos_ = 0;
            end_pos_ = n;
            size_ = n;
            capacity_ = n;
            real_capacity_ = n + 1;
            data_ = ndata;
        }

        for (size_type i = 0; i < real_capacity_; ++i) {
            alloc_.construct(data_ + i, *first);
            ++first;
        }
    }

    void assign(const std::initializer_list<value_type>& init_list) {
        assign(init_list.begin(), init_list.end());
    }
public:
    virtual void push_front(const_reference element) {
        begin_pos_ = GetPrevPosition(begin_pos_);

        if (size_ < capacity_) {
            data_[begin_pos_] = element;
            ++size_;

            return;
        }

        std::swap(data_[end_pos_], data_[GetPrevPosition(end_pos_)]);
        
        data_[begin_pos_] = element;
        end_pos_ = GetPrevPosition(end_pos_);
    }

    virtual void push_back(const_reference element) {
        if (size_ < capacity_) {
            data_[size_] = element;
            end_pos_ = GetNextPosition(end_pos_);
            ++size_;

            return;
        }

        std::swap(data_[begin_pos_], data_[end_pos_]);

        data_[end_pos_] = element;
        begin_pos_ = GetNextPosition(begin_pos_);
        end_pos_ = GetNextPosition(end_pos_);
    }

    void pop_front() {
        if (empty()) {
            throw std::runtime_error("Cannot delete the element from empty buffer.");
        }

        --size_;
        begin_pos_ = GetNextPosition(begin_pos_);
    }

    void pop_back() {
        if (empty()) {
            throw std::runtime_error("Cannot delete the element from empty buffer.");
        }

        --size_;
        end_pos_ = GetPrevPosition(end_pos_);
    }
public:
    reference operator[](size_type n) {
        return *(begin() + n);
    }

    const_reference operator[](size_type n) const {
        return *(begin() + n);
    }

    reference at(size_type n) {
        if (n >= size()) {
            throw std::out_of_range("The index of element exceeds the size of buffer.");
        }

        return *(begin() + n);
    }

    const_reference at(size_type n) const {
        if (n >= size()) {
            throw std::out_of_range("The index of element exceeds the size of buffer.");
        }

        return *(begin() + n);
    }
protected:
    size_type capacity_;
    size_type real_capacity_;
    size_type size_;
    value_type* data_;
    Allocator alloc_;
protected:
    size_type begin_pos_;
    size_type end_pos_;
protected:
    size_type GetPrevPosition(size_type pos) {
        return pos <= 0 ? pos + real_capacity_ - 1 : pos - 1;
    }

    size_type GetNextPosition(size_type pos) {
        return pos + 1 >= real_capacity_ ? pos + 1 - real_capacity_ : pos + 1;
    }
};

template<
    typename T,
    typename Allocator = std::allocator<T>
>
class CircularBufferExt : public CircularBuffer<T> {
public:
    using value_type       = typename Allocator::value_type;
    using reference        = value_type&;
    using const_reference  = const value_type&;
    using iterator         = BufferIterator<CircularBufferExt<T>>;
    using const_iterator   = BufferIterator<const CircularBufferExt<T>>;
    using difference_type  = typename Allocator::difference_type;
    using size_type        = typename Allocator::size_type;
public:
    CircularBufferExt()
        : CircularBuffer<T>()
    {}

    CircularBufferExt(size_type capacity)
        : CircularBuffer<T>(capacity)
    {}

    CircularBufferExt(size_type size, const_reference fill_with)
        : CircularBuffer<T>(size, fill_with)
    {}

    template<
        typename InputIterator,
        typename = std::_RequireInputIter<InputIterator>
    >
    CircularBufferExt(InputIterator first, InputIterator last) 
        : CircularBuffer<T>(first, last)
    {}

    CircularBufferExt(const std::initializer_list<value_type>& init_list)
        : CircularBuffer<T>(init_list)
    {}

    CircularBufferExt(const CircularBufferExt<value_type, Allocator>& other)
        : CircularBuffer<T>(other)
    {}

    CircularBufferExt& operator=(const CircularBufferExt<value_type, Allocator>& other) {
        if (this == &other) {
            return *this;
        }

        for (size_t i = 0; i < this->real_capacity_; ++i) {
            this->alloc_.destroy(this->data_ + i);
        }

        this->alloc_.deallocate(this->data_, this->real_capacity_);

        this->capacity_ = other.capacity_;
        this->real_capacity_ = other.real_capacity_;
        this->size_ = other.size_;
        this->data_ = this->alloc_.allocate(this->real_capacity_);
        this->begin_pos_ = other.begin_pos_;
        this->end_pos_ = other.end_pos_;

        for (size_type i = 0; i < this->real_capacity_; ++i) {
            this->data_[i] = other.data_[i];
        }

        return *this;
    }

    CircularBufferExt& operator=(const std::initializer_list<value_type>& other) {
        for (size_t i = 0; i < this->real_capacity_; ++i) {
            this->alloc_.destroy(this->data_ + i);
        }

        this->alloc_.deallocate(this->data_, this->real_capacity_);

        this->capacity_ = other.end() - other.begin();
        this->real_capacity_ = this->capacity_ + 1;
        this->size_ = this->capacity_;
        this->data_ = this->alloc_.allocate(this->real_capacity_);
        this->begin_pos_ = 0;
        this->end_pos_ = this->size_;

        auto current = other.begin();

        for (size_type i = 0; i < this->size_; ++i) {
            this->data_[i] = *current;
            ++current;
        }

        return *this;
    }
public:
    iterator begin() {
        return iterator(
            this->data_ + this->begin_pos_,
            this->data_,
            this->data_ + this->begin_pos_,
            this->real_capacity_
        );
    }

    iterator end() {
        return iterator(
            this->data_ + this->end_pos_,
            this->data_,
            this->data_ + this->begin_pos_,
            this->real_capacity_
        );
    }

    const_iterator begin() const {
        return const_iterator(
            this->data_ + this->begin_pos_,
            this->data_,
            this->data_ + this->begin_pos_,
            this->real_capacity_
        );
    }

    const_iterator end() const {
        return const_iterator(
            this->data_ + this->end_pos_,
            this->data_,
            this->data_ + this->begin_pos_,
            this->real_capacity_
        );
    }

    const_iterator cbegin() const {
        return const_iterator(
            this->data_ + this->begin_pos_,
            this->data_,
            this->data_ + this->begin_pos_,
            this->real_capacity_
        );
    }

    const_iterator cend() const {
        return const_iterator(
            this->data_ + this->end_pos_,
            this->data_,
            this->data_ + this->begin_pos_,
            this->real_capacity_
        );
    }
public:
    iterator insert(iterator p, const_reference t) {
        for (auto it = this->end() - 1; it != (p - 1); --it) {
            *(it + 1) = *it;
        }

        *p = t;
        
        if (this->size_ == this->capacity_) {
            this->reserve(this->capacity_ * 2);
        }

        ++this->size_;
        this->end_pos_ = this->GetNextPosition(this->end_pos_);

        return p;
    }

    iterator insert(iterator p, size_type n, const_reference t) {
        for (auto it = this->end() - 1; it != (p + n - 2); --it) {
            *(it + 1) = *it;
        }

        for (size_type i = 0; i < n; ++i) {
            *(p + i) = t;
        }

        while (this->size_ + n > this->capacity_) {
            this->reserve(this->capacity_ * 2);
        }

        this->size_ += n;
        this->end_pos_ = (this->end_pos_ + n) % this->real_capacity_;

        return p;
    }
    
    template<
        typename InputIterator,
        typename = std::_RequireInputIter<InputIterator>
    >
    iterator insert(iterator p, InputIterator first, InputIterator last) {
        InputIterator temp_first = first;
        size_type n = 0;

        while (temp_first != last) {
            ++n;
            ++temp_first;
        }

        for (auto it = this->end() - 1; it != (p + n - 2); --it) {
            *(it + 1) = *it;
        }

        for (size_type i = 0; i < n; ++i) {
            *(p + i) = *first;
            ++first;
        }

        while (this->size_ + n > this->capacity_) {
            this->reserve(this->capacity_ * 2);
        }

        this->size_ += n;
        this->end_pos_ = (this->end_pos_ + n) % this->real_capacity_;

        return p;
    }

    iterator erase(iterator q) {
        if (this->empty() || q >= this->end()) {
            throw std::runtime_error("Cannot erase non-existing element");
        }

        for (auto it = q; it != this->end(); ++it) {
            *it = *(it + 1);
        }

        --this->size_;
        this->end_pos_ = this->GetPrevPosition(this->end_pos_);

        return q;
    }

    iterator erase(iterator q1, iterator q2) {
        size_type removed = q2 - q1;

        if (this->empty() || this->size_ < removed || q1 >= end() || q2 >= end()) {
            throw std::runtime_error("Cannot erase non-existing element");
        }

        for (auto it = q1; it + removed != this->end(); ++it) {
            *it = *(it + removed);
        }

        this->size_ -= removed;
        this->end_pos_ = ((this->end_pos_ - removed) % this->real_capacity_ + this->real_capacity_) % this->real_capacity_;

        return q1;
    }

    void push_front(const_reference element) override {
        this->begin_pos_ = this->GetPrevPosition(this->begin_pos_);

        if (this->size_ == this->capacity_) {
            this->reserve(this->capacity_ * 2);
        }

        this->data_[this->begin_pos_] = element;
        ++this->size_;
    }

    void push_back(const_reference element) override {
        if (this->size_ == this->capacity_) {
            this->reserve(this->capacity_ * 2);
        }

        this->data_[this->size_] = element;
        this->end_pos_ = this->GetNextPosition(this->end_pos_);
        ++this->size_;
    }
};
