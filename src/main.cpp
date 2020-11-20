#include <exception>
#include <glad/glad.h>

#include "buffers.hpp"
#include "glfw_context.hpp"
#include "input/keys.hpp"
#include "shaders.hpp"
#include "utility.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

// settings
constexpr dpsg::window::width SCR_WIDTH{800};
constexpr dpsg::window::height SCR_HEIGHT{600};

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(dpsg::window &window);

template <class F> dpsg::ExecutionStatus make_window(F f) {
  using namespace dpsg;

  return within_glfw_context([f]() -> dpsg::ExecutionStatus {
    using wh = window_hint;

    return with_window(
        wh::context_version(3, 3), wh::opengl_profile(profile::core),
#ifdef __APPLE__
        wh::opengl_forward_compat(true),
#endif
        SCR_WIDTH, SCR_HEIGHT, window::title{"LearnOpenGL"},
        [f](window &wdw) -> ExecutionStatus {
          // glfw window creation
          // --------------------
          wdw.make_context_current();
          wdw.set_framebuffer_size_callback(framebuffer_size_callback);

          // glad: load all OpenGL function pointers
          // ---------------------------------------
          if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
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

float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};
const char *vertex_shader_source =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragment_shader_source = "#version 330 core\n"
                                     "out vec4 fragColor;\n"
                                     "void main() {"
                                     "fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);"
                                     "}";

int main() {

  dpsg::ExecutionStatus r = dpsg::ExecutionStatus::Failure;
  try {
    r = make_window([](dpsg::window &wdw) {
      auto vvar = dpsg::vertex_shader::create(vertex_shader_source);
      if (auto *verror = std::get_if<dpsg::shader_error>(&vvar)) {
        throw std::runtime_error{verror->what()};
      }
      auto vshader = std::get<dpsg::vertex_shader>(std::move(vvar));

      auto fvar = dpsg::fragment_shader::create(fragment_shader_source);
      if (auto *ferror = std::get_if<dpsg::shader_error>(&fvar)) {
        throw std::runtime_error{ferror->what()};
      }
      auto fshader = std::get<dpsg::fragment_shader>(std::move(fvar));

      auto pvar = dpsg::program::create(vshader, fshader);
      if (auto *perror = std::get_if<dpsg::program_error>(&pvar)) {
        throw std::runtime_error{perror->what()};
      }
      auto shader_program = std::get<dpsg::program>(std::move(pvar));

      dpsg::buffer vbo;
      unsigned int vao;
      glGenVertexArrays(1, &vao);

      glBindVertexArray(vao);

      glBindBuffer(GL_ARRAY_BUFFER, static_cast<unsigned int>(vbo));
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                            (void *)0);
      glEnableVertexAttribArray(0);

      wdw.render_loop([&] {
        processInput(wdw);
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader_program();
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
      });
    });

  } catch (std::exception &e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unhandled error occured" << std::endl;
  }

  return static_cast<int>(r);
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(dpsg::window &window) {
  if (window.get_key(dpsg::input::key::escape) ==
      dpsg::input::status::pressed) {
    window.should_close(true);
  }
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