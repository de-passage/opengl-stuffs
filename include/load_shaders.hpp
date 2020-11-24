#ifndef GUARD_DPSG_LOAD_SHADERS_HEADER
#define GUARD_DPSG_LOAD_SHADERS_HEADER

#include "c_str.hpp"
#include "shaders.hpp"
#include <string>
#include <type_traits>
#include <variant>

namespace dpsg {
template <class V, class R = std::variant_alternative_t<0, V>>
R get_or_throw(V &&v) {
  if (auto *err = std::get_if<std::variant_alternative_t<1, V>>(&v)) {
    throw *err;
  }
  return std::get<R>(std::forward<V>(v));
}

namespace detail {

template <class T>
using deduce_shader_source_contained_type =
    std::conditional_t<detail::is_char_ptr_v<T>, std::decay_t<T>, T>;
} // namespace detail

template <class T> class vs_source {
  T _value;

public:
  template <class I,
            std::enable_if_t<std::is_same_v<std::decay_t<I>, T>, int> = 0>
  // NOLINTNEXTLINE the SFINAE check guarantees no shadowing
  constexpr explicit vs_source(I &&v) noexcept : _value(std::forward<T>(v)){};
  T c_str() const { return detail::c_str(_value); }
};

template <class T>
vs_source(T &&) -> vs_source<detail::deduce_shader_source_contained_type<T>>;

template <class T> class fs_source {
  T _value;

public:
  template <class I,
            std::enable_if_t<std::is_same_v<std::decay_t<I>, T>, int> = 0>
  // NOLINTNEXTLINE the SFINAE check guarantees no shadowing
  constexpr explicit fs_source(I &&v) noexcept : _value(std::forward<I>(v)){};
  T c_str() const { return detail::c_str(_value); }
};

template <class T>
fs_source(T &&) -> fs_source<detail::deduce_shader_source_contained_type<T>>;

template <class T>
program create_program(const vs_source<T> &vshader_source,
                       const fs_source<T> &fshader_source) {
  auto vshader =
      get_or_throw(vertex_shader::create(detail::c_str(vshader_source)));
  auto fshader =
      get_or_throw(fragment_shader::create(detail::c_str(fshader_source)));
  auto prog = get_or_throw(program::create(vshader, fshader));
  return prog;
}

} // namespace dpsg

#endif // GUARD_DPSG_LOAD_SHADERS_HEADER