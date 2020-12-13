#ifndef GUARD_DPSG_LOAD_SHADERS_HEADER
#define GUARD_DPSG_LOAD_SHADERS_HEADER

#include "c_str.hpp"
#include "common.hpp"
#include "shaders.hpp"
#include "program.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>

namespace dpsg {

template <class T, class U>
program create_program(const vs_source<T>& vshader_source,
                       const fs_source<U>& fshader_source) {
  auto vshader = vertex_shader::create(vshader_source).value();
  auto fshader = fragment_shader::create(fshader_source).value();
  auto prog = program::create(vshader, fshader).value();
  return prog;
}

DPSG_LAZY_STR_WRAPPER_IMPL(vs_filename)  // NOLINT
DPSG_LAZY_STR_WRAPPER_IMPL(fs_filename)  // NOLINT

namespace detail {
template <class... Args>
inline std::string load_from_stream(std::basic_istream<Args...>& stream) {
  std::stringstream sstream;
  sstream << stream.rdbuf();
  return sstream.str();
}

inline std::string load_from_disk(const char* name) {
  std::ifstream file(name);
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  return load_from_stream(file);
}
}  // namespace detail

template <class T>
fs_source<std::string> load(const fs_filename<T>& fname) {
  return fs_source{detail::load_from_disk(c_str(fname))};
}

template <class T>
vs_source<std::string> load(const vs_filename<T>& fname) {
  return vs_source{detail::load_from_disk(c_str(fname))};
}

template <class T, class U>
program load(const vs_filename<T>& vs, const fs_filename<U>& fs) {
  return create_program(load(vs), load(fs));
}

}  // namespace dpsg

#endif  // GUARD_DPSG_LOAD_SHADERS_HEADER