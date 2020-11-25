#ifndef GUARD_DPSG_LOAD_SHADERS_HEADER
#define GUARD_DPSG_LOAD_SHADERS_HEADER

#include "c_str.hpp"
#include "common.hpp"
#include "shaders.hpp"


#include <fstream>
#include <sstream>
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

DPSG_LAZY_STR_WRAPPER_IMPL(vs_source) // NOLINT
DPSG_LAZY_STR_WRAPPER_IMPL(fs_source) // NOLINT

template <class T, class U>
program create_program(const vs_source<T> &vshader_source,
                       const fs_source<U> &fshader_source) {
  auto vshader = get_or_throw(vertex_shader::create(c_str(vshader_source)));
  auto fshader = get_or_throw(fragment_shader::create(c_str(fshader_source)));
  auto prog = get_or_throw(program::create(vshader, fshader));
  return prog;
}

DPSG_LAZY_STR_WRAPPER_IMPL(vs_filename) // NOLINT
DPSG_LAZY_STR_WRAPPER_IMPL(fs_filename) // NOLINT

namespace detail {
template <class... Args>
inline std::string load_from_stream(std::basic_istream<Args...> &stream) {
  std::stringstream sstream;
  sstream << stream.rdbuf();
  return sstream.str();
}

inline std::string load_from_disk(const char *name) {
  std::ifstream file(name);
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  return load_from_stream(file);
}
} // namespace detail

template <class T>
fs_source<std::string> load_from_disk(const fs_filename<T> &fname) {
  return fs_source{detail::load_from_disk(c_str(fname))};
}

template <class T>
vs_source<std::string> load_from_disk(const vs_filename<T> &fname) {
  return vs_source{detail::load_from_disk(c_str(fname))};
}

} // namespace dpsg

#endif // GUARD_DPSG_LOAD_SHADERS_HEADER