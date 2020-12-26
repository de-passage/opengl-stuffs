#ifndef GUARD_DPSG_WINDOW_HEADER
#define GUARD_DPSG_WINDOW_HEADER

#include "glad/glad.h"

#include "common.hpp"
#include "input/keys.hpp"
#include "meta/mixin.hpp"
#include "utility.hpp"
#include "window/hints.hpp"

#include "GLFW/glfw3.h"

#include <chrono>
#include <thread>
#include <type_traits>
#include <utility>

namespace dpsg {

enum class input_mode : int {
  sticky_keys = GLFW_STICKY_KEYS,
  sticky_mouse_buttons = GLFW_STICKY_MOUSE_BUTTONS,
  lock_key_mods = GLFW_LOCK_KEY_MODS,
  raw_mouse_motion = GLFW_RAW_MOUSE_MOTION,
};

enum class cursor_mode : int {
  normal = GLFW_CURSOR_NORMAL,
  hidden = GLFW_CURSOR_HIDDEN,
  disabled = GLFW_CURSOR_DISABLED,
};

struct rect {
  width width;
  height height;
};

struct framebuffer_dimension : rect {};
struct window_dimension : rect {};

struct window_impl {
  template <class T>
  class type : public T {
   public:
    explicit type(GLFWwindow* w) : _window(w) {
      glfwSetWindowUserPointer(w, static_cast<void*>(this));
    }

    type(type&& w) noexcept : _window(std::exchange(w._window, nullptr)) {}
    type(const type&) = delete;
    type& operator=(type&& w) noexcept {
      std::swap(w._window, _window);
      w._clean();
      return *this;
    }

    type& operator=(const type&) = delete;
    ~type() { _clean(); }

    [[nodiscard]] constexpr const GLFWwindow* data() const noexcept {
      return _window;
    }
    [[nodiscard]] constexpr GLFWwindow* data() noexcept { return _window; }
    GLFWwindow* release() && noexcept {
      return std::exchange(_window, nullptr);
    }

    void make_context_current() const { glfwMakeContextCurrent(_window); }

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

    template <class F>
    void render_loop(F f) const noexcept(noexcept(f())) {
      while (!should_close()) {
        auto start = std::chrono::steady_clock::now();
        f();
        swap_buffers();
        glfwPollEvents();
        auto elapsed = std::chrono::steady_clock::now() - start;
        constexpr auto max_framerate = std::chrono::milliseconds(1000) / 60;
        if (elapsed < max_framerate) {
          std::this_thread::sleep_for(max_framerate - elapsed);
        }
      }
    }

    inline void set_input_mode(cursor_mode mode) const {
      glfwSetInputMode(_window, GLFW_CURSOR, static_cast<int>(mode));
    }

    inline void set_input_mode(input_mode mode, bool enabled) const {
      glfwSetInputMode(
          _window, static_cast<int>(mode), enabled ? GLFW_TRUE : GLFW_FALSE);
    }

    [[nodiscard]] inline cursor_mode get_cursor_mode() const {
      return static_cast<cursor_mode>(glfwGetInputMode(_window, GLFW_CURSOR));
    }

    inline void framebuffer_size(width& w, height& h) const {
      glfwGetFramebufferSize(_window, &w.value, &h.value);
    }

    [[nodiscard]] framebuffer_dimension framebuffer_size() const {
      framebuffer_dimension fd;
      framebuffer_size(fd.width, fd.height);
      return fd;
    }

    inline void window_size(width& w, height& h) const {
      glfwGetWindowSize(_window, &w.value, &h.value);
    }

    [[nodiscard]] window_dimension window_size() const {
      window_dimension wd;
      window_size(wd.width, wd.height);
      return wd;
    }

   private:
    void _clean() {
      if (_window != nullptr) {
        glfwDestroyWindow(_window);
      }
    }

   protected:
    GLFWwindow* _window;
  };
};

template <class... Args>
class window_base
    : public dpsg::mixin<window_base<Args...>, Args..., window_impl> {
  using base = mixin<window_base<Args...>, Args..., window_impl>;

 public:
  explicit window_base(GLFWwindow* w) : base(w) {}
};

namespace detail {
template <class T>
T* get_window_ptr(GLFWwindow* w) {
  return static_cast<T*>(glfwGetWindowUserPointer(w));
}
}  // namespace detail
}  // namespace dpsg

#endif  // GUARD_DPSG_WINDOW_HEADER