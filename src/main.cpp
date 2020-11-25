#include <glad/glad.h>

#include "buffers.hpp"
#include "glfw_context.hpp"
#include "input/keys.hpp"
#include "key_mapper.hpp"
#include "load_shaders.hpp"
#include "shaders.hpp"
#include "texture.hpp"
#include "utility.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

// settings
constexpr dpsg::width SCR_WIDTH{800};
constexpr dpsg::height SCR_HEIGHT{600};

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
        SCR_WIDTH, SCR_HEIGHT, title{"LearnOpenGL"},
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

float vertices[] = {
    0.f,    0.f, 0.f,   0.5f, -0.5f,  0.25f, -0.5f,
    -0.25f, 0.f, -0.5f, 0.5f, -0.25f, 0.5f,  0.25f,
};

unsigned int indices[] = {0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1};

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
      GL_TRIANGLES_ADJACENCY,
  };

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

enum class drawing_mode {
  points = GL_POINTS,
  line_strip = GL_LINE_STRIP,
  line_loop = GL_LINE_LOOP,
  lines = GL_LINES,
  line_strip_adjacency = GL_LINE_STRIP_ADJACENCY,
  lines_adjacency = GL_LINES_ADJACENCY,
  triangle_strip = GL_TRIANGLE_STRIP,
  triangle_fan = GL_TRIANGLE_FAN,
  triangles = GL_TRIANGLES,
  triangle_strip_adjacency = GL_TRIANGLE_STRIP_ADJACENCY,
  triangles_adjacency = GL_TRIANGLES_ADJACENCY,
};

template <std::size_t S, std::size_t N, class T = float>
struct vertex_array_renderer {
  // NOLINTNEXTLINE
  vertex_array_renderer(T (&arr)[S * N]) {
    vao.bind();
    vbo.bind(dpsg::buffer_type::array);
    glBufferData(GL_ARRAY_BUFFER, N * S * sizeof(T), static_cast<void *>(arr),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, S, GL_FLOAT, GL_FALSE, S * sizeof(T),
                          static_cast<void *>(0));
    glEnableVertexAttribArray(0); // Needs to be in its own function?
  }

  vertex_array_renderer(const std::array<T, N * S> &arr) {
    vao.bind();
    vbo.bind(dpsg::buffer_type::array);
    glBufferData(GL_ARRAY_BUFFER, N * S * sizeof(T), arr.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, S, GL_FLOAT, GL_FALSE, S * sizeof(T),
                          static_cast<void *>(0));
    glEnableVertexAttribArray(0); // Needs to be in its own function?
  }

  void render(dpsg::program &prog, drawing_mode dm) const {
    prog();
    render(dm);
  }
  void render(drawing_mode dm = drawing_mode::triangles) const {
    vao.bind();
    glDrawArrays(static_cast<int>(dm), 0, N);
  }

private:
  dpsg::buffer vbo;
  dpsg::vertex_array vao;
};

template <std::size_t Dimensions, std::size_t Vertices, std::size_t Elements>
struct element_renderer {
  template <class T, class U> element_renderer() {}

private:
  dpsg::buffer vbo;
  dpsg::buffer ebo;
  dpsg::vertex_array vao;
};

namespace opengl {}

int main() {
  using namespace dpsg;
  using namespace dpsg::input;

  dpsg::ExecutionStatus r = dpsg::ExecutionStatus::Failure;

  try {
    r = make_window([](dpsg::window &wdw) {
      auto prog = load(vs_filename{"shaders/textured.vs"},
                       fs_filename{"shaders/textured.fs"});
      auto wallText = load(texture_filename{"assets/wall.jpg"}).value();
      key_mapper kmap;
      wdw.set_key_callback(window::key_callback{std::ref(kmap)});

      constexpr auto exit = [](window &w) { w.should_close(true); };
      constexpr auto ignore = [](auto f) {
        return [f = std::move(f)]([[maybe_unused]] window &w) { f(); };
      };
      kmap.on(key::escape, exit);

      float vertices[] = {
          // positions        // colors         // texture coords
          0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
          0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
          -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
          -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
      };

      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      wdw.render_loop([&] {
        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT);
      });
    });

  } catch (std::exception &e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unhandled error occured" << std::endl;
  }

  return static_cast<int>(r);
}
