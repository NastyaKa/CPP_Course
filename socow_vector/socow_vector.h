#pragma once
#include <array>
#include <cstddef>

template <typename T, size_t SMALL_SIZE>
struct socow_vector {
  using iterator = T*;
  using const_iterator = T const*;

  socow_vector() : size_(0), is_dynamic(false) {}

  socow_vector(socow_vector const& other)
      : size_(other.size_), is_dynamic(other.is_dynamic) {
    if (other.is_dynamic) {
      d_data_ = other.d_data_;
      d_data_->count_++;
    } else {
      copy_elements(get_data(), other.data(), size_);
    }
  }

  socow_vector& operator=(socow_vector const& other) {
    if (this != &other) {
      socow_vector(other).swap(*this);
    }
    return *this;
  }

  ~socow_vector() {
    clean_with_size_save();
    size_ = 0;
  }

  T& operator[](size_t i) {
    return *(begin() + i);
  }
  T const& operator[](size_t i) const {
    return *(begin() + i);
  }

  T* data() {
    return begin();
  }
  T const* data() const {
    return begin();
  }
  size_t size() const {
    return size_;
  }

  T& front() {
    return *begin();
  }
  T const& front() const {
    return *begin();
  }

  T& back() {
    return *(end() - 1);
  }
  T const& back() const {
    return *(end() - 1);
  }

  void pop_back() {
    back().~T();
    size_--;
  }

  bool empty() const {
    return size_ == 0;
  }

  size_t capacity() const {
    return is_dynamic ? d_data_->capacity_ : SMALL_SIZE;
  }

  void clear() {
    if (is_dynamic && d_data_->count_ > 1) {
      d_data_->count_--;
      d_data_ = allocate_buffer(capacity());
    } else {
      clean_up(get_data(), size_, 0);
    }
    size_ = 0;
  }

  T* begin() {
    unshare();
    return get_data();
  }
  T* end() {
    return begin() + size_;
  }

  T const* begin() const {
    return is_dynamic ? d_data_->data_ : data_.data();
  }
  T const* end() const {
    return begin() + size_;
  }

  iterator insert(const_iterator pos, T const& element) {
    size_t insert_pos = pos - get_data();
    push_back(element);
    for (size_t i = size() - 1; i != insert_pos; i--) {
      std::swap((*this)[i], (*this)[i - 1]);
    }
    return begin() + insert_pos;
  }

  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  iterator erase(const_iterator first, T const* last) {
    size_t beg = first - get_data();
    size_t len = last - first;
    for (size_t i = beg; i + len < size(); i++) {
      std::swap((*this)[i], (*this)[i + len]);
    }
    clean_up(get_data(), size_, size_ - len);
    size_ -= len;
    return begin() + beg;
  }

  void push_back(T const& element) {
    if (size() == capacity()) {
      dynamic_data* tmp = copy_wider(capacity() * 2);
      try {
        new (tmp->data_ + size_) T(element);
      } catch (...) {
        clean_up(tmp->data_, size_, 0);
        operator delete(tmp);
        throw;
      }
      clean_with_size_save();
      is_dynamic = true;
      d_data_ = tmp;
    } else {
      unshare();
      new (end()) T(element);
    }
    size_++;
  }

  void swap(socow_vector& other) {
    if (is_dynamic && other.is_dynamic) {
      std::swap(d_data_, other.d_data_);
    } else if (!is_dynamic && !other.is_dynamic) {
      if (size_ < other.size_) {
        small_swap(*this, other);
      } else {
        small_swap(other, *this);
      }
    } else if (is_dynamic) {
      big_small_swap(*this, other);
    } else {
      big_small_swap(other, *this);
    }
    std::swap(size_, other.size_);
    std::swap(is_dynamic, other.is_dynamic);
  }

  void reserve(size_t new_cap) {
    if (new_cap > capacity()) {
      copy_and_recapas(new_cap);
    }
    unshare();
  }

  void shrink_to_fit() {
    if (is_dynamic) {
      if (size() <= SMALL_SIZE) {
        become_small();
      } else if (size() < capacity()) {
        copy_and_recapas(size());
      }
    }
  }

private:
  size_t size_;
  bool is_dynamic;

  struct dynamic_data {
    size_t count_;
    size_t capacity_;
    T data_[0];

    explicit dynamic_data(size_t capacity_) : count_(1), capacity_(capacity_) {}
  };

  union {
    std::array<T, SMALL_SIZE> data_;
    dynamic_data* d_data_;
  };

  void copy_elements(T* to, T const* from, size_t sz, size_t beg = 0) {
    size_t new_sz = beg;
    try {
      for (; new_sz < sz; new_sz++) {
        new (to + new_sz) T(*(from + new_sz));
      }
    } catch (...) {
      clean_up(to, new_sz, beg);
      throw;
    }
  }

  static dynamic_data* allocate_buffer(size_t capacity) {
    dynamic_data* new_data = new (static_cast<dynamic_data*>(operator new(
        sizeof(dynamic_data) + capacity * sizeof(T)))) dynamic_data(capacity);
    return new_data;
  }

  void unshare() {
    if (is_dynamic && d_data_->count_ > 1) {
      copy_and_recapas(capacity());
    }
  }

  void clean_with_size_save() {
    size_t prev_sz = size_;
    if (!is_dynamic || --d_data_->count_ == 0) {
      clean_up(get_data(), size_, 0);
      if (is_dynamic) {
        d_data_->capacity_ = 0;
        operator delete(d_data_);
      }
    }
    size_ = prev_sz;
  }

  void clean_up(T* from, size_t sz, size_t last) {
    for (; sz > last; sz--) {
      (*(from + sz - 1)).~T();
    }
  }

  dynamic_data* copy_wider(size_t cap) {
    dynamic_data* new_data = allocate_buffer(cap);
    try {
      copy_elements(new_data->data_, get_data(), size());
    } catch (...) {
      operator delete(new_data);
      throw;
    }
    return new_data;
  }

  T* get_data() {
    return (is_dynamic ? d_data_->data_ : data_.data());
  }

  void copy_and_recapas(size_t cap) {
    dynamic_data* tmp = copy_wider(cap);
    clean_with_size_save();
    d_data_ = tmp;
    is_dynamic = true;
  }

  void become_small() {
    assert(is_dynamic);
    dynamic_data* tmp = d_data_;
    d_data_ = nullptr;
    try {
      copy_elements(data_.data(), tmp->data_, size_);
    } catch (...) {
      d_data_ = tmp;
      throw;
    }
    if (--tmp->count_ == 0) {
      clean_up(tmp->data_, size_, 0);
      tmp->capacity_ = 0;
      operator delete(tmp);
    }
    is_dynamic = false;
  }

  void big_small_swap(socow_vector& big, socow_vector& small) {
    dynamic_data* tmp = big.d_data_;
    big.d_data_ = nullptr;
    try {
      copy_elements(big.data_.data(), small.get_data(), small.size());
    } catch (...) {
      big.d_data_ = tmp;
      throw;
    }
    small.clean_with_size_save();
    small.d_data_ = tmp;
  }

  void small_swap(socow_vector& smaller, socow_vector& bigger) {
    copy_elements(smaller.data_.data(), bigger.data_.data(), bigger.size_,
                  smaller.size_);
    clean_up(bigger.get_data(), bigger.size_, smaller.size_);
    for (size_t i = 0; i < smaller.size_; i++) {
      std::swap(smaller[i], bigger[i]);
    }
  }
};
