#ifndef GUARD_DPSG_WINDOW_HEADER
#define GUARD_DPSG_WINDOW_HEADER

#include "input/keys.hpp"
#include "meta/mixin.hpp"
#include "utility.hpp"
#include "window/hints.hpp"

#include "GLFW/glfw3.h"

#include <functional>
#include <type_traits>
#include <utility>

namespace dpsg {

template <class T, class... Args>
decltype(auto) with_window(window_hint::value<T> hint, Args &&... args) {
  hint();
  return with_window(std::forward<Args>(args)...);
}

struct window_impl {
  template <class T> class type : public T {
  public:
    explicit type(GLFWwindow *w) : _window(w) {
      glfwSetWindowUserPointer(w, static_cast<void *>(this));
    }

    type(type &&w) noexcept : _window(std::exchange(w._window, nullptr)) {}
    type(const type &) = delete;
    type &operator=(type &&w) noexcept {
      std::swap(w._window, _window);
      w._clean();
      return *this;
    }

    type &operator=(const type &) = delete;
    ~type() { _clean(); }

    [[nodiscard]] constexpr const GLFWwindow *data() const noexcept {
      return _window;
    }
    [[nodiscard]] constexpr GLFWwindow *data() noexcept { return _window; }
    GLFWwindow *release() &&noexcept { return std::exchange(_window, nullptr); }

    void make_context_current() const { glfwMakeContextCurrent(_window); }

    GLFWframebuffersizefun
    set_framebuffer_size_callback(GLFWframebuffersizefun f) const {
      return glfwSetFramebufferSizeCallback(_window, f);
    }

    [[nodiscard]] bool should_close() const noexcept {
      return glfwWindowShouldClose(_window) == GLFW_TRUE;
    }

    void should_close(bool b) const noexcept {
      glfwSetWindowShouldClose(_window, b ? GLFW_TRUE : GLFW_FALSE);
    }

    [[nodiscard]] input::status get_key(input::key k) const noexcept {
      return static_cast<input::status>(
          glfwGetKey(_window, static_cast<int>(k)));
    }

    void swap_buffers() const noexcept { glfwSwapBuffers(_window); }

    template <class F> void render_loop(F f) const noexcept(noexcept(f())) {
      while (!should_close()) {
        f();
        swap_buffers();
        glfwPollEvents();
      }
    }

  private:
    void _clean() {
      if (_window != nullptr) {
        glfwDestroyWindow(_window);
      }
    }

  protected:
    GLFWwindow *_window;
  };
};

template <class... Args>
class window_base
    : public dpsg::mixin<window_base<Args...>, Args..., window_impl> {
  using base = mixin<window_base<Args...>, Args..., window_impl>;

public:
  explicit window_base(GLFWwindow *w) : base(w) {}
};

namespace detail {
template <class T> T *get_window_ptr(GLFWwindow *w) {
  return static_cast<T *>(glfwGetWindowUserPointer(w));
}
} // namespace detail

template <class R = void> struct function_key_cb {
  template <class B> struct type : B {
    using signature = R(real_type_t<B> &, input::key, int, input::status, int);
    using key_callback = std::function<signature>;

    template <class... Args>
    constexpr explicit type(Args &&... args) noexcept(
        std::is_nothrow_constructible_v<B, Args...>)
        : B(std::forward<Args>(args)...) {}

  private:
    static void call(GLFWwindow *w, int key, int scancode, int action,
                     int mods) {
      auto *ptr = detail::get_window_ptr<type>(w);
      if (auto &f = ptr->_cb) {
        f(*static_cast<real_type_t<B> *>(ptr), static_cast<input::key>(key),
          static_cast<int>(scancode), static_cast<input::status>(action),
          static_cast<int>(mods));
      }
    }

  public:
    key_callback set_key_callback(key_callback f) {
      if (f) {
        glfwSetKeyCallback(B::_window, &call);
        return std::exchange(_cb, std::move(f));
      } else {
        glfwSetKeyCallback(B::_window, nullptr);
        return std::exchange(_cb, nullptr);
      }
    }

  private:
    key_callback _cb;
  };
};

class window : public mixin<window, function_key_cb<>, window_impl> {
  using base = mixin<window, function_key_cb<>, window_impl>;

public:
  explicit window(GLFWwindow *w) : base(w) {}

  struct width {
    int value;
  };
  struct height {
    int value;
  };
  struct title {
    const char *value;
  };
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