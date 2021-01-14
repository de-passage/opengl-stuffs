#ifndef GUARD_MAKE_WINDOW_HEADER
#define GUARD_MAKE_WINDOW_HEADER

#include "glad/glad.h"

#include "glfw_context.hpp"
#include "input_timer.hpp"
#include "key_mapper.hpp"
#include "meta/mixin.hpp"
#include "utility.hpp"
#include "utils.hpp"
#include "with_window.hpp"

#include <iostream>
#include <type_traits>
// settings
constexpr static inline dpsg::width SCR_WIDTH{800};
constexpr static inline dpsg::height SCR_HEIGHT{600};

namespace mixin {
struct key_mapper {
  template <class B>
  struct type : B {
    using base = B;

   private:
    using kmap_t = basic_key_mapper<dpsg::real_type_t<B>>;
    constexpr static auto interval = std::chrono::milliseconds(10);
    kmap_t kmap;
    input_timer<std::function<void()>> timer;

   public:
    using key_pressed_callback = typename kmap_t::key_pressed_callback;
    template <class... Args>
    constexpr explicit type(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<base, Args...>)
        : base{std::forward<Args>(args)...},
          timer{[this] {
                  kmap.trigger_pressed_callbacks(
                      *static_cast<dpsg::real_type_t<B>*>(this));
                },
                interval} {
      base::set_key_callback(std::ref(kmap));
    }

    inline void on(dpsg::input::key k, key_pressed_callback cb) {
      kmap.on(k, std::move(cb));
    }

    inline void while_(dpsg::input::key k, key_pressed_callback cb) {
      kmap.while_(k, std::move(cb));
    }

    template <class F>
    inline auto render_loop(F&& f) noexcept(
        noexcept(base::render_loop(std::forward<F>(f)))) {
      base::render_loop([this, &f](auto&&... args) {
        std::forward<F>(f)(std::forward<decltype(args)>(args)...);
        timer.trigger();
        kmap.trigger_pressed_callbacks(
            *static_cast<dpsg::real_type_t<B>*>(this));
      });
    }
  };
};

}  // namespace mixin

namespace detail {
template <class T>
constexpr static inline bool uses_kmap =
    std::is_invocable_v<T, dpsg::window&, key_mapper&>;
}  // namespace detail

template <class F, std::enable_if_t<detail::uses_kmap<F>, int> = 0>
void invoke(F&& f, dpsg::window& window, key_mapper& kmap) {
  std::forward<F>(f)(window, kmap);
}

template <class F, std::enable_if_t<!detail::uses_kmap<F>, int> = 0>
void invoke(F&& f, dpsg::window& window, [[maybe_unused]] key_mapper& unused) {
  std::forward<F>(f)(window);
}

using kmap_window = dpsg::base_window<glad_loader,
                                      ::mixin::key_mapper,
                                      dpsg::function_key_cb,
                                      dpsg::framebuffer_size_cb,
                                      dpsg::scroll_cb,
                                      dpsg::cursor_pos_cb>;

namespace detail {

constexpr static inline auto default_window_type = [](auto&& f) {
  using F = decltype(f);
  using namespace dpsg;
  using namespace dpsg::input;
  return [&f](window& wdw) -> ExecutionStatus {
    // glfw window creation
    // --------------------
    wdw.make_context_current();
    wdw.set_framebuffer_size_callback(resize);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(  // NOLINT
            glfwGetProcAddress))) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return dpsg::ExecutionStatus::Failure;
    }

    key_mapper kmap;
    wdw.set_key_callback(std::ref(kmap));
    kmap.on(key::escape, close);
    invoke(std::forward<F>(f), wdw, kmap);
    return ExecutionStatus::Success;
  };
};

constexpr static inline auto kmap_window = [](auto&& f) {
  using F = decltype(f);
  return [&f](::kmap_window& wdw) {
    wdw.set_framebuffer_size_callback(resize);
    std::forward<F>(f)(wdw);
    return dpsg::ExecutionStatus::Success;
  };
};
}  // namespace detail

template <class F>
dpsg::ExecutionStatus make_window(F&& f) {
  using namespace dpsg;
  return within_glfw_context([&f]() -> dpsg::ExecutionStatus {
    using wh = window_hint;
    glfwSwapInterval(1);

    constexpr auto window_cb = [] {
      if constexpr (::detail::uses_kmap<F> ||
                    std::is_invocable_v<F, dpsg::window&>) {
        return ::detail::default_window_type;
      }
      else if constexpr (std::is_invocable_v<F, kmap_window&>) {
        return ::detail::kmap_window;
      }
    }();
    using window_type = std::conditional_t<std::is_invocable_v<F, kmap_window&>,
                                           kmap_window,
                                           dpsg::window>;

    return with_window<window_type>(wh::context_version(3, 3),
                                    wh::opengl_profile(profile::core),
#ifdef __APPLE__
                                    wh::opengl_forward_compat(true),
#endif
                                    SCR_WIDTH,
                                    SCR_HEIGHT,
                                    title{"OpenGL example"},
                                    window_cb(std::forward<F>(f)));
  });
}

template <class F>
int windowed(F&& func) noexcept {
  using namespace dpsg;

  ExecutionStatus r = ExecutionStatus::Failure;

  try {
    r = make_window(std::forward<F>(func));
  }
  catch (std::exception& e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "Unhandled error occured" << std::endl;
  }

  return static_cast<int>(r);
}

#endif  // GUARD_MAKE_WINDOW_HEADER