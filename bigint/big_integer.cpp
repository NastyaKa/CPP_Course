#include "big_integer.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <ostream>
#include <stdexcept>

static const uint32_t BASE_32 = 32;
static const uint64_t BASE = (1ull << BASE_32);

big_integer::big_integer() : sign(false) {}

big_integer::big_integer(big_integer const& other) = default;

big_integer::big_integer(long long a) : val(2, 0), sign(a < 0) {
  if (a < 0) {
    val[0] = (std::abs(a + 1) & 0xffffffff) + 1;
    val[1] = (((std::abs(a + 1) & 0xffffffff00000000) + 1) >> BASE_32) +
             ((std::abs(a + 1) & 0xffffffff80000000) ? 1 : 0);
  } else {
    val[0] = static_cast<int64_t>(a) & (BASE - 1);
    val[1] = static_cast<int64_t>(a) >> (BASE_32);
  }
  clean_up();
}

big_integer::big_integer(unsigned long long a) : val(2, 0), sign(false) {
  val[0] = static_cast<uint64_t>(a) & (BASE - 1);
  val[1] = static_cast<uint64_t>(a) >> (BASE_32);
  clean_up();
}

big_integer::big_integer(int a) : big_integer(static_cast<long long>(a)) {}
big_integer::big_integer(unsigned int a)
    : big_integer(static_cast<unsigned long long>(a)) {}
big_integer::big_integer(long a) : big_integer(static_cast<long long>(a)) {}
big_integer::big_integer(unsigned long a)
    : big_integer(static_cast<unsigned long long>(a)) {}

void big_integer::sum_long_short(int64_t rhs) {
  val.resize(size() + 1, 0);
  for (size_t i = 0; i < size(); i++) {
    rhs += static_cast<int64_t>(get(i)) + BASE;
    val[i] = rhs & (BASE - 1);
    rhs = (rhs >> BASE_32) - 1;
  }
  clean_up();
}

int64_t big_integer::read_block(const std::string& str, size_t beg,
                                 size_t en) {
  int64_t ans = 0;
  for (; beg < en; beg++) {
    if (!std::isdigit(str[beg])) {
      throw std::invalid_argument("number is incorrect");
    }
    ans = ans * 10 + (str[beg] - '0');
  }
  return ans;
}

big_integer::big_integer(std::string const& str) : big_integer() {
  if (str.size() == 0 || (str.size() == 1 && !std::isdigit(str[0]))) {
    throw std::invalid_argument("number is incorrect");
  }
  size_t iter = str[0] == '-';
  bool save_sign = iter;
  static const size_t block_size = 8;
  static const big_integer BLOCK = big_integer(100000000);
  static const big_integer TEN = big_integer(10);
  for (; iter + block_size < str.size(); iter += block_size) {
    *this *= BLOCK;
    sum_long_short(read_block(str, iter, iter + block_size));
  }
  if (iter < str.size()) {
    *this *= TEN;
    for (size_t j = 1; iter + j < str.size(); j++) {
      *this *= TEN;
    }
    *this += read_block(str, iter, str.size());
  }
  sign = save_sign;
  clean_up();
}

big_integer::~big_integer() = default;

big_integer& big_integer::operator=(big_integer const& other) {
  if (this == &other) {
    return *this;
  }
  big_integer(other).swap(*this);
  return *this;
}

big_integer big_integer::operator+() const {
  return *this;
}

big_integer big_integer::operator-() const {
  big_integer ans = *this;
  ans.sign ^= !ans.val.empty();
  return ans;
}

big_integer big_integer::operator~() const {
  static const big_integer ONE = 1;
  return -(*this + ONE);
}

big_integer& big_integer::operator++() {
  big_integer::sum_long_short(1);
  return *this;
}

big_integer big_integer::operator++(int) {
  big_integer res = *this;
  ++(*this);
  return res;
}

big_integer& big_integer::operator--() {
  big_integer::sum_long_short(sign ? 1 : -1);
  return *this;
}

big_integer big_integer::operator--(int) {
  big_integer res = *this;
  --(*this);
  return res;
}

void big_integer::sum_with_coef(int first, int second, big_integer const& rhs) {
  long long car = 0;
  val.resize(std::max(size(), rhs.size()) + 1, 0);
  for (size_t i = 0; i < size(); i++) {
    car += static_cast<uint64_t>(get(i)) * first + static_cast<uint64_t>(rhs.get(i)) * second + BASE;
    val[i] = car & (BASE - 1);
    car = (car >> BASE_32) - 1;
  }
  clean_up();
}

big_integer operator+(big_integer a, big_integer const& b) {
  return a += b;
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
  if (sign ^ rhs.sign) {
    if (abs(*this) > abs(rhs)) {
      sum_with_coef(1, -1, rhs);
    } else {
      sign = rhs.sign;
      sum_with_coef(-1, 1, rhs);
    }
  } else {
    sum_with_coef(1, 1, rhs);
  }
  return *this;
}

big_integer operator-(big_integer a, big_integer const& b) {
  return a -= b;
}

big_integer& big_integer::operator-=(big_integer const& rhs) {
  if (sign ^ !rhs.sign) {
    if (abs(*this) > abs(rhs)) {
      sum_with_coef(1, -1, rhs);
    } else {
      sign = !rhs.sign;
      sum_with_coef(-1, 1, rhs);
    }
  } else {
    sum_with_coef(1, 1, rhs);
  }
  return *this;
}

big_integer operator*(big_integer a, big_integer const& b) {
  return a *= b;
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
  big_integer result;
  result.val = std::vector<uint32_t>(size() + rhs.size() + 1, 0);
  result.sign = sign ^ rhs.sign;
  for (int i = 0; i < size(); i++) {
    uint64_t left = 0;
    for (int j = 0; j < rhs.size(); j++) {
      left += static_cast<uint64_t>(val[i]) * rhs[j] + result[i + j];
      result[i + j] = left & (BASE - 1);
      left >>= BASE_32;
    }
    result[i + rhs.size()] = left & (BASE - 1);
  }
  result.clean_up();
  swap(result);
  return *this;
}

big_integer operator&(big_integer a, big_integer const& b) {
  return a &= b;
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
  bit_op(rhs, [](uint32_t first, uint32_t second) { return first & second; });
  return *this;
}

big_integer operator|(big_integer a, big_integer const& b) {
  return a |= b;
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
  bit_op(rhs, [](uint32_t first, uint32_t second) { return first | second; });
  return *this;
}

big_integer operator^(big_integer a, big_integer const& b) {
  return a ^= b;
}

big_integer& big_integer::operator^=(big_integer const& rhs) {
  bit_op(rhs, [](uint32_t first, uint32_t second) { return first ^ second; });
  return *this;
}

big_integer big_integer::two_compl(const big_integer& a) {
  if (!a.sign) {
    return a;
  }
  big_integer result;
  result.val.resize(a.size(), 0);
  for (size_t i = 0; i < a.size(); i++) {
    result[i] = ~a[i];
  }
  result++;
  result.sign = true;
  return result;
}

template <typename Func>
void big_integer::bit_op(const big_integer& a, const Func& function) {
  uint32_t save_sign = sign;
  big_integer this_compl = two_compl(*this);
  big_integer a_compl = two_compl(a);

  size_t n = std::max(this_compl.size(), a_compl.size());
  val.resize(n);
  for (size_t i = 0; i < n; i++) {
    val[i] = function(this_compl.get(i, BASE - 1), a_compl.get(i, BASE - 1));
  }
  sign = function(save_sign, a.sign);
  *this = two_compl(*this);
  clean_up();
}

big_integer operator>>(big_integer a, int b) {
  return a >>= b;
}

big_integer& big_integer::operator>>=(int rhs) {
  return *this <<= -rhs;
}

big_integer operator<<(big_integer a, int b) {
  return a <<= b;
}

big_integer& big_integer::operator<<=(int rhs) {
  bool save_sign = sign;
  uint32_t shft = std::abs(rhs) % BASE_32, zer = std::abs(rhs) / BASE_32;
  if (rhs >= 0) {
    *this = mul_long_short(1ull << shft);
    val.insert(val.begin(), zer, 0);
  } else {
    val.erase(val.begin(), val.begin() + std::min(size(), static_cast<size_t>(zer)));
    div_long_short(1ull << shft, true);
  }
  sign = save_sign;
  clean_up();
  return *this;
}

bool operator==(big_integer const& a, big_integer const& b) {
  if ((a.sign ^ b.sign) || a.size() != b.size()) {
    return false;
  }
  return a.val == b.val;
}

bool operator>(big_integer const& a, big_integer const& b) {
  if (a.sign != b.sign) {
    return b.sign;
  } else if (a.size() != b.size()) {
    return a.sign ^ (a.size() > b.size());
  }
  for (size_t i = a.size(); i > 0; i--) {
    if (a[i - 1] != b[i - 1]) {
      return a.sign ^ (a[i - 1] > b[i - 1]);
    }
  }
  return false;
}

bool operator!=(big_integer const& a, big_integer const& b) {
  return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b) {
  return b > a;
}

bool operator<=(big_integer const& a, big_integer const& b) {
  return !(a > b);
}

bool operator>=(big_integer const& a, big_integer const& b) {
  return !(a < b);
}

std::string to_string(big_integer const& a) {
  std::string ans;
  big_integer copy = a;
  static const size_t block_size = 9;
  static const big_integer BLOCK(1000000000);
  do {
    uint32_t cur = (copy % BLOCK).get(0);
    size_t len = 0;
    for (; cur > 0; cur /= 10, len++) {
      ans.push_back('0' + cur % 10);
    }
    ans.resize(ans.size() + block_size - len, '0');
    copy /= BLOCK;
  } while (copy.size());
  while (ans.size() > 1 && ans[ans.size() - 1] == '0') {
    ans.pop_back();
  }
  if (a.sign) {
    ans.push_back('-');
  }
  std::reverse(ans.begin(), ans.end());
  return ans;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
  return s << to_string(a);
}

void big_integer::clean_up() {
  while (size() > 0 && val.back() == 0) {
    val.pop_back();
  }
  if (!size()) {
    sign = false;
  }
}

big_integer big_integer::abs(big_integer const& a) {
  return a.sign ? -a : a;
}

void big_integer::swap(big_integer& other) {
  std::swap(other.sign, sign);
  val.swap(other.val);
}

big_integer big_integer::mul_long_short(uint32_t c) const {
  uint64_t left = 0;
  big_integer result;
  result.val.resize(size() + 1, 0);
  for (size_t i = 0; i < result.size(); i++) {
    left += static_cast<uint64_t>(get(i)) * c;
    result[i] = left & (BASE - 1);
    left >>= BASE_32;
  }
  return result;
}

uint32_t big_integer::div_long_short(uint32_t b, bool signd) {
  uint64_t carry = 0;
  for (size_t i = size(); i != 0; i--) {
    carry <<= BASE_32;
    carry += val[i - 1];
    val[i - 1] = carry / b;
    carry %= b;
  }
  clean_up();
  if (signd && sign && carry) {
    *this -= 1;
  }
  return carry;
}

std::pair<big_integer, big_integer>
big_integer::div_mod(const big_integer& rhs) {
  bool ans_sign = sign ^ rhs.sign;
  bool this_sign = sign;
  sign = rhs.sign;
  if ((!rhs.sign && rhs > *this) || (rhs.sign && rhs < *this)) {
    sign = this_sign;
    clean_up();
    return {0, *this};
  } else if (rhs.size() == 1) {
    big_integer ost = div_long_short(rhs[0]);
    sign = ans_sign;
    ost.sign = this_sign;
    clean_up();
    ost.clean_up();
    return {*this, ost};
  }
  uint32_t normalized =
      static_cast<uint64_t>(BASE) / (static_cast<uint64_t>(rhs.val.back()) + 1);
  *this *= normalized;
  big_integer divisor = rhs * normalized;
  val.push_back(0);
  size_t m = divisor.size() + 1;
  big_integer ans, dq;
  ans.val.resize(size() - divisor.size());
  uint32_t qt = 0;
  for (size_t j = ans.size(); j != 0; j--) {
    qt = trial(divisor);
    dq = divisor * qt;
    while (smaller(dq, m)) {
      qt--;
      dq -= divisor;
    }
    ans[j - 1] = qt;
    difference(dq, m);
    val.pop_back();
  }
  ans.sign = ans_sign;
  ans.clean_up();
  sign = this_sign;
  div_long_short(normalized);
  clean_up();
  return {ans, *this};
}

uint32_t big_integer::trial(big_integer const& b) {
  uint64_t dividend = (static_cast<uint64_t>(val.back()) << BASE_32) |
                      (static_cast<uint64_t>(val[size() - 2]));
  uint64_t divider = b.get(b.size() - 1);

  return (dividend / divider) & (BASE - 1);
}

bool big_integer::smaller(big_integer const& b, size_t m) {
  for (size_t i = 0; i < size(); i++) {
    if (val[size() - i - 1] != b.get(m - i - 1)) {
      return val[size() - i - 1] < b.get(m - i - 1);
    }
  }
  return false;
}

void big_integer::difference(big_integer const& b, size_t m) {
  int64_t borrow = 0;
  uint64_t start = size() - m;
  for (size_t i = 0; i < m; i++) {
    borrow = static_cast<int64_t>(get(start + i)) - b.get(i) - borrow;
    val[start + i] = (borrow < 0 ? borrow + BASE : borrow);
    borrow = borrow < 0;
  }
}

big_integer operator/(big_integer a, big_integer const& b) {
  return a /= b;
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
  auto ans = div_mod(rhs);
  swap(ans.first);
  return *this;
}

big_integer operator%(big_integer a, big_integer const& b) {
  return a %= b;
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
  auto ans = div_mod(rhs);
  swap(ans.second);
  return *this;
}
