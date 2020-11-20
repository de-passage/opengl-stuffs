#ifndef GUARD_DPSG_WINDOW_HEADER
#define GUARD_DPSG_WINDOW_HEADER

#include "input/keys.hpp"
#include "utility.hpp"
#include "window/hints.hpp"

#include "GLFW/glfw3.h"

#include <type_traits>
#include <utility>

namespace dpsg {

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

  void make_context_current() const { glfwMakeContextCurrent(_window); }

  GLFWframebuffersizefun
  set_framebuffer_size_callback(GLFWframebuffersizefun f) const {
    return glfwSetFramebufferSizeCallback(_window, f);
  }

  bool should_close() const noexcept {
    return glfwWindowShouldClose(_window) == GLFW_TRUE;
  }

  void should_close(bool b) const noexcept {
    glfwSetWindowShouldClose(_window, b ? GLFW_TRUE : GLFW_FALSE);
  }

  input::status get_key(input::key k) const noexcept {
    return static_cast<input::status>(glfwGetKey(_window, static_cast<int>(k)));
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