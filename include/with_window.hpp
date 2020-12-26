#ifndef GUARD_DPSG_WITH_WINDOW_HEADER
#define GUARD_DPSG_WITH_WINDOW_HEADER

#include "window.hpp"
#include "window/mixins.hpp"

namespace dpsg {

template <class Window, class... Args>
using window_mixin = mixin<Window, Args..., window_impl>;

namespace detail {
template <class Window>
using window_mixin = mixin<Window,
                           function_key_cb,
                           framebuffer_size_cb,
                           scroll_cb,
                           cursor_pos_cb,
                           window_impl>;
}  // namespace detail

template <class... Mixins>
class base_window : public window_mixin<base_window<Mixins...>, Mixins...> {
  using base = window_mixin<base_window, Mixins...>;

 public:
  explicit base_window(GLFWwindow* w) : base(w) {}
};

class window : public detail::window_mixin<window> {
  using base = detail::window_mixin<window>;

 public:
  explicit window(GLFWwindow* w) : base(w) {}
};

template <class W = window, class T, class... Args>
decltype(auto) with_window(window_hint::value<T> hint, Args&&... args) {
  hint();
  return with_window<W>(std::forward<Args>(args)...);
}

template <class W = window,
          class F,
          class T,
          std::enable_if_t<
              !std::is_same_v<std::invoke_result_t<F&&, W&>, ExecutionStatus>,
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

template <class W = window,
          class F,
          class T,
          std::enable_if_t<
              std::is_same_v<std::invoke_result_t<F&&, W&>, ExecutionStatus>,
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