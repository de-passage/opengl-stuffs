#define GLM_FORCE_SILENT_WARNINGS

#include "glad/glad.h"

#include "buffers.hpp"
#include "camera.hpp"
#include "common.hpp"
#include "glfw_context.hpp"
#include "glfw_controls.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "glm_traits.hpp"
#include "input/keys.hpp"
#include "input_timer.hpp"
#include "key_mapper.hpp"
#include "load_shaders.hpp"
#include "nk_glfw.hpp"
#include "nuklear/enums.hpp"
#include "nuklear/widgets.hpp"
#include "opengl.hpp"
#include "opengl/glm.hpp"
#include "program.hpp"
#include "utility.hpp"
#include "window.hpp"
#include "window/hints.hpp"
#include "window/mixins.hpp"
#include "with_window.hpp"

#include <chrono>
#include <iomanip>
#include <type_traits>

// clang-format off
constexpr float vertices[] = {
   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};
// clang-format on

template <template <class...> class T, class C>
struct is_template_instance : std::false_type {};

template <template <class...> class T, class... Args>
struct is_template_instance<T, T<Args...>> : std::true_type {};

template <template <class...> class T, class C>
constexpr static inline bool is_template_instance_v =
    is_template_instance<T, C>::value;

struct projection_program {
  template <class U,
            class T,
            std::enable_if_t<is_template_instance_v<dpsg::fs_filename, T> &&
                                 is_template_instance_v<dpsg::vs_filename, U>,
                             int> = 0>
  explicit projection_program(U&& vertex_shader, T&& fragment_shader)
      : _program{load(std::forward<U>(vertex_shader),
                      std::forward<T>(fragment_shader))
                     .value()},
        _projected_view{uniform_location<glm::mat4>("projected_view")},
        _model{uniform_location<glm::mat4>("model")} {}

  void project(const glm::mat4& projected_view) const noexcept {
    _projected_view.bind(projected_view);
  }

  void set_model(const glm::mat4& model) const noexcept { _model.bind(model); }

  template <class T>
  auto uniform_location(const char* name) const {
    return _program.uniform_location<T>(name).value();
  }

  template <class T>
  void use(const dpsg::camera<T>& cam, const glm::mat4& model) const noexcept {
    _program.use();
    project(cam.projected_view());
    set_model(model);
  }

 private:
  dpsg::program _program;
  dpsg::program::uniform<glm::mat4> _projected_view;
  dpsg::program::uniform<glm::mat4> _model;
};

struct object_program : projection_program {
  using projection_program::uniform_location;
  object_program()
      : projection_program{
            dpsg::vs_filename{"shaders/projection_with_normal.vs"},
            dpsg::fs_filename{"shaders/basic_lighting.fs"}} {}

  template <class T>
  void use(const dpsg::camera<T>& cam,
           const glm::vec3& object_color,
           const glm::vec3& light_color,
           float ambient,
           const glm::vec3& light_position,
           const glm::vec3& camera_position,
           float specular,
           uint8_t shininess) const noexcept {
    projection_program::use(cam, glm::mat4{1.0});
    _object_color_uniform.bind(object_color);
    _light_color_uniform.bind(light_color);
    _ambient_uniform.bind(ambient);
    _light_position_uniform.bind(light_position);
    _camera_position_uniform.bind(camera_position);
    _specular_uniform.bind(specular);
    _shininess_uniform.bind(1 << shininess);
  }

 private:
  dpsg::program::uniform<glm::vec3> _object_color_uniform{
      uniform_location<glm::vec3>("object_color")};
  dpsg::program::uniform<glm::vec3> _light_color_uniform{
      uniform_location<glm::vec3>("light_color")};
  dpsg::program::uniform<float> _ambient_uniform{
      uniform_location<float>("ambient")};
  dpsg::program::uniform<glm::vec3> _light_position_uniform{
      uniform_location<glm::vec3>("light_position")};
  dpsg::program::uniform<glm::vec3> _camera_position_uniform{
      uniform_location<glm::vec3>("camera_position")};
  dpsg::program::uniform<float> _specular_uniform{
      uniform_location<float>("specular")};
  dpsg::program::uniform<int> _shininess_uniform{
      uniform_location<int>("shininess")};
};

struct light_program : projection_program {
  using projection_program::uniform_location;
  light_program()
      : projection_program{dpsg::vs_filename{"shaders/basic_projection.vs"},
                           dpsg::fs_filename{"shaders/uniform.fs"}} {}

  template <class T>
  void use(const dpsg::camera<T>& cam,
           const glm::vec3& light_position,
           const glm::vec3& light_color) const noexcept {
    projection_program::use(
        cam,
        glm::scale(glm::translate(glm::mat4{1.0}, light_position),
                   glm::vec3{0.2}));
    _light_color_uniform.bind(glm::vec4{light_color, 1.0});
  }

 private:
  dpsg::program::uniform<glm::vec4> _light_color_uniform{
      uniform_location<glm::vec4>("ourColor")};
};

int main() {
  using namespace dpsg;
  using window_t = prepend_t<append_t<nk_glfw::window,
                                      framebuffer_size_cb,
                                      function_key_cb,
                                      scroll_cb,
                                      cursor_pos_cb>,
                             glfw_controls::mixin::key_mapper>;

  ExecutionStatus r = dpsg::ExecutionStatus::Failure;

  try {
    r = within_glfw_context([] {
      return with_window<window_t>(
          window_hint::context_version(3, 3),
          SCR_WIDTH,
          SCR_HEIGHT,
          title{"Lighting"},
          [](auto& wdw) {
            wdw.load_font("assets/fonts/ProggyClean.ttf", 14);
            camera<traits::glm> cam{SCR_WIDTH / SCR_HEIGHT};
            glfw_controls::bind_control_scheme(
                glfw_controls::free_camera, cam, wdw);

            wdw.set_framebuffer_size_callback(camera_resize(cam));

            wdw.on(input::key::space, [b = true, &cam](auto& window) mutable {
              if (b) {
                window.set_cursor_pos_callback(nullptr);
              }
              else {
                window.set_cursor_pos_callback(camera_tracks_cursor{cam});
              }
              b = !b;
            });
            wdw.on(input::key::escape, close);

            object_program object_program;

            light_program light_program;

            vertex_buffer cube;
            cube.bind();
            cube.set_data(vertices);

            // both VAOs use the cube VBO
            vertex_array object_vao;
            object_vao.bind();
            gl::vertex_attrib_pointer<float>(
                gl::attrib_location{0}, gl::element_count{3}, gl::stride{6});
            gl::vertex_attrib_pointer<float>(gl::attrib_location{1},
                                             gl::element_count{3},
                                             gl::stride{6},
                                             gl::offset{3});
            gl::enable_vertex_attrib_array(gl::attrib_location{0},
                                           gl::attrib_location{1});

            vertex_array light_vao;
            light_vao.bind();
            gl::vertex_attrib_pointer<float>(
                gl::attrib_location{0}, gl::element_count{3}, gl::stride{6});
            gl::enable_vertex_attrib_array(gl::attrib_location{0});

            gl::clear_color(gl::r{0.1}, gl::g{0.1}, gl::b{0.1});

            // Colors
            glm::vec3 light_position{1.2, 1.0, 2.0};
            glm::vec3 object_color{1.0, 0.5, 0.31};
            glm::vec3 light_color{1.0, 1.0, 1.0};
            float ambient{0.1};
            float specular{0.5};
            int shininess{5};
            wdw.on(input::key::T, ignore([&] {
                     std::cout << std::setprecision(3)
                               << "color: " << light_color.r << " "
                               << light_color.g << " " << light_color.b
                               << "\nambient: " << ambient << std::endl;
                   }));

            // Camera
            using namespace std::literals::chrono_literals;

            wdw.render_loop([&]([[maybe_unused]] nk::context& ctx) {
              gl::enable(gl::capability::depth_test);
              gl::disable(gl::capability::scissor_test);
              gl::clear(gl::buffer_bit::color | gl::buffer_bit::depth);

              ctx.with_window(
                  "Colors",
                  nk_rect(5, 5, 230, 235),
                  nk::panel_flags::no_scrollbar,
                  [&](nk::window w) {
                    namespace nkw = nk::widget;
                    w.row_dynamic(20, 2);
                    nkw::label(
                        w, "Object", nk_text_alignment::NK_TEXT_CENTERED);
                    nkw::label(w, "Light", nk_text_alignment::NK_TEXT_CENTERED);
                    const auto layout =
                        [&](float& otarget, float& ltarget, const char* label) {
                          w.with_row(NK_DYNAMIC, 33, 3, [&](nk::row r) {
                            r.push(0.1);
                            nkw::label(
                                w, label, nk_text_alignment::NK_TEXT_CENTERED);
                            r.push(0.45);
                            nkw::slider(w, 0, otarget, 1, 0.001);
                            r.push(0.45);
                            nkw::slider(w, 0, ltarget, 1, 0.001);
                          });
                        };
                    layout(object_color.r, light_color.r, "R");
                    layout(object_color.g, light_color.g, "G");
                    layout(object_color.b, light_color.b, "B");
                    w.row_dynamic(30, 2);
                    nkw::label(w, "Ambient");
                    nkw::slider(w, 0, ambient, 1, 0.001);
                    nkw::label(w, "Specular");
                    nkw::slider(w, 0, specular, 1, 0.001);
                    nkw::label(w, "Shininess");
                    nkw::slider(w, 1, shininess, 8, 1);
                  });

              object_program.use(cam,
                                 object_color,
                                 light_color,
                                 ambient,
                                 light_position,
                                 cam.position(),
                                 specular,
                                 shininess);
              object_vao.bind();
              gl::draw_arrays(gl::drawing_mode::triangles,
                              gl::index{0},
                              gl::element_count{36});

              light_program.use(cam, light_position, light_color);

              object_vao.bind();
              gl::draw_arrays(gl::drawing_mode::triangles,
                              gl::index{0},
                              gl::element_count{36});
            });
          });
    });
  }
  catch (std::exception& e) {
    std::cout << "Exception caught at top level:\n" << e.what() << std::endl;
  }
  return static_cast<int>(r);
}