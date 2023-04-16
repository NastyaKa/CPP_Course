#pragma once
#include <cstddef>

template <typename T>
struct vector {
  using iterator = T*;
  using const_iterator = T const*;

  // O(N) nothrow
  vector() : data_(nullptr), size_(0), capacity_(0) {}

  // O(1) strong
  vector(vector const& other)
      : data_(nullptr), size_(other.size()), capacity_(other.size()) {
    if (other.size()) {
      data_ = copy_raw(other.data_, other.size_, other.size_);
    }
  }

  // O(N) strong
  vector& operator=(vector const& other) {
    if (this == &other) {
      return *this;
    }
    vector(other).swap(*this);
    return *this;
  }

  // O(N) nothrow
  ~vector() {
    clear();
    operator delete(data_);
  }

  // O(1) nothrow
  T& operator[](size_t i) {
    return data_[i];
  }

  // O(1) nothrow
  T const& operator[](size_t i) const {
    return data_[i];
  }

  // O(1) nothrow
  T* data() {
    return data_;
  }

  // O(1) nothrow
  T const* data() const {
    return data_;
  }

  // O(1) nothrow
  size_t size() const {
    return size_;
  }

  // O(1) nothrow
  T& front() {
    return *begin();
  }

  // O(1) nothrow
  T const& front() const {
    return *begin();
  }

  // O(1) nothrow
  T& back() {
    return *(end() - 1);
  }

  // O(1) nothrow
  T const& back() const {
    return *(end() - 1);
  };

  // O(1)* strong
  void push_back(T const& element) {
    if (size() == capacity()) {
      capacity_ = (size_ == 0 ? 1 : (size_ << 1));
      T* tmp = copy_raw(data_, size(), capacity());
      try {
        new (tmp + size_) T(element);
      } catch (...) {
        clean_up(tmp, size());
        throw;
      }
      clean_up(data_, size());
      data_ = tmp;
    } else {
      new (end()) T(element);
    }
    size_++;
  }

  // O(1) nothrow
  void pop_back() {
    back().~T();
    size_--;
  }

  // O(1) nothrow
  bool empty() const {
    return size() == 0;
  }

  // O(1) nothrow
  size_t capacity() const {
    return capacity_;
  }

  // O(N) strong
  void reserve(size_t new_cap) {
    if (capacity() < new_cap) {
      copy_and_recapas(new_cap);
    }
  }

  // O(N) strong
  void shrink_to_fit() {
    if (size() == 0) {
      operator delete(data_);
      data_ = nullptr;
      capacity_ = 0;
    } else if (size() != capacity()) {
      copy_and_recapas(size());
    }
  }

  // O(N) nothrow
  void clear() {
    while (!empty()) {
      pop_back();
    }
  }

  // O(1) nothrow
  void swap(vector& other) {
    std::swap(this->data_, other.data_);
    std::swap(this->size_, other.size_);
    std::swap(this->capacity_, other.capacity_);
  }

  // O(1) nothrow
  iterator begin() {
    return data_;
  }

  // O(1) nothrow
  iterator end() {
    return data_ + size_;
  }

  // O(1) nothrow
  const_iterator begin() const {
    return data_;
  }

  // O(1) nothrow
  const_iterator end() const {
    return data_ + size_;
  }

  // O(N) strong
  iterator insert(const_iterator pos, T const& element) {
    size_t insert_pos = pos - begin();
    push_back(element);
    for (size_t i = size() - 1; i != insert_pos; i--) {
      std::swap(data_[i], data_[i - 1]);
    }
    return &data_[insert_pos];
  };

  // O(N) nothrow(swap)
  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  };

  // O(N) nothrow(swap)
  iterator erase(const_iterator first, const_iterator last) {
    size_t beg = first - begin();
    size_t len = last - first;
    for (size_t i = beg; i + len < size(); i++) {
      std::swap(data_[i], data_[i + len]);
    }
    for (size_t i = 0; i < len; i++) {
      pop_back();
    }
    return &data_[beg];
  };

private:
  T* data_;
  size_t size_;
  size_t capacity_;

  static void clean_up(iterator from, size_t sz) {
    for (size_t i = sz; i > 0; i--) {
      from[i - 1].~T();
    }
    operator delete(from);
  }

  static iterator copy_raw(iterator from, size_t sz, size_t capacity) {
    T* tmp = static_cast<T*>(operator new(sizeof(T) * capacity));
    size_t new_sz = 0;
    try {
      for (; new_sz < sz; new_sz++) {
        new (tmp + new_sz) T(from[new_sz]);
      }
    } catch (...) {
      clean_up(tmp, new_sz);
      throw;
    }
    return tmp;
  }

  void copy_and_recapas(size_t cap) {
    T* tmp = copy_raw(data_, size(), cap);
    for (size_t i = size(); i > 0; i--) {
      data_[i - 1].~T();
    }
    std::swap(data_, tmp);
    operator delete(tmp);
    capacity_ = cap;
  }
};