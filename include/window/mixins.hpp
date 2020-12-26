#ifndef GUARD_DPSG_WINDOW_MIXINS_HEADER
#define GUARD_DPSG_WINDOW_MIXINS_HEADER

#include "common.hpp"
#include "input/keys.hpp"
#include "meta/mixin.hpp"

#include <functional>

namespace dpsg {

struct function_key_cb {
  template <class B>
  struct type : B {
    using signature =
        void(real_type_t<B>&, input::key, int, input::status, int);
    using key_callback = std::function<signature>;

    template <class... Args>
    constexpr explicit type(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<B, Args...>)
        : B(std::forward<Args>(args)...) {}

   private:
    static void call(GLFWwindow* w,
                     int key,
                     int scancode,
                     int action,
                     int mods) {
      auto* ptr = detail::get_window_ptr<type>(w);
      if (auto& f = ptr->_cb) {
        f(*static_cast<real_type_t<B>*>(ptr),
          static_cast<input::key>(key),
          static_cast<int>(scancode),
          static_cast<input::status>(action),
          static_cast<int>(mods));
      }
    }

   public:
    key_callback set_key_callback(key_callback f) {
      if (f) {
        glfwSetKeyCallback(B::_window, &call);
        return std::exchange(_cb, std::move(f));
      }
      else {
        glfwSetKeyCallback(B::_window, nullptr);
        return std::exchange(_cb, nullptr);
      }
    }

   private:
    key_callback _cb;
  };
};

struct framebuffer_size_cb {
  template <class B>
  struct type : B {
    using signature = void(real_type_t<B>&, width, height);
    using framebuffer_size_callback = std::function<signature>;

    template <class... Args>
    constexpr explicit type(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<B, Args...>)
        : B(std::forward<Args>(args)...) {}

    framebuffer_size_callback set_framebuffer_size_callback(
        framebuffer_size_callback&& f) {
      if (f != nullptr) {
        glfwSetFramebufferSizeCallback(B::_window, &call);
      }
      else {
        glfwSetFramebufferSizeCallback(B::_window, nullptr);
      }
      return std::exchange(_cb, f);
    }

   private:
    static void call(GLFWwindow* wdw, int w, int h) {
      auto* ptr = detail::get_window_ptr<type>(wdw);
      if (auto& f = ptr->_cb) {
        f(static_cast<real_type_t<B>&>(*ptr), width{w}, height{h});
      }
    }

    framebuffer_size_callback _cb;
  };
};

struct cursor_pos_cb {
  template <class B>
  struct type : B {
    using signature = void(real_type_t<B>&, double, double);
    using cursor_pos_callback = std::function<signature>;

    template <class... Args>
    constexpr explicit type(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<B, Args...>)
        : B(std::forward<Args>(args)...) {}

    cursor_pos_callback set_cursor_pos_callback(cursor_pos_callback&& f) {
      if (f != nullptr) {
        glfwSetCursorPosCallback(B::_window, &call);
      }
      else {
        glfwSetCursorPosCallback(B::_window, nullptr);
      }
      return std::exchange(_cb, f);
    }

   private:
    static void call(GLFWwindow* wdw, double x, double y) {
      auto* ptr = detail::get_window_ptr<type>(wdw);
      if (auto& f = ptr->_cb) {
        f(static_cast<real_type_t<B>&>(*ptr), x, y);
      }
    }

    cursor_pos_callback _cb;
  };
};

struct scroll_cb {
  template <class B>
  struct type : B {
    using signature = void(real_type_t<B>&, double, double);
    using scroll_callback = std::function<signature>;

    template <class... Args>
    constexpr explicit type(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<B, Args...>)
        : B(std::forward<Args>(args)...) {}

    scroll_callback set_scroll_callback(scroll_callback&& f) {
      if (f != nullptr) {
        glfwSetScrollCallback(B::_window, &call);
      }
      else {
        glfwSetScrollCallback(B::_window, nullptr);
      }
      return std::exchange(_cb, f);
    }

   private:
    static void call(GLFWwindow* wdw, double x, double y) {
      auto* ptr = detail::get_window_ptr<type>(wdw);
      if (auto& f = ptr->_cb) {
        f(static_cast<real_type_t<B>&>(*ptr), x, y);
      }
    }

    scroll_callback _cb;
  };
};

}  // namespace dpsg

#endif  // GUARD_DPSG_WINDOW_MIXINS_HEADER