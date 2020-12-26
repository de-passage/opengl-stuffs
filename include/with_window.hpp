#ifndef GUARD_DPSG_WITH_WINDOW_HEADER
#define GUARD_DPSG_WITH_WINDOW_HEADER

#include "window.hpp"
#include "window/mixins.hpp"

namespace dpsg {
namespace detail {
template <class Window>
using window_mixin = mixin<Window,
                           function_key_cb,
                           framebuffer_size_cb,
                           scroll_cb,
                           cursor_pos_cb,
                           window_impl>;
}  // namespace detail

class window : public detail::window_mixin<window> {
  using base = detail::window_mixin<window>;

 public:
  explicit window(GLFWwindow* w) : base(w) {}
};

template <class T, class W = window, class... Args>
decltype(auto) with_window(window_hint::value<T> hint, Args&&... args) {
  hint();
  return with_window(std::forward<Args>(args)...);
}

template <class F,
          class T,
          class W = window,
          std::enable_if_t<!std::is_same_v<std::invoke_result_t<F&&, window&>,
                                           ExecutionStatus>,
                           int> = 0>
ExecutionStatus with_window(width w, height h, title<T> ttle, F&& f) {
  GLFWwindow* wptr =
      glfwCreateWindow(w.value, h.value, c_str(ttle), NULL, NULL);
  if (!wptr) {
    return ExecutionStatus::Failure;
  }
  W win(wptr);
  std::forward<F>(f)(win);
  return ExecutionStatus::Success;
}

template <class F,
          class T,
          class W = window,
          std::enable_if_t<std::is_same_v<std::invoke_result_t<F&&, window&>,
                                          ExecutionStatus>,
                           int> = 0>
ExecutionStatus with_window(width w, height h, title<T> ttle, F&& f) {
  GLFWwindow* wptr =
      glfwCreateWindow(w.value, h.value, ttle.c_str(), NULL, NULL);
  if (!wptr) {
    return ExecutionStatus::Failure;
  }
  W win(wptr);
  return std::forward<F>(f)(win);
}

}  // namespace dpsg

#endif  // GUARD_DPSG_WITH_WINDOW_HEADER