#ifndef GUARD_DPSG_INIT_HINT_HEADER
#define GUARD_DPSG_INIT_HINT_HEADER

#include "utility.hpp"

#include "GLFW/glfw3.h"

#include <type_traits>

namespace dpsg {

struct init_hint {

  template <int Hint> struct value {
    value(const value &) noexcept = default;
    value(value &&) noexcept = default;
    value &operator=(const value &) noexcept = default;
    value &operator=(value &&) noexcept = default;
    ~value() noexcept = default;

  private:
    bool v = true;
    value() noexcept = default;
    constexpr explicit value(bool b) noexcept : v(b) {}
    friend init_hint;

  public:
    void operator()() noexcept { glfwInitHint(Hint, v); }
  };

  static constexpr value<GLFW_JOYSTICK_HAT_BUTTONS>
  joystick_hat_buttons(bool b) noexcept {
    return value<GLFW_JOYSTICK_HAT_BUTTONS>{b};
  }
  static constexpr value<GLFW_COCOA_CHDIR_RESOURCES>
  cocoa_chdir_resources(bool b) noexcept {
    return value<GLFW_COCOA_CHDIR_RESOURCES>{b};
  }
  static constexpr value<GLFW_COCOA_MENUBAR>
  cocoa_chdir_menubar(bool b) noexcept {
    return value<GLFW_COCOA_MENUBAR>{b};
  }
};

template <class F,
          std::enable_if_t<
              !std::is_same_v<std::invoke_result<F>, ExecutionStatus>, int> = 0>
ExecutionStatus
within_glfw_context(F &&f) noexcept(noexcept(std::forward<F>(f)())) {
  if (glfwInit() == GLFW_FALSE) {
    return ExecutionStatus::Failure;
  }

  auto on_exit = on_scope_exit_t{glfwTerminate};

  std::forward<F>(f)();

  return ExecutionStatus::Success;
}

template <class F,
          std::enable_if_t<
              std::is_same_v<std::invoke_result<F>, ExecutionStatus>, int> = 0>
ExecutionStatus
within_glfw_context(F &&f) noexcept(noexcept(std::forward<F>(f)())) {
  if (glfwInit() == GLFW_FALSE) {
    return ExecutionStatus::Failure;
  }

  auto on_exit = on_scope_exit_t{glfwTerminate};

  auto result = std::forward<F>(f)();

  return result;
}

template <int I, class... Args>
decltype(auto) within_glfw_context(init_hint::value<I> hint,
                                   Args &&... args) noexcept {
  hint();
  return within_glfw_context(std::forward<Args>(args)...);
}

} // namespace dpsg

#endif // GUARD_DPSG_INIT_HINT_HEADER