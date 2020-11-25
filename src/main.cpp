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

  vertex_array_renderer(const std::array<T, N> &arr)
      : vertex_array_renderer(arr.data()) {}

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

int main() {
  using namespace dpsg;
  using namespace dpsg::input;

  dpsg::ExecutionStatus r = dpsg::ExecutionStatus::Failure;

  try {
    r = make_window([](dpsg::window &wdw) {
      auto fragment_shader_source =
          load_from_disk(fs_filename{"shaders/basic.fs"});
      auto vertex_shader_source =
          load_from_disk(vs_filename{"shaders/basic.vs"});
      auto uniform_fshader = load_from_disk(fs_filename{"shaders/uniform.fs"});
      auto yellow_shader = load_from_disk(fs_filename{"shaders/yellow.fs"});
      auto positionable_vertices =
          load_from_disk(vs_filename{"shaders/positionable_triangle.vs"});
      auto lerp_fshader = load_from_disk(fs_filename{"shaders/lerp.fs"});
      auto shader_program =
          create_program(vertex_shader_source, fragment_shader_source);
      auto yprogram = create_program(vertex_shader_source, uniform_fshader);
      auto posProg = create_program(positionable_vertices, lerp_fshader);
      auto wall_texture =
          load_from_disk(texture_filename{"assets/container.jpg"}).value();

      element_drawer elem_d{7, 18};

      key_mapper kmap;
      wdw.set_key_callback(window::key_callback{std::ref(kmap)});

      float r = 1.0f, g = 0.0f, b = 1.0f, a = 1.0f, x_offset = 0.f,
            y_offset = 0.0f;
      constexpr auto exit = [](window &w) { w.should_close(true); };
      constexpr auto ignore = [](auto f) {
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

      constexpr auto move_axis = [ignore](float &target) {
        return [&](float of) {
          return ignore([&target, of] {
            target += of;
            std::cout << "value changed: " << target << std::endl;
          });
        };
      };
      const auto move_y = move_axis(y_offset);
      const auto move_x = move_axis(x_offset);
      constexpr float std_offset = 0.01f;
      kmap.on(key::left, move_x(-std_offset));
      kmap.on(key::right, move_x(+std_offset));
      kmap.on(key::up, move_y(+std_offset));
      kmap.on(key::down, move_y(-std_offset));

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

      float v3s[] = {0.6f, 0.6f, 0.5f, 0.55f, 0.7f, 0.5f};
      vertex_array_renderer<2, 3> var(v3s);

      float v2s[] = {
          -1.0f, 0.3f, -0.3f, 1.0f, -1.0f, 1.0f,
      };
      vertex_array_renderer<2, 3> var2(v2s);

      auto unifPos =
          yprogram.uniform_location<vec<4, float>>("ourColor").value();
      auto xy_unif =
          posProg.uniform_location<vec<2, float>>("xyOffset").value();

      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      wdw.render_loop([&] {
        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT);

        shader_program();
        glBindVertexArray(vao.id());
        elem_d();

        yprogram();
        unifPos.bind(r, g, b, a);
        var2.render();

        posProg();
        xy_unif.bind(x_offset, y_offset);
        var.render();
      });
    });

  } catch (std::exception &e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unhandled error occured" << std::endl;
  }

  return static_cast<int>(r);
}
