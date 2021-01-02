#ifndef GUARD_DPSG_SHADER_HEADER
#define GUARD_DPSG_SHADER_HEADER

#include "opengl.hpp"

#include "c_str_wrapper.hpp"
#include "common.hpp"
#include "result.hpp"

#include <optional>
#include <stdexcept>
#include <type_traits>

namespace dpsg {
struct sampler2D;

class gl_error : public std::exception {
 public:
  [[nodiscard]] const char* what() const override {
    if (!_what) {
      return "error unavailable: bad alloc";
    }
    return _what.c_str();
  }

  [[nodiscard]] const c_str_wrapper& error_message() const& { return _what; }

  [[nodiscard]] c_str_wrapper&& error_message() && { return std::move(_what); }

 protected:
  c_str_wrapper _what;
};

class shader_error : public gl_error {
 public:
  explicit shader_error(gl::generic_shader_id id) {
    int out_i{};
    glGetShaderiv(id.value, GL_INFO_LOG_LENGTH, &out_i);
    _what.realloc(out_i);
    if (_what != nullptr) {
      glGetShaderInfoLog(id.value, out_i, nullptr, _what.c_str());
    }
  }
};

template <gl::shader_type Tag, class T>
class shader_source {
  T _value;

 public:
  template <
      class I,
      std::enable_if_t<std::is_convertible_v<std::decay_t<I>, T>, int> = 0>
  constexpr explicit shader_source(I&& v) noexcept
      : _value(std::forward<T>(v)){};
  const auto* c_str() const noexcept { return ::dpsg::c_str(_value); }
};

template <class T>
struct fs_source : shader_source<gl::shader_type::fragment, T> {
  template <class I>
  constexpr explicit fs_source(I&& v) noexcept
      : shader_source<gl::shader_type::fragment, T>{std::forward<I>(v)} {}

  // NOLINTNEXTLINE
  operator const shader_source<gl::shader_type::fragment, T> &()
      const noexcept {
    return *this;
  }
};
template <class T>
fs_source(T) -> fs_source<std::decay_t<T>>;

template <class T>
struct vs_source : shader_source<gl::shader_type::vertex, T> {
  template <class I>
  constexpr explicit vs_source(I&& v) noexcept
      : shader_source<gl::shader_type::vertex, T>(std::forward<I>(v)) {}

  // NOLINTNEXTLINE
  operator const shader_source<gl::shader_type::vertex, T> &() const noexcept {
    return *this;
  }
};
template <class T>
vs_source(T) -> vs_source<std::decay_t<T>>;

template <gl::shader_type Type>
class shader {
 public:
  using id_type = gl::shader_id<Type>;
  explicit shader(id_type i) noexcept : _id{i} {}

  shader(const shader&) = delete;
  shader(shader&& s) noexcept : _id(std::exchange(s._id, id_type{{0}})) {}
  shader& operator=(const shader&) = delete;
  shader& operator=(shader&& s) noexcept {
    _id = std::exchange(s._id, id_type{0});
    return *this;
  }
  ~shader() noexcept { gl::delete_shader(_id); }

  template <class Str>
  [[nodiscard]] static result<shader, shader_error> create(
      shader_source<Type, Str> source) noexcept {
    auto shader_id = gl::create_shader<Type>();
    gl::shader_source(shader_id, c_str(source));
    gl::compile_shader(shader_id);
    int success = 0;
    glGetShaderiv(shader_id.value, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
      return result<shader, shader_error>{in_place_error, shader_id};
    }
    return result<shader, shader_error>{in_place_success, shader_id};
  }

  [[nodiscard]] explicit operator unsigned int() const noexcept {
    return _id.value;
  }
  [[nodiscard]] id_type id() const noexcept { return _id; }

 private:
  id_type _id;
};

using vertex_shader = shader<gl::shader_type::vertex>;
using fragment_shader = shader<gl::shader_type::fragment>;

template <class T>
struct is_shader : std::false_type {};
template <gl::shader_type type>
struct is_shader<shader<type>> : std::true_type {};
template <class T>
constexpr static inline bool is_shader_v = is_shader<T>::value;

}  // namespace dpsg

#endif  // GUARD_DPSG_SHADER_HEADER