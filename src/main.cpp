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
    glfwSwapInterval(1);

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

float vertices[] = {
    0.f,    0.f, 0.f,   0.5f, -0.5f,  0.25f, -0.5f,
    -0.25f, 0.f, -0.5f, 0.5f, -0.25f, 0.5f,  0.25f,
};

unsigned int indices[] = {0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1};

const char *vertex_shader_source = "#version 330 core\n"
                                   "layout location = 0) in vec2 aPos;\n"
                                   "out vec4 vertexColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   gl_Position = vec4(aPos, 1.0, 1.0);\n"
                                   "   vertexColor = vec4(0.5, 0.0, 0.0, 1.0);"
                                   "}";

const char *fragment_shader_source = "#version 330 core\n"
                                     "out vec4 fragColor;\n"
                                     "in vec4 vertexColor;"
                                     "void main() {"
                                     "fragColor = vertexColor;"
                                     "}";

const char *yellow_fragment_shader_source =
    "#version 330 core\n"
    "out vec4 fragColor;\n"
    "void main() {"
    "fragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);"
    "}";

const char *lerp_fshader =
    "#version 330 core\n"
    "out vec4 outputColor;\n"
    "void main() {"
    "  float lerpValue = gl_FragCoord.y / 500.0f;"
    "  outputColor = mix(vec4(1.0f, 1.0f, 1.0f, 1.0f),"
    "                     vec4(0.2f, 0.2f, 0.2f, 1.0f), lerpValue);"
    "}";

const char *uniform_fshader = "#version 330 core\n"
                              "out vec4 outputColor;"
                              "uniform vec4 ourColor;"
                              "void main() {"
                              "  outputColor = ourColor;"
                              "}";

struct drawing_modes {
  static constexpr inline int gl_acceptable_values[11] = {
      GL_POINTS,
      GL_LINE_STRIP,
      GL_LINE_LOOP,
      GL_LINES,
      GL_LINE_STRIP_ADJACENCY,
      GL_LINES_ADJACENCY,
      GL_TRIANGLE_STRIP,
      GL_TRIANGLE_FAN,
      GL_TRIANGLES,
      GL_TRIANGLE_STRIP_ADJACENCY,
      GL_TRIANGLES_ADJACENCY};

  void rotate() {
    ++_current_value;
    if (_current_value >= sizeof(gl_acceptable_values) / sizeof(int)) {
      _current_value = 0;
    }
  }

  int current_value() const { return gl_acceptable_values[_current_value]; }

private:
  std::size_t _current_value = 7;
};

struct element_drawer : drawing_modes {
  element_drawer(int vcount, int icount)
      : vertice_count{vcount}, indice_count{icount} {}

  void operator()() {
    if (_draw_elem) {

      glDrawElements(current_value(), indice_count, GL_UNSIGNED_INT,
                     static_cast<void *>(0));
    } else {
      glDrawArrays(current_value(), 0, vertice_count);
    }
  }

  void switch_render_func() { _draw_elem = !_draw_elem; }

private:
  bool _draw_elem = true;
  int vertice_count;
  int indice_count;
};

template <class V, class R = std::variant_alternative_t<0, V>>
R get_or_throw(V &&v) {
  if (auto *err = std::get_if<std::variant_alternative_t<1, V>>(&v)) {
    throw *err;
  }
  return std::get<R>(std::forward<V>(v));
}

int main() {
  using namespace dpsg;
  using namespace dpsg::input;

  dpsg::ExecutionStatus r = dpsg::ExecutionStatus::Failure;

  try {
    r = make_window([](dpsg::window &wdw) {
      auto vshader = get_or_throw(vertex_shader::create(vertex_shader_source));
      auto fshader =
          get_or_throw(fragment_shader::create(fragment_shader_source));
      auto shader_program = get_or_throw(program::create(vshader, fshader));
      auto yfshader = get_or_throw(fragment_shader::create(lerp_fshader));
      auto infshader = get_or_throw(fragment_shader::create(uniform_fshader));
      auto yprogram = get_or_throw(program::create(vshader, infshader));

      element_drawer elem_d{7, 18};

      key_mapper kmap;
      wdw.set_key_callback(window::key_callback{std::ref(kmap)});

      float r = 1.0f, g = 0.0f, b = 1.0f, a = 1.0f;
      const auto exit = [](window &w) { w.should_close(true); };
      const auto ignore = [](auto f) {
        return [f = std::move(f)]([[maybe_unused]] window &w) { f(); };
      };
      const auto switch_f = [&](float &f) {
        return ignore([&f] { f = 1.0 - f; });
      };
      kmap.on(key::A, switch_f(a));
      kmap.on(key::R, switch_f(r));
      kmap.on(key::G, switch_f(g));
      kmap.on(key::B, switch_f(b));
      kmap.on(key::I, ignore([&elem_d] { elem_d.switch_render_func(); }));
      kmap.on(key::escape, exit);
      kmap.on(key::X, ignore([] {
                static bool b = true;
                b = !b;
                if (!b) {
                  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                } else {
                  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                }
              }));
      kmap.on(key::E, ignore([&elem_d] { elem_d.rotate(); }));

      dpsg::buffer vbo, ebo;
      vertex_array vao;

      vao.bind();

      vbo.bind(buffer_type::array);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
                   static_cast<void *>(vertices), GL_STATIC_DRAW);

      ebo.bind(buffer_type::element_array);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
                   static_cast<void *>(indices), GL_STATIC_DRAW);

      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                            (void *)0);
      glEnableVertexAttribArray(0);

      float v2s[] = {
          -1.0f, 0.3f, -0.3f, 1.0f, -1.0f, 1.0f,
      };
      dpsg::buffer vbo2;
      vertex_array vao2;
      vao2.bind();
      vbo2.bind(buffer_type::array);
      glBufferData(GL_ARRAY_BUFFER, sizeof(v2s), v2s, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                            static_cast<void *>(0));
      glEnableVertexAttribArray(0);

      auto unifPos = yprogram.uniform_location("ourColor").value();

      wdw.render_loop([&] {
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader_program();

        glBindVertexArray(vao.id());
        elem_d();

        yprogram();
        unifPos.bind(r, g, b, a);
        glBindVertexArray(vao2.id());
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

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback([[maybe_unused]] GLFWwindow *window, int width,
                               int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}