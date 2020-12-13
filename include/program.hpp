#ifndef GUARD_DPSG_PROGRAM_HEADER
#define GUARD_DPSG_PROGRAM_HEADER

#include "shaders.hpp"

namespace dpsg {
namespace detail {

  template <template<class...> class B, template <class> class U, typename T>
  struct make_bind_impl {
    using type = B<U<T>, T>;
  };

  template <template<class...> class Bi, class B, std::size_t N, typename T, typename... Ts>
  struct repeat_param {
    using type = typename repeat_param<Bi, B, N - 1, T, T, Ts...>::type;
  };

  template <template<class...>class Bi, class B, typename T, typename... Ts>
  struct repeat_param<Bi, B, 0, T, Ts...> {
    using type = Bi<B, T, Ts...>;
  };

  template <template<class...> class B, template <class> class U, std::size_t N, typename T>
  struct make_bind_impl<B, U, gl::vec_t<N, T>> {
    using type = typename repeat_param<B, U<gl::vec_t<N, T>>, N - 1, T>::type;
  };

} // namespace detail

class program_error : public gl_error {
 public:
  explicit program_error(gl::program_id id) {
    int len{};
    glGetProgramiv(id.value, GL_INFO_LOG_LENGTH, &len);
    _what.realloc(len);
    if (_what != nullptr) {
      glGetProgramInfoLog(id.value, len, nullptr, _what.c_str());
    }
  }
};

class program {
 public:
  explicit program(gl::program_id i) noexcept : id{i} {}

  program(const program&) = delete;
  program(program&& s) noexcept : id(std::exchange(s.id, gl::program_id{0})) {}
  program& operator=(const program&) = delete;
  program& operator=(program&& s) noexcept {
    id = std::exchange(s.id, gl::program_id{0});
    return *this;
  }
  ~program() noexcept { gl::delete_program(id); }

  template <class... Args>
  [[nodiscard]] static result<program, program_error> create(
      Args&&... shaders) noexcept {
    static_assert(std::conjunction_v<is_shader<std::decay_t<Args>>...>,
                  "create expects a list of shaders");
    auto id = gl::create_program();
    (gl::attach_shader(id, std::forward<Args>(shaders).id()), ...);
    gl::link_program(id);
    int success{};
    glGetProgramiv(id.value, GL_LINK_STATUS, &success);

    if (success != GL_TRUE) {
      return result<program, program_error>{in_place_error, id};
    }

    return result<program, program_error>{in_place_success, id};
  }

  void use() const noexcept { gl::use_program(id); }

private:
  template <class B, class... Ts>
  struct bind_impl {
    template <
        class... Us,
        std::enable_if_t<
            std::conjunction_v<std::is_convertible<std::decay_t<Us>, Ts>...>,
            int> = 0>
    void bind(Us&&... args) const {
      gl::uniform(static_cast<const B*>(this)->id(),
                  static_cast<std::add_const_t<Ts>>(args)...);
    }
  };

  template <class B>
  struct bind_impl<B, sampler2D> {
    template <class T>
    void bind(T&& t, gl::texture_name name) const {
      bind(name);
      gl::active_texture(name);
      t.bind();
    }
    void bind(gl::texture_name name) const {
      gl::uniform(static_cast<const B*>(this)->id(),
                  static_cast<gl::int_t>(name) -
                      static_cast<gl::int_t>(gl::texture_name::_0));
    }
  };
  public:

  template <class T>
  class uniform : detail::make_bind_impl<bind_impl, uniform, T>::type {
    using base = typename detail::make_bind_impl<bind_impl, uniform, T>::type;
    gl::uniform_location _id;
    explicit uniform(gl::uniform_location i) : _id{i} {}
    friend class program;

   public:
    using base::bind;
    [[nodiscard]] gl::uniform_location id() const noexcept { return _id; }
  };

  template <class S>
  [[nodiscard]] std::optional<uniform<S>> uniform_location(
      const char* name) const noexcept {
    auto i = gl::get_uniform_location(id, name);
    if (!i.has_value()) {
      return {};
    }
    return {uniform<S>{i}};
  }

  [[nodiscard]] std::optional<gl::attrib_location> attrib_location(
      const char* name) const noexcept {
    if (auto i = gl::get_attrib_location(id, name); i.has_value()) {
      return {i};
    }
    return {};
  }

 private:
  gl::program_id id;
};
} // namespace dpsg

#endif  // GUARD_DPSG_PROGRAM_HEADER