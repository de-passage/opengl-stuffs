#include <exception>
#include <glad/glad.h>

#include "buffers.hpp"
#include "glfw_context.hpp"
#include "input/keys.hpp"
#include "key_mapper.hpp"
#include "shaders.hpp"
#include "utility.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

// settings
constexpr dpsg::window::width SCR_WIDTH{800};
constexpr dpsg::window::height SCR_HEIGHT{600};

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

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

float vertices[] = {0.f,   0.f,  0.f,    0.f,    0.5f, 0.f,   -0.5f,
                    0.25f, 0.f,  -0.5f,  -0.25f, 0.f,  0.f,   -0.5f,
                    0.f,   0.5f, -0.25f, 0.f,    0.5f, 0.25f, 0.f};

unsigned int indices[] = {0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1};

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
  using namespace dpsg;
  using namespace dpsg::input;

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

      key_mapper kmap;
      wdw.set_key_callback(window::key_callback{std::ref(kmap)});

      const auto exit = [](window &w) { w.should_close(true); };
      kmap.on(key::I, exit);
      kmap.on(key::escape, exit);
      kmap.on(key::X, []([[maybe_unused]] window &w) {
        static bool b = true;
        b = !b;
        if (!b) {
          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
      });

      dpsg::buffer vbo, ebo;
      unsigned int vao;
      glGenVertexArrays(1, &vao);

      glBindVertexArray(vao);

      glBindBuffer(GL_ARRAY_BUFFER, vbo.id());
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                   static_cast<void *>(vertices), GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.id());
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
                   static_cast<void *>(indices), GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                            (void *)0);
      glEnableVertexAttribArray(0);

      wdw.render_loop([&] {
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader_program();

        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.id());
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
        // glDrawArrays(GL_LINES, 0, 7);
      });
    });

  } catch (std::exception &e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unhandled error occured" << std::endl;
  }

  return static_cast<int>(r);
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