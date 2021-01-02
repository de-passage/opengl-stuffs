#ifndef GUARD_DPSG_LOAD_SHADERS_HEADER
#define GUARD_DPSG_LOAD_SHADERS_HEADER

#include "c_str.hpp"
#include "c_str_wrapper.hpp"
#include "common.hpp"
#include "program.hpp"
#include "result.hpp"
#include "shaders.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>

namespace dpsg {

template <class T, class U>
result<program, gl_error> create_program(
    const vs_source<T>& vshader_source,
    const fs_source<U>& fshader_source) noexcept {
  return vertex_shader::create(vshader_source).then([&](auto&& vshader) {
    return fragment_shader::create(fshader_source).then([&](auto&& fshader) {
      return result<program, gl_error>(program::create(vshader, fshader));
    });
  });
}

DPSG_LAZY_STR_WRAPPER_IMPL(vs_filename)  // NOLINT
DPSG_LAZY_STR_WRAPPER_IMPL(fs_filename)  // NOLINT

struct loading_error : std::exception {
  loading_error(const char* filename, const char* error_message) noexcept
      : loading_error(filename,
                      std::strlen(filename),
                      error_message,
                      std::strlen(error_message)) {}

  loading_error(const char* filename,
                std::size_t fn_len,
                const char* error_message,
                std::size_t em_len) noexcept
      : _what(reserve{fn_len + em_len + 3}) {
    if (_what) {
      using std::begin;
      using std::end;
      auto it = std::copy(filename, filename + fn_len + 1, begin(_what));
      *it++ = ':';
      *it++ = '\n';
      std::copy(error_message, error_message + em_len, it);
    }
  }

  [[nodiscard]] const char* what() const noexcept { return _what.c_str(); }

 private:
  c_str_wrapper _what;
};

namespace detail {
template <class... Args>
inline result<std::string, loading_error> load_from_stream(
    std::basic_istream<Args...>& stream) {
  std::stringstream sstream;
  sstream << stream.rdbuf();

  return success{sstream.str()};
}

inline result<std::string, loading_error> load_from_disk(const char* name) {
  std::ifstream file(name);
  if (!file.is_open()) {
    return failure{name, std::strerror(errno)};
  }
  return load_from_stream(file);
}
}  // namespace detail

template <class T>
result<fs_source<std::string>, loading_error> load(
    const fs_filename<T>& fname) {
  return detail::load_from_disk(c_str(fname))
      .template cast<fs_source<std::string>>();
}

template <class T>
result<vs_source<std::string>, loading_error> load(
    const vs_filename<T>& fname) {
  return detail::load_from_disk(c_str(fname))
      .template cast<vs_source<std::string>>();
}

template <class T, class U>
result<program, loading_error> load(const vs_filename<T>& vs,
                                    const fs_filename<U>& fs) {
  constexpr auto to_loading_error = [](auto&& filename) {
    return [&filename](gl_error&& error) -> loading_error {
      return loading_error(c_str(filename), error.error_message().c_str());
    };
  };
  return load(vs).then([&](auto&& vshader_source) {
    return load(fs).then([&](auto&& fshader_source) {
      return vertex_shader::create(std::move(vshader_source))
          .map_error(to_loading_error(vs))
          .then([&](auto&& vshader) {
            return fragment_shader::create(std::move(fshader_source))
                .map_error(to_loading_error(fs))
                .then([&](auto&& fshader) {
                  return program::create(std::move(vshader), std::move(fshader))
                      .map_error(to_loading_error("linking"));
                });
          });
    });
  });
}

}  // namespace dpsg

#endif  // GUARD_DPSG_LOAD_SHADERS_HEADER