#ifndef GUARD_DPSG_SHADER_HEADER
#define GUARD_DPSG_SHADER_HEADER

#include "glad/glad.h"

#include "c_str_wrapper.hpp"
#include "meta/is_one_of.hpp"
#include "texture.hpp"


#include <optional>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace dpsg {
class shader_error : public std::exception {
public:
  constexpr static inline std::size_t buffer_size = 512;
  explicit shader_error(unsigned int i) : id(i) {
    int out_i{};
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &out_i);
    _what.realloc(out_i);
    if (_what != nullptr) {
      glGetShaderInfoLog(id, out_i, nullptr, _what.c_str());
    }
  }

  [[nodiscard]] const char *what() const override {
    if (!_what) {
      return "error unavailable: bad alloc";
    }
    return _what.c_str();
  }

private:
  unsigned int id;
  c_str_wrapper _what;
};

template <int Type> class shader {
public:
  explicit shader(unsigned int i) noexcept : id{i} {}

  shader(const shader &) = delete;
  shader(shader &&s) noexcept : id(std::exchange(s.id, 0)) {}
  shader &operator=(const shader &) = delete;
  shader &operator=(shader &&s) noexcept {
    id = std::exchange(s.id, 0);
    return *this;
  }
  ~shader() noexcept { glDeleteShader(id); }

  static std::variant<shader, shader_error>
  create(const char *source) noexcept {
    unsigned int shader_id = glCreateShader(Type);
    glShaderSource(shader_id, 1, &source, nullptr);
    glCompileShader(shader_id);
    int success = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
      return std::variant<shader, shader_error>{
          std::in_place_type<shader_error>, shader_id};
    }
    return std::variant<shader, shader_error>{std::in_place_type<shader>,
                                              shader_id};
  }

  explicit operator unsigned int() const noexcept { return id; }

private:
  unsigned int id;
};

using vertex_shader = shader<GL_VERTEX_SHADER>;
using fragment_shader = shader<GL_FRAGMENT_SHADER>;

class program_error : public std::exception {
public:
  constexpr static inline std::size_t buffer_size = 512;
  explicit program_error(unsigned int i) : id{i} {
    int len{};
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &len);
    _what.realloc(len);
    if (_what != nullptr) {
      glGetProgramInfoLog(id, len, nullptr, _what.c_str());
    }
  }

  [[nodiscard]] const char *what() const override {
    if (_what == nullptr) {
      return "error unavailable: bad alloc";
    }

    return _what.c_str();
  }

private:
  unsigned int id;
  c_str_wrapper _what;
};

template <std::size_t N, class T> struct vec {
  static_assert(is_one_of_v<T, float, int, unsigned int>,
                "invalid type in opengl vec specification");
  static_assert(N > 1 && N <= 4, "invalid vec size");
};

class program {
public:
  explicit program(unsigned int i) noexcept : id{i} {}

  program(const program &) = delete;
  program(program &&s) noexcept : id(std::exchange(s.id, 0)) {}
  program &operator=(const program &) = delete;
  program &operator=(program &&s) noexcept {
    id = std::exchange(s.id, 0);
    return *this;
  }
  ~program() noexcept { glDeleteProgram(id); }

  template <int I>
  static void attach_shader(unsigned int i, const shader<I> &shader) noexcept {
    glAttachShader(i, static_cast<unsigned int>(shader));
  }

  template <class... Args>
  static std::variant<program, program_error>
  create(Args &&... shaders) noexcept {
    unsigned int id = glCreateProgram();
    (attach_shader(id, std::forward<Args>(shaders)), ...);
    glLinkProgram(id);
    int success{};
    glGetProgramiv(id, GL_LINK_STATUS, &success);

    if (success != GL_TRUE) {
      return std::variant<program, program_error>{
          std::in_place_type<program_error>, id};
    }

    return std::variant<program, program_error>{std::in_place_type<program>,
                                                id};
  }

  void use() const noexcept { glUseProgram(id); }

private:
  template <class B, class T> struct bind_impl;
  template <class B> struct bind_impl<B, int> {
    void bind(int i1) const {
      glUniform1i(static_cast<const B *>(this)->id(), i1);
    }
  };

  template <class B> struct bind_impl<B, float> {
    void bind(float f1) const {
      glUniform1f(static_cast<const B *>(this)->id(), f1);
    }
  };

  template <class B> struct bind_impl<B, vec<2, float>> {
    void bind(float f1, float f2) const {
      glUniform2f(static_cast<const B *>(this)->id(), f1, f2);
    }
  };

  template <class B> struct bind_impl<B, vec<3, float>> {
    void bind(float f1, float f2, float f3) const {
      glUniform2f(static_cast<const B *>(this)->id(), f1, f2, f3);
    }
  };

  template <class B> struct bind_impl<B, vec<4, float>> {
    void bind(float f1, float f2, float f3, float f4) const {
      glUniform4f(static_cast<const B *>(this)->id(), f1, f2, f3, f4);
    }
  };

  template <class B> struct bind_impl<B, texture> {
    void bind(const texture &txt) const {
      glUniform1i(static_cast<const B *>(this)->id(), txt.id());
    }
  };

public:
  template <class T> class uniform : bind_impl<uniform<T>, T> {
    int _id;
    explicit uniform(int i) : _id{i} {}
    friend class program;

  public:
    using bind_impl<uniform<T>, T>::bind;
    [[nodiscard]] int id() const { return _id; }
  };

  template <class S>
  std::optional<uniform<S>> uniform_location(const char *c) const {
    int i = glGetUniformLocation(id, c);
    if (i < 0) {
      return {};
    }
    return {uniform<S>{i}};
  }

private:
  unsigned int id;
};

} // namespace dpsg

#endif // GUARD_DPSG_SHADER_HEADER