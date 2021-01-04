#define GLM_FORCE_SILENT_WARNINGS

#include "glad/glad.h"

#include "buffers.hpp"
#include "camera.hpp"
#include "common.hpp"
#include "glfw_context.hpp"
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
           const glm::vec3& light_position) const noexcept {
    projection_program::use(cam, glm::mat4{1.0});
    _object_color_uniform.bind(object_color);
    _light_color_uniform.bind(light_color);
    _ambient_uniform.bind(ambient);
    _light_position_uniform.bind(light_position);
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
  using window_t = append_t<nk_glfw::window,
                            framebuffer_size_cb,
                            function_key_cb,
                            cursor_pos_cb>;

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
            basic_key_mapper<window_t> kmap;

            wdw.set_key_callback(std::ref(kmap));
            wdw.set_framebuffer_size_callback(camera_resize(cam));
            wdw.set_cursor_pos_callback(camera_tracks_cursor{cam});

            kmap.on(input::key::space, [b = true, &cam](auto& window) mutable {
              if (b) {
                window.set_cursor_pos_callback(nullptr);
              }
              else {
                window.set_cursor_pos_callback(camera_tracks_cursor{cam});
              }
              b = !b;
            });
            kmap.on(input::key::escape, close);

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
            gl::vertex_attrib_pointer<float>(
                gl::attrib_location{1}, gl::element_count{3}, gl::stride{6});
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
            kmap.on(input::key::T, ignore([&] {
                      std::cout << std::setprecision(3)
                                << "color: " << light_color.r << " "
                                << light_color.g << " " << light_color.b
                                << "\nambient: " << ambient << std::endl;
                    }));

            // Camera
            using namespace std::literals::chrono_literals;
            constexpr radians default_yaw{to_radians(degrees{-90})};
            constexpr radians default_pitch{0};
            constexpr radians default_fov{to_radians(degrees{45})};
            constexpr auto interval = 10ms;
            constexpr float camera_speed = .04;
            input_timer timer{[&] { kmap.trigger_pressed_callbacks(wdw); },
                              interval};

            const auto move_forward =
                ignore([&] { cam.advance(camera_speed); });
            const auto move_backward =
                ignore([&] { cam.advance(-camera_speed); });
            const auto strafe_left = ignore([&] { cam.strafe(-camera_speed); });
            const auto strafe_right = ignore([&] { cam.strafe(camera_speed); });
            const auto climb_up = ignore([&] { cam.climb(camera_speed); });
            const auto climb_down = ignore([&] { cam.climb(-camera_speed); });
            const auto rotate_left =
                ignore([&] { cam.rotate(camera_speed, 0); });
            const auto rotate_right =
                ignore([&] { cam.rotate(-camera_speed, 0); });

            kmap.while_(input::key::up, move_forward);
            kmap.while_(input::key::W, move_forward);
            kmap.while_(input::key::down, move_backward);
            kmap.while_(input::key::S, move_backward);
            kmap.while_(input::key::left, strafe_left);
            kmap.while_(input::key::A, strafe_left);
            kmap.while_(input::key::right, strafe_right);
            kmap.while_(input::key::D, strafe_right);
            kmap.while_(input::key::R, climb_up);
            kmap.while_(input::key::F, climb_down);
            kmap.while_(input::key::Q, rotate_left);
            kmap.while_(input::key::E, rotate_right);

            kmap.on(input::key::G, ignore([&] {
                      cam.reset(default_yaw, default_pitch, default_fov);
                    }));

            wdw.render_loop([&]([[maybe_unused]] nk::context& ctx) {
              gl::enable(gl::capability::depth_test);
              gl::disable(gl::capability::scissor_test);
              gl::clear(gl::buffer_bit::color | gl::buffer_bit::depth);

              ctx.with_window(
                  "Colors",
                  nk_rect(5, 5, 200, 165),
                  nk::panel_flags::no_scrollbar,
                  [&](nk::window w) {
                    namespace nkw = nk::widget;
                    w.row_dynamic(20, 2);
                    nkw::label(
                        w, "Object", nk_text_alignment::NK_TEXT_CENTERED);
                    nkw::label(w, "Light", nk_text_alignment::NK_TEXT_CENTERED);
                    w.row_dynamic(33, 2);
                    nkw::slider(w, 0, object_color.r, 1, 0.001);
                    nkw::slider(w, 0, light_color.r, 1, 0.001);
                    w.row_dynamic(33, 2);
                    nkw::slider(w, 0, object_color.g, 1, 0.001);
                    nkw::slider(w, 0, light_color.g, 1, 0.001);
                    w.row_dynamic(33, 2);
                    nkw::slider(w, 0, object_color.b, 1, 0.001);
                    nkw::slider(w, 0, light_color.b, 1, 0.001);
                    w.row_dynamic(30, 2);
                    nkw::label(w, "Ambient");
                    nkw::slider(w, 0, ambient, 1, 0.001);
                  });

              object_program.use(
                  cam, object_color, light_color, ambient, light_position);
              object_vao.bind();
              gl::draw_arrays(gl::drawing_mode::triangles,
                              gl::index{0},
                              gl::element_count{36});

              light_program.use(cam, light_position, light_color);

              object_vao.bind();
              gl::draw_arrays(gl::drawing_mode::triangles,
                              gl::index{0},
                              gl::element_count{36});

              timer.trigger();
            });
          });
    });
  }
  catch (std::exception& e) {
    std::cout << "Exception caught at top level:\n" << e.what() << std::endl;
  }
  return static_cast<int>(r);
}