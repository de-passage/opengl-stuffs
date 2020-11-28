#ifndef GUARD_DPSG_COMMON_HEADER
#define GUARD_DPSG_COMMON_HEADER

#include "glad/glad.h"

#include "c_str.hpp"
#include <type_traits>

namespace dpsg {
struct width {
  int value;
};
struct height {
  int value;
};

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

enum class drawing_mode {
  points = GL_POINTS,
  line_strip = GL_LINE_STRIP,
  line_loop = GL_LINE_LOOP,
  lines = GL_LINES,
  line_strip_adjacency = GL_LINE_STRIP_ADJACENCY,
  lines_adjacency = GL_LINES_ADJACENCY,
  triangle_strip = GL_TRIANGLE_STRIP,
  triangle_fan = GL_TRIANGLE_FAN,
  triangles = GL_TRIANGLES,
  triangle_strip_adjacency = GL_TRIANGLE_STRIP_ADJACENCY,
  triangles_adjacency = GL_TRIANGLES_ADJACENCY,
};

} // namespace dpsg

#endif // GUARD_DPSG_COMMON_HEADER