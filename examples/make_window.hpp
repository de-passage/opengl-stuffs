#ifndef GUARD_MAKE_WINDOW_HEADER
#define GUARD_MAKE_WINDOW_HEADER

#include "glad/glad.h"

#include "glfw_context.hpp"
#include "window.hpp"

#include <iostream>

// settings
constexpr static inline dpsg::width SCR_WIDTH{800};
constexpr static inline dpsg::height SCR_HEIGHT{600};

template <class F> dpsg::ExecutionStatus make_window(F f) {
  using namespace dpsg;

  return within_glfw_context([f = std::move(f)]() -> dpsg::ExecutionStatus {
    using wh = window_hint;
    glfwSwapInterval(1);

    return with_window(
        wh::context_version(3, 3), wh::opengl_profile(profile::core),
#ifdef __APPLE__
        wh::opengl_forward_compat(true),
#endif
        SCR_WIDTH, SCR_HEIGHT, title{"OpenGL example"},
        [f](window &wdw) -> ExecutionStatus {
          // glfw window creation
          // --------------------
          wdw.make_context_current();
          wdw.set_framebuffer_size_callback(
              []([[maybe_unused]] dpsg::window &unused, dpsg::width w,
                 dpsg::height h) { glViewport(0, 0, w.value, h.value); });

          // glad: load all OpenGL function pointers
          // ---------------------------------------
          if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>( // NOLINT
                  glfwGetProcAddress))) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return dpsg::ExecutionStatus::Failure;
          }

          // render loop
          // -----------
          f(wdw);
          return ExecutionStatus::Success;
        });
  });
}

template <class F> int windowed(F &&func) {
  using namespace dpsg;

  ExecutionStatus r = ExecutionStatus::Failure;

  try {
    r = make_window(std::forward<F>(func));

  } catch (std::exception &e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unhandled error occured" << std::endl;
  }

  return static_cast<int>(r);
}

#endif // GUARD_MAKE_WINDOW_HEADER