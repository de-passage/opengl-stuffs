#include <glad/glad.h>

#include "glfw_context.hpp"
#include "input/keys.hpp"
#include "utility.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdlib.h>
#include <string_view>
#include <type_traits>
#include <utility>

// settings
constexpr dpsg::window::width SCR_WIDTH{800};
constexpr dpsg::window::height SCR_HEIGHT{600};

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(dpsg::window &window);

int main() {
  using namespace dpsg;

  auto r = within_glfw_context([]() -> dpsg::ExecutionStatus {
    using wh = window_hint;

    return with_window(
        wh::context_version(3, 3), wh::opengl_profile(profile::core),

#ifdef __APPLE__
        wh::opengl_forward_compat(true),
#endif
        SCR_WIDTH, SCR_HEIGHT, window::title{"LearnOpenGL"},
        [](window &wdw) -> ExecutionStatus {
          // glad: load all OpenGL function pointers
          // ---------------------------------------
          if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return dpsg::ExecutionStatus::Failure;
          }
          // glfw window creation
          // --------------------
          wdw.make_context_current();
          wdw.set_framebuffer_size_callback(framebuffer_size_callback);

          // render loop
          // -----------
          while (!wdw.should_close()) {
            // input
            // -----
            processInput(wdw);

            // render
            // ------
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // glfw: swap buffers and poll IO events (keys
            // pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            wdw.swap_buffers();
            glfwPollEvents();
          }
          return ExecutionStatus::Success;
        });
  });

  return static_cast<int>(r);
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(dpsg::window &window) {
  if (window.get_key(dpsg::input::key::escape) == dpsg::input::status::pressed)
    window.should_close(true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback([[maybe_unused]] GLFWwindow *window, int width,
                               int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}