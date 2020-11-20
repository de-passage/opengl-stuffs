#ifndef GUARD_DPSG_WINDOW_HEADER
#define GUARD_DPSG_WINDOW_HEADER

#include "input/keys.hpp"
#include "utility.hpp"


#include "GLFW/glfw3.h"

#include <type_traits>
#include <utility>

namespace dpsg {

enum class profile {
  core = GLFW_OPENGL_CORE_PROFILE,
  any = GLFW_OPENGL_ANY_PROFILE,
  compat = GLFW_OPENGL_COMPAT_PROFILE
};

struct window_hint {
private:
  template <int Hint, class T> struct trait_impl {
    template <class U,
              std::enable_if_t<std::is_convertible_v<U &&, T>, int> = 0>
    constexpr static void set_hint(U &&u) noexcept {
      glfwWindowHint(Hint, static_cast<int>(std::forward<T>(u)));
    }
  };

  template <int Hint> struct trait_impl<Hint, const char *> {
    constexpr static void set_hint(const char *c) noexcept {
      glfwWindowHintString(Hint, c);
    }
  };

  template <int Hint> struct trait_impl<Hint, bool> {
    constexpr static void set_hint(bool b) noexcept {
      glfwWindowHint(Hint, b ? GLFW_TRUE : GLFW_FALSE);
    }
  };

  template <int Hint, class T> struct trait : trait_impl<Hint, T> {};

public:
  template <int Hint, class T> struct hint_type {
    constexpr static inline int hint = Hint;
    using type = T;
  };
  template <class... Args> struct hint_list;

  template <class T> struct value;

  template <int Hint, class T> struct value<hint_type<Hint, T>> {
    value(const value &) noexcept = default;
    value(value &&) noexcept = default;
    value &operator=(const value &) noexcept = default;
    value &operator=(value &&) noexcept = default;
    ~value() noexcept = default;

  private:
    T v = {};
    value() noexcept = default;
    constexpr explicit value(T t) noexcept : v(t) {}
    friend window_hint;

  public:
    void operator()() noexcept { trait<Hint, T>::set_hint(v); }
  };

  template <class T, class... Ts>
  struct value<hint_list<T, Ts...>> : value<hint_list<Ts...>> {
    value(const value &) noexcept = default;
    value(value &&) noexcept = default;
    value &operator=(const value &) noexcept = default;
    value &operator=(value &&) noexcept = default;
    ~value() noexcept = default;

  private:
    using base = value<hint_list<Ts...>>;
    typename T::type _value;
    value() noexcept = default;
    template <
        class U, class... Args,
        std::enable_if_t<std::is_convertible_v<U, typename T::type>, int> = 0>
    constexpr explicit value(U &&u, Args &&... ts) noexcept
        : base(std::forward<Args>(ts)...), _value(std::forward<U>(u)) {}

    friend window_hint;

  public:
    void operator()() noexcept {
      trait<T::hint, typename T::type>::set_hint(_value);
      base::operator()();
    }
  };

  template <> struct value<hint_list<>> {
    void operator()() noexcept {}
  };

  template <int I, class T> using hint = value<hint_type<I, T>>;

  template <int I> using bool_hint = hint_type<I, bool>;

  template <int I> using string_hint = hint_type<I, const char *>;

  template <int I> using int_hint = hint_type<I, int>;

  using context_version_major_t = int_hint<GLFW_CONTEXT_VERSION_MAJOR>;
  static constexpr value<context_version_major_t>
  context_version_major(int i) noexcept {
    return value<context_version_major_t>{i};
  }

  using context_version_minor_t = int_hint<GLFW_CONTEXT_VERSION_MINOR>;
  static constexpr value<context_version_minor_t>
  context_version_minor(int i) noexcept {
    return value<context_version_minor_t>{i};
  }

  template <class... Args> using composite_hint = value<hint_list<Args...>>;
  static constexpr composite_hint<context_version_major_t,
                                  context_version_minor_t>
  context_version(int major, int minor) noexcept {
    return composite_hint<context_version_major_t, context_version_minor_t>{
        major, minor};
  }

  static constexpr hint<GLFW_OPENGL_PROFILE, profile>
  opengl_profile(profile p) noexcept {
    return hint<GLFW_OPENGL_PROFILE, profile>{p};
  }

  static constexpr value<bool_hint<GLFW_OPENGL_FORWARD_COMPAT>>
  opengl_forward_compat(bool b) noexcept {
    return value<bool_hint<GLFW_OPENGL_FORWARD_COMPAT>>{b};
  }
};

template <class T, class... Args>
decltype(auto) with_window(window_hint::value<T> hint, Args &&... args) {
  hint();
  return with_window(std::forward<Args>(args)...);
}

class window {
public:
  struct width {
    int value;
  };
  struct height {
    int value;
  };
  struct title {
    const char *value;
  };

  explicit window(GLFWwindow *w) : _window(w) {}
  window(window &&w) noexcept : _window(std::exchange(w._window, nullptr)) {}
  window(const window &) = delete;
  window &operator=(window &&w) noexcept {
    std::swap(w._window, _window);
    w._clean();
    return *this;
  }
  window &operator=(const window &) = delete;
  ~window() { _clean(); }

  constexpr const GLFWwindow *data() const noexcept { return _window; }
  constexpr GLFWwindow *data() noexcept { return _window; }
  GLFWwindow *release() &&noexcept { return std::exchange(_window, nullptr); }

  void make_context_current() { glfwMakeContextCurrent(_window); }
  GLFWframebuffersizefun
  set_framebuffer_size_callback(GLFWframebuffersizefun f) {
    return glfwSetFramebufferSizeCallback(_window, f);
  }

  bool should_close() const {
    return glfwWindowShouldClose(_window) == GLFW_TRUE;
  }

  void should_close(bool b) const {
    glfwSetWindowShouldClose(_window, b ? GLFW_TRUE : GLFW_FALSE);
  }

  input::status get_key(input::key k) const {
    return static_cast<input::status>(glfwGetKey(_window, static_cast<int>(k)));
  }

  void swap_buffers() { glfwSwapBuffers(_window); }

private:
  void _clean() {
    if (_window) {
      glfwDestroyWindow(_window);
    }
  }
  GLFWwindow *_window;
};

template <class F,
          std::enable_if_t<!std::is_same_v<std::invoke_result_t<F &&, window &>,
                                           ExecutionStatus>,
                           int> = 0>
ExecutionStatus with_window(window::width w, window::height h,
                            window::title title, F &&f) {
  GLFWwindow *wptr =
      glfwCreateWindow(w.value, h.value, title.value, NULL, NULL);
  if (!wptr) {
    return ExecutionStatus::Failure;
  }
  window win(wptr);
  std::forward<F>(f)(win);
  return ExecutionStatus::Success;
}

template <class F,
          std::enable_if_t<std::is_same_v<std::invoke_result_t<F &&, window &>,
                                          ExecutionStatus>,
                           int> = 0>
ExecutionStatus with_window(window::width w, window::height h,
                            window::title title, F &&f) {
  GLFWwindow *wptr =
      glfwCreateWindow(w.value, h.value, title.value, NULL, NULL);
  if (!wptr) {
    return ExecutionStatus::Failure;
  }
  window win(wptr);
  return std::forward<F>(f)(win);
}

} // namespace dpsg

#endif // GUARD_DPSG_WINDOW_HEADER