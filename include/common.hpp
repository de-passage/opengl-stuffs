#ifndef GUARD_DPSG_COMMON_HEADER
#define GUARD_DPSG_COMMON_HEADER

#include "glad/glad.h"

#include "c_str.hpp"
#include "opengl.hpp"
#include <limits>
#include <type_traits>

namespace dpsg {
struct width {
  int value;
};
struct height {
  int value;
};

struct aspect_ratio {
  float value;
};

inline constexpr aspect_ratio operator/(width w, height h) {
  return aspect_ratio{static_cast<float>(w.value) /
                      static_cast<float>(h.value)};
}

// NOLINTNEXTLINE
#define DPSG_LAZY_STR_WRAPPER_IMPL(name)                                       \
  template <class T> class name {                                              \
    T _value;                                                                  \
                                                                               \
  public:                                                                      \
    template <class I,                                                         \
              std::enable_if_t<std::is_same_v<std::decay_t<I>, T>, int> = 0>   \
    constexpr explicit name(I &&v) noexcept : _value(std::forward<T>(v)){};    \
    const auto *c_str() const { return ::dpsg::c_str(_value); }                \
  };                                                                           \
  template <class T> name(T &&) -> name<std::decay_t<T>>;

DPSG_LAZY_STR_WRAPPER_IMPL(title) // NOLINT

template <class> struct basic_degrees;

template <class T> struct basic_radians {
  static_assert(std::numeric_limits<T>::is_iec559,
                "radians only accept floating-point underlying types");
  T value;
};
template <class T>
constexpr inline basic_degrees<T> to_degrees(basic_radians<T> rads) {
  return {rads.value *
          static_cast<T>(57.295779513082320876798154814105)}; // NOLINT
}
using radians = basic_radians<float>;

template <class T> struct basic_degrees {
  static_assert(std::numeric_limits<T>::is_iec559,
                "degrees only accept floating-point underlying types");
  T value;
};

template <class T>
constexpr inline basic_radians<T> to_radians(basic_degrees<T> degs) {
  return {degs.value *
          static_cast<T>(0.01745329251994329576923690768489)}; // NOLINT
}

using degrees = basic_degrees<float>;

} // namespace dpsg

#endif // GUARD_DPSG_COMMON_HEADER