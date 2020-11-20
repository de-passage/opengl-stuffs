#ifndef GUARD_DPSG_SHADER_HEADER
#define GUARD_DPSG_SHADER_HEADER

#include "glad/glad.h"

#include <stdexcept>
#include <variant>

namespace dpsg {
class shader_error : public std::exception {
public:
  constexpr static inline std::size_t buffer_size = 512;
  explicit shader_error(unsigned int i) : id(i) {}

  [[nodiscard]] const char *what() const override {
    glGetShaderInfoLog(id, buffer_size, nullptr, info);
    return info;
  }

private:
  unsigned int id;
  static inline char info[buffer_size]{};
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
  explicit program_error(unsigned int i) : id{i} {}

  [[nodiscard]] const char *what() const override {
    glGetProgramInfoLog(id, 512, nullptr, info);
    return info;
  }

private:
  unsigned int id;
  static inline char info[buffer_size];
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

  void operator()() const noexcept { glUseProgram(id); }

private:
  unsigned int id;
};

} // namespace dpsg

#endif // GUARD_DPSG_SHADER_HEADER