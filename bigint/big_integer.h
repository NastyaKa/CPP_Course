#pragma once

#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

struct big_integer {
  big_integer();
  big_integer(big_integer const& other);
  big_integer(int a);
  big_integer(unsigned int a);
  big_integer(long a);
  big_integer(unsigned long a);
  big_integer(long long a);
  big_integer(unsigned long long a);
  explicit big_integer(std::string const& str);
  ~big_integer();

  big_integer& operator=(big_integer const& other);

  big_integer& operator+=(big_integer const& rhs);
  big_integer& operator-=(big_integer const& rhs);
  big_integer& operator*=(big_integer const& rhs);
  big_integer& operator/=(big_integer const& rhs);
  big_integer& operator%=(big_integer const& rhs);

  big_integer& operator&=(big_integer const& rhs);
  big_integer& operator|=(big_integer const& rhs);
  big_integer& operator^=(big_integer const& rhs);

  big_integer& operator<<=(int rhs);
  big_integer& operator>>=(int rhs);

  big_integer operator+() const;
  big_integer operator-() const;
  big_integer operator~() const;

  big_integer& operator++();
  big_integer operator++(int);

  big_integer& operator--();
  big_integer operator--(int);

  friend bool operator==(big_integer const& a, big_integer const& b);
  friend bool operator!=(big_integer const& a, big_integer const& b);
  friend bool operator<(big_integer const& a, big_integer const& b);
  friend bool operator>(big_integer const& a, big_integer const& b);
  friend bool operator<=(big_integer const& a, big_integer const& b);
  friend bool operator>=(big_integer const& a, big_integer const& b);

  friend std::string to_string(big_integer const& a);

  big_integer abs(big_integer const& a);
  void swap(big_integer& other);

private:
  std::vector<uint32_t> val;
  bool sign;

  size_t size() const {
    return val.size();
  }

  uint32_t& operator[](size_t i) {
    return val[i];
  }

  uint32_t const& operator[](size_t i) const {
    return val[i];
  }

  uint32_t get(size_t i, uint32_t def = 0) const {
    if (i < size()) {
      return val[i];
    }
    return def;
  }

  void sum_long_short(int64_t rhs);
  void sum_with_coef(int first, int second, big_integer const& rhs);
  static int64_t read_block(std::string const& str, size_t beg, size_t en);
  template<typename Func>
  void bit_op(big_integer const& a,
              const Func& function);
  big_integer mul_long_short(uint32_t c) const;
  void clean_up();
  big_integer two_compl(big_integer const& a);

  std::pair<big_integer, big_integer> div_mod(big_integer const& rhs);
  uint32_t trial(big_integer const& b);
  bool smaller(big_integer const& b, size_t m);
  void difference(big_integer const& b, size_t m);
  uint32_t div_long_short(uint32_t b, bool signd = false);
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);
