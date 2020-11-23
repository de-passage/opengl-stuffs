#ifndef GUARD_DPSG_C_STR_WRAPPER_HEADER
#define GUARD_DPSG_C_STR_WRAPPER_HEADER

#include <cstdlib>
#include <iterator>
#include <string>

namespace dpsg {

struct reserve_t {
  std::size_t value;
};

template <class CharT> struct copy { const CharT *value; };

template <class CharT> copy(const CharT *) -> copy<CharT>;

template <class CharT> struct take_ownership { const CharT *value; };
template <class CharT> take_ownership(const CharT *) -> take_ownership<CharT>;

template <class CharT, class CharTraitT = std::char_traits<CharT>>
class basic_c_str_wrapper {
public:
  using char_traits = CharTraitT;
  using value_type = CharT;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;

  constexpr basic_c_str_wrapper() noexcept = default;

  basic_c_str_wrapper([[maybe_unused]] reserve_t reserve,
                      std::size_t size) noexcept
      : _value{_alloc(size)} {
    if (_value != nullptr) {
      _value[0] = '\0';
    }
  }

  basic_c_str_wrapper([[maybe_unused]] copy<value_type> copy,
                      const_pointer str) noexcept {}

  basic_c_str_wrapper(
      [[maybe_unused]] take_ownership<value_type> take_ownership,
      const_pointer str) noexcept
      : _value{str.value} {}

  basic_c_str_wrapper(const basic_c_str_wrapper &w) noexcept
      : _value(w.clone()) {}

  basic_c_str_wrapper &operator=(const basic_c_str_wrapper &rhs) noexcept {
    if (this != &w) {
      using ::std::swap;

      pointer tmp = w.clone();
      swap(tmp, _value);
      ::std::free(tmp);
    }
    return *this;
  }

  basic_c_str_wrapper &operator=(basic_c_str_wrapper &&w) noexcept {
    using ::std::swap;
    swap(w._value, value);
    ::std::free(w.release());
    return *this;
  }

  basic_c_str_wrapper(basic_c_str_wrapper &&w) noexcept
      : _value(std::exchange(w._value, nullptr)) {}

  [[nodiscard]] std::size_t strlen() const noexcept {
    return _value ? char_traits::length(_value) : 0;
  }

  ~basic_c_str_wrapper() noexcept { ::std::free(_value); }

  // NOLINTNEXTLINE
  constexpr operator bool() const noexcept { return _value != nullptr; }
  constexpr explicit operator const_pointer() const noexcept { return _value; }
  constexpr explicit operator pointer() noexcept { return _value; }

  // NOLINTNEXTLINE
  constexpr inline const_pointer c_str() const noexcept { return _value; }
  constexpr inline pointer c_str() noexcept { return _value; }

  // NOLINTNEXTLINE
  constexpr inline char operator[](int i) const { return _value[i]; }
  // NOLINTNEXTLINE
  constexpr inline char &operator[](int i) { return _value[i]; }

  pointer release() { return std::exchange(_value, nullptr); }

  pointer clone() const {
    auto len = strlen();
    if (len == 0) {
      return nullptr;
    }

    pointer ret = _alloc(len + 1);
    if (ret == nullptr) {
      return nullptr;
    }
    return char_traits::copy(ret, _value, len + 1);
  }

  void swap(basic_c_str_wrapper &w) noexcept {
    using ::std::swap;
    swap(w._value, _value);
  }

  friend void swap(basic_c_str_wrapper &lhs,
                   basic_c_str_wrapper &rhs) noexcept {
    lhs.swap(rhs);
  }

  bool realloc(std::size_t s) {
    if (s == 0) {
      ::std::free(_value);
      return true;
    }

    if ((_value = static_cast<pointer>(::std::realloc(_value, s)))) {
      _value[0] = '\0';
      return true;
    }

    return false;
  }

private:
  pointer _value = nullptr;

  static pointer _alloc(std::size_t len) {
    return static_cast<pointer>(::std::malloc(len * sizeof(value_type)));
  }
};

template <class CharT>
bool operator==(const basic_c_str_wrapper<CharT> &lhs, std::nullptr_t) {
  return lhs;
}

template <class CharT>
bool operator==(nullptr_t, const basic_c_str_wrapper<CharT> &rhs) {
  return rhs;
}

template <class CharT>
bool operator!=(const basic_c_str_wrapper<CharT> &lhs, std::nullptr_t) {
  return lhs;
}

template <class CharT>
bool operator!=(nullptr_t, const basic_c_str_wrapper<CharT> &lhs) {
  return rhs;
}

using c_str_wrapper = basic_c_str_wrapper<char>;
using wc_str_wrapper = basic_c_str_wrapper<wchar_t>;

} // namespace dpsg

#endif // GUARD_DPSG_C_STR_WRAPPER_HEADER