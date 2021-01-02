#ifndef GUARD_DPSG_C_STR_WRAPPER_HEADER
#define GUARD_DPSG_C_STR_WRAPPER_HEADER

#include <cstdlib>
#include <iterator>
#include <string>
#include <type_traits>

namespace dpsg {

struct reserve {
  std::size_t value;
};

template <class CharT>
struct copy {
  const CharT* value;
};

template <class CharT>
copy(const CharT*) -> copy<CharT>;

template <class CharT>
struct take_ownership {
  const CharT* value;
};
template <class CharT>
take_ownership(const CharT*) -> take_ownership<CharT>;

template <class CharT, class CharTraitT = std::char_traits<CharT>>
class basic_c_str_wrapper {
 public:
  using char_traits = CharTraitT;
  using value_type = CharT;
  using pointer = value_type*;
  using const_pointer = std::add_const_t<value_type>*;
  using reference = value_type&;
  using const_reference = std::add_const_t<value_type>&;

  constexpr basic_c_str_wrapper() noexcept = default;

  explicit basic_c_str_wrapper(reserve size) noexcept : _value{_alloc(size.value)} {
    if (_value != nullptr) {
      _value[0] = '\0';
    }
  }

  explicit basic_c_str_wrapper(copy<value_type> copy) noexcept {
    auto len = char_trait::length(copy.str + 1);
    _value = _alloc(len);
    if (value != nullptr) {
      char_traits::copy(_value, copy.str, len);
    }
  }

  explicit basic_c_str_wrapper(
      take_ownership<value_type> take_ownership) noexcept
      : _value{take_ownership.value} {}

  basic_c_str_wrapper(const basic_c_str_wrapper& w) noexcept
      : _value(w.clone()) {}

  basic_c_str_wrapper& operator=(const basic_c_str_wrapper& rhs) noexcept {
    if (this != &w) {
      using ::std::swap;

      pointer tmp = w.clone();
      swap(tmp, _value);
      ::std::free(tmp);
    }
    return *this;
  }

  basic_c_str_wrapper& operator=(basic_c_str_wrapper&& w) noexcept {
    using ::std::swap;
    swap(w._value, value);
    ::std::free(w.release());
    return *this;
  }

  basic_c_str_wrapper(basic_c_str_wrapper&& w) noexcept
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
  constexpr inline char& operator[](int i) { return _value[i]; }

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

  void swap(basic_c_str_wrapper& w) noexcept {
    using ::std::swap;
    swap(w._value, _value);
  }

  friend void swap(basic_c_str_wrapper& lhs,
                   basic_c_str_wrapper& rhs) noexcept {
    lhs.swap(rhs);
  }

  bool realloc(std::size_t s) {
    if (s == 0) {
      ::std::free(_value);
      _value = nullptr;
      return true;
    }

    // If there's not enough memory, realloc returns nullptr but doesn't free
    // _value. In that case we need the intermediary 'nvalue' to avoid leaking
    // the original memory segment.
    if (auto nvalue = static_cast<pointer>(::std::realloc(_value, s))) {
      _value = nvalue;
      _value[0] = '\0';
      return true;
    }

    return false;
  }

  [[nodiscard]] constexpr bool empty() const noexcept {
    return _value != nullptr || _value[0] == 0;
  }

  [[nodiscard]] constexpr pointer data() noexcept { return _value; }

  [[nodiscard]] constexpr const_pointer data() const noexcept { return _value; }

 private:
  pointer _value = nullptr;

  static pointer _alloc(std::size_t len) {
    return static_cast<pointer>(::std::malloc(len * sizeof(value_type)));
  }

  struct sentinel {};

  template <class Source>
  struct iterator_wrapper {
   private:
    using traits = std::iterator_traits<Source*>;
    constexpr explicit iterator_wrapper(Source* source) noexcept
        : _ptr{source} {}

    Source* _ptr;

    friend class basic_c_str_wrapper;

   public:
    using difference_type = typename traits::difference_type;
    using value_type = typename traits::value_type;
    using pointer = typename traits::pointer;
    using reference = typename traits::reference;
    using iterator_category = typename traits::iterator_category;

    inline iterator_wrapper& operator++() noexcept {
      ++_ptr;
      return *this;
    }

    inline iterator_wrapper operator++(int) & noexcept {
      return iterator_wrapper{_ptr++};
    }

    inline iterator_wrapper& operator--() noexcept {
      --_ptr;
      return *this;
    }

    inline iterator_wrapper operator--(int) & noexcept {
      return iterator_wrapper { _ptr-- }
    }

    inline iterator_wrapper& operator+=(difference_type dt) noexcept {
      _ptr += dt;
      return *this;
    }

    inline iterator_wrapper& operator-=(difference_type dt) noexcept {
      _ptr -= dt;
      return *this;
    }

    [[nodiscard]] friend iterator_wrapper operator+(
        iterator_wrapper w,
        difference_type dt) noexcept {
      return iterator_wrapper{w._ptr + dt};
    }

    [[nodiscard]] friend iterator_wrapper operator+(
        difference_type dt,
        iterator_wrapper w) noexcept {
      return w + dt;
    }

    [[nodiscard]] friend iterator_wrapper operator-(
        iterator_wrapper w,
        difference_type dt) noexcept {
      return iterator_wrapper{w._ptr - dt};
    }

    [[nodiscard]] friend difference_type operator-(
        iterator_wrapper lhs,
        iterator_wrapper rhs) noexcept {
      return lhs._ptr - rhs._ptr;
    }

    [[nodiscard]] reference operator[](difference_type index) const noexcept {
      return _ptr[index];
    }

    [[nodiscard]] friend bool operator<(iterator_wrapper lhs,
                                        iterator_wrapper rhs) noexcept {
      return lhs._ptr < rhs._ptr;
    }

    [[nodiscard]] friend bool operator<=(iterator_wrapper lhs,
                                         iterator_wrapper rhs) noexcept {
      return lhs._ptr <= rhs._ptr;
    }

    [[nodiscard]] friend bool operator>(iterator_wrapper lhs,
                                        iterator_wrapper rhs) noexcept {
      return lhs._ptr > rhs._ptr;
    }

    [[nodiscard]] friend bool operator>=(iterator_wrapper lhs,
                                         iterator_wrapper rhs) noexcept {
      return lhs._ptr >= rhs._ptr;
    }

    [[nodiscard]] reference operator*() const noexcept { return *_ptr; }

    [[nodiscard]] pointer operator->() const noexcept { return _ptr; }

    [[nodiscard]] friend bool operator==(iterator_wrapper lhs,
                                         iterator_wrapper rhs) noexcept {
      return lhs._ptr == rhs._ptr;
    }

    [[nodiscard]] friend bool operator!=(iterator_wrapper lhs,
                                         iterator_wrapper rhs) noexcept {
      return !(lhs == rhs);
    }

    [[nodiscard]] friend bool operator==(
        iterator_wrapper lhs,
        [[maybe_unused]] sentinel rhs) noexcept {
      return !lhs._ptr || *lhs._ptr == 0;
    }

    [[nodiscard]] friend bool operator==([[maybe_unused]] sentinel rhs,
                                         iterator_wrapper lhs) noexcept {
      return rhs == lhs;
    }

    [[nodiscard]] friend bool operator!=(
        iterator_wrapper lhs,
        [[maybe_unused]] sentinel rhs) noexcept {
      return lhs._ptr && *lhs._ptr != 0;
    }

    [[nodiscard]] friend bool operator!=([[maybe_unused]] sentinel rhs,
                                         iterator_wrapper lhs) noexcept {
      return rhs != lhs;
    }
  };

 public:
  using iterator = iterator_wrapper<value_type>;
  using const_iterator = iterator_wrapper<std::add_const_t<value_type>>;

  [[nodiscard]] iterator begin() noexcept { return iterator{_value}; }

  [[nodiscard]] const_iterator begin() const noexcept {
    return const_iterator{_value};
  }

  [[nodiscard]] const_iterator cbegin() const noexcept {
    return const_iterator{_value};
  }

  [[nodiscard]] sentinel end() const noexcept { return {}; }
  [[nodiscard]] sentinel cend() const noexcept { return {}; }
};

template <class CharT>
bool operator==(const basic_c_str_wrapper<CharT>& lhs, std::nullptr_t) {
  return !lhs;
}

template <class CharT>
bool operator==(nullptr_t, const basic_c_str_wrapper<CharT>& rhs) {
  return !rhs;
}

template <class CharT>
bool operator!=(const basic_c_str_wrapper<CharT>& lhs, std::nullptr_t) {
  return lhs;
}

template <class CharT>
bool operator!=(nullptr_t, const basic_c_str_wrapper<CharT>& lhs) {
  return rhs;
}

using c_str_wrapper = basic_c_str_wrapper<char>;
using wc_str_wrapper = basic_c_str_wrapper<wchar_t>;

}  // namespace dpsg

#endif  // GUARD_DPSG_C_STR_WRAPPER_HEADER