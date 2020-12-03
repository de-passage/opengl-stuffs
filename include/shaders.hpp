#ifndef GUARD_DPSG_SHADER_HEADER
#define GUARD_DPSG_SHADER_HEADER

#include "opengl.hpp"

#include "c_str_wrapper.hpp"

#include <optional>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace dpsg {
struct sampler2D;

class shader_error : public std::exception {
public:
  constexpr static inline std::size_t buffer_size = 512;

  explicit shader_error(gl::generic_shader_id id) {
    int out_i{};
    glGetShaderiv(id.value, GL_INFO_LOG_LENGTH, &out_i);
    _what.realloc(out_i);
    if (_what != nullptr) {
      glGetShaderInfoLog(id.value, out_i, nullptr, _what.c_str());
    }
  }

  [[nodiscard]] const char *what() const override {
    if (!_what) {
      return "error unavailable: bad alloc";
    }
    return _what.c_str();
  }

  c_str_wrapper _what;
};

template <gl::shader_type Type> class shader {
public:
  using id_type = gl::shader_id<Type>;
  explicit shader(id_type i) noexcept : _id{i} {}

  shader(const shader &) = delete;
  shader(shader &&s) noexcept : _id(std::exchange(s._id, id_type{{0}})) {}
  shader &operator=(const shader &) = delete;
  shader &operator=(shader &&s) noexcept {
    _id = std::exchange(s._id, id_type{0});
    return *this;
  }
  ~shader() noexcept { gl::delete_shader(_id); }

  static std::variant<shader, shader_error>
  create(const char *source) noexcept {
    auto shader_id = gl::create_shader<Type>();
    gl::shader_source(shader_id, source);
    gl::compile_shader(shader_id);
    int success = 0;
    glGetShaderiv(shader_id.value, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
      return std::variant<shader, shader_error>{
          std::in_place_type<shader_error>, shader_id};
    }
    return std::variant<shader, shader_error>{std::in_place_type<shader>,
                                              shader_id};
  }

  explicit operator unsigned int() const noexcept { return _id.value; }
  [[nodiscard]] id_type id() const noexcept { return _id; }

private:
  id_type _id;
};

using vertex_shader = shader<gl::shader_type::vertex>;
using fragment_shader = shader<gl::shader_type::fragment>;

class program_error : public std::exception {
public:
  constexpr static inline std::size_t buffer_size = 512;
  explicit program_error(gl::program_id id) {
    int len{};
    glGetProgramiv(id.value, GL_INFO_LOG_LENGTH, &len);
    _what.realloc(len);
    if (_what != nullptr) {
      glGetProgramInfoLog(id.value, len, nullptr, _what.c_str());
    }
  }

  [[nodiscard]] const char *what() const override {
    if (_what == nullptr) {
      return "error unavailable: bad alloc";
    }

    return _what.c_str();
  }

private:
  c_str_wrapper _what;
};

class program {
public:
  explicit program(gl::program_id i) noexcept : id{i} {}

  program(const program &) = delete;
  program(program &&s) noexcept : id(std::exchange(s.id, gl::program_id{0})) {}
  program &operator=(const program &) = delete;
  program &operator=(program &&s) noexcept {
    id = std::exchange(s.id, gl::program_id{0});
    return *this;
  }
  ~program() noexcept { gl::delete_program(id); }

  template <class... Args>
  static std::variant<program, program_error>
  create(Args &&... shaders) noexcept {
    auto id = gl::create_program();
    (gl::attach_shader(id, std::forward<Args>(shaders).id()), ...);
    gl::link_program(id);
    int success{};
    glGetProgramiv(id.value, GL_LINK_STATUS, &success);

    if (success != GL_TRUE) {
      return std::variant<program, program_error>{
          std::in_place_type<program_error>, id};
    }

    return std::variant<program, program_error>{std::in_place_type<program>,
                                                id};
  }

  void use() const noexcept { gl::use_program(id); }

private:
  template <class B, class... Ts> struct bind_impl {
    template <
        class... Us,
        std::enable_if_t<
            std::conjunction_v<std::is_convertible<std::decay_t<Us>, Ts>...>,
            int> = 0>
    void bind(Us &&... args) const {
      gl::uniform(static_cast<const B *>(this)->id(),
                  std::forward<Ts>(args)...);
    }
  };

  template <class B> struct bind_impl<B, sampler2D> {
    template <class T> void bind(T &&t, gl::texture_name name) const {
      bind(name);
      gl::active_texture(name);
      t.bind();
    }
    void bind(gl::texture_name name) const {
      gl::uniform(static_cast<const B *>(this)->id(),
                  static_cast<gl::int_t>(name) -
                      static_cast<gl::int_t>(gl::texture_name::_0));
    }
  };

  template <template <class> class U, typename T> struct make_bind_impl {
    using type = bind_impl<U<T>, T>;
  };

  template <class B, std::size_t N, typename T, typename... Ts>
  struct repeat_param {
    using type = typename repeat_param<B, N - 1, T, T, Ts...>::type;
  };

  template <class B, typename T, typename... Ts>
  struct repeat_param<B, 0, T, Ts...> {
    using type = bind_impl<B, T, Ts...>;
  };

  template <template <class> class U, std::size_t N, typename T>
  struct make_bind_impl<U, gl::vec_t<N, T>> {
    using type = typename repeat_param<U<gl::vec_t<N, T>>, N - 1, T>::type;
  };

public:
  template <class T> class uniform : make_bind_impl<uniform, T>::type {
    using base = typename make_bind_impl<uniform, T>::type;
    gl::uniform_location _id;
    explicit uniform(gl::uniform_location i) : _id{i} {}
    friend class program;

  public:
    using base::bind;
    [[nodiscard]] gl::uniform_location id() const { return _id; }
  };

  template <class S>
  std::optional<uniform<S>> uniform_location(const char *c) const {
    auto i = gl::get_uniform_location(id, c);
    if (!i.has_value()) {
      return {};
    }
    return {uniform<S>{i}};
  }

private:
  gl::program_id id;
};

} // namespace dpsg

#endif // GUARD_DPSG_SHADER_HEADER