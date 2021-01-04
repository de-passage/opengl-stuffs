#include <type_traits>
#define GLM_FORCE_SILENT_WARNINGS

#include "glad/glad.h"

#include "buffers.hpp"
#include "camera.hpp"
#include "glfw_context.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "glm_traits.hpp"
#include "input/keys.hpp"
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

// clang-format off
constexpr float vertices[] = {
    -0.5f, -0.5f, -0.5f, 
    0.5f, -0.5f, -0.5f,  
    0.5f,  0.5f, -0.5f,  
    0.5f,  0.5f, -0.5f,  
    -0.5f,  0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f, 

    -0.5f, -0.5f,  0.5f, 
    0.5f, -0.5f,  0.5f,  
    0.5f,  0.5f,  0.5f,  
    0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f, 
    -0.5f, -0.5f,  0.5f, 

    -0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f, 
    -0.5f, -0.5f, -0.5f, 
    -0.5f, -0.5f,  0.5f, 
    -0.5f,  0.5f,  0.5f, 

    0.5f,  0.5f,  0.5f,  
    0.5f,  0.5f, -0.5f,  
    0.5f, -0.5f, -0.5f,  
    0.5f, -0.5f, -0.5f,  
    0.5f, -0.5f,  0.5f,  
    0.5f,  0.5f,  0.5f,  

    -0.5f, -0.5f, -0.5f, 
    0.5f, -0.5f, -0.5f,  
    0.5f, -0.5f,  0.5f,  
    0.5f, -0.5f,  0.5f,  
    -0.5f, -0.5f,  0.5f, 
    -0.5f, -0.5f, -0.5f, 

    -0.5f,  0.5f, -0.5f, 
    0.5f,  0.5f, -0.5f,  
    0.5f,  0.5f,  0.5f,  
    0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f, 
    -0.5f,  0.5f, -0.5f, 
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
  template <
      class T,
      std::enable_if_t<is_template_instance_v<dpsg::fs_filename, T>, int> = 0>
  explicit projection_program(T&& fragment_shader)
      : _program{load(dpsg::vs_filename{"shaders/basic_projection.vs"},
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
      : projection_program{dpsg::fs_filename{"shaders/basic_lighting.fs"}} {}

  template <class T>
  void use(const dpsg::camera<T>& cam,
           const glm::mat4& model,
           const glm::vec3& object_color,
           const glm::vec3& light_color) const noexcept {
    projection_program::use(cam, model);
    _object_color_uniform.bind(object_color);
    _light_color_uniform.bind(light_color);
  }

 private:
  dpsg::program::uniform<glm::vec3> _object_color_uniform{
      uniform_location<glm::vec3>("object_color")};
  dpsg::program::uniform<glm::vec3> _light_color_uniform{
      uniform_location<glm::vec3>("light_color")};
};

struct light_program : projection_program {
  using projection_program::uniform_location;
  light_program()
      : projection_program{dpsg::fs_filename{"shaders/uniform.fs"}} {}

  template <class T>
  void use(const dpsg::camera<T>& cam,
           const glm::mat4& model,
           const glm::vec3& light_color) const noexcept {
    projection_program::use(cam, model);
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
                gl::attrib_location{0}, gl::element_count{3}, gl::stride{3});
            gl::enable_vertex_attrib_array(gl::attrib_location{0});

            vertex_array light_vao;
            light_vao.bind();
            gl::vertex_attrib_pointer<float>(
                gl::attrib_location{0}, gl::element_count{3}, gl::stride{3});
            gl::enable_vertex_attrib_array(gl::attrib_location{0});

            gl::clear_color(gl::r{0.1}, gl::g{0.1}, gl::b{0.1});

            glm::vec3 light_position{1.2, 1.0, 2.0};
            glm::vec3 object_color{1.0, 0.5, 0.31};
            glm::vec3 light_color{1.0, 1.0, 1.0};

            wdw.render_loop([&]([[maybe_unused]] nk::context& ctx) {
              gl::enable(gl::capability::depth_test);
              gl::disable(gl::capability::scissor_test);
              gl::clear(gl::buffer_bit::color | gl::buffer_bit::depth);

              ctx.with_window(
                  "Colors",
                  nk_rect(5, 5, 200, 135),
                  nk::panel_flags::no_scrollbar,
                  [&](nk::window w) {
                    namespace nkw = nk::widget;
                    w.row_dynamic(20, 2);
                    nkw::label(w, "Object");
                    nkw::label(w, "Light");
                    w.row_dynamic(33, 2);
                    nk::widget::slider(w, 0, object_color.r, 1, 0.001);
                    nk::widget::slider(w, 0, light_color.r, 1, 0.001);
                    w.row_dynamic(33, 2);
                    nk::widget::slider(w, 0, object_color.g, 1, 0.001);
                    nk::widget::slider(w, 0, light_color.g, 1, 0.001);
                    w.row_dynamic(33, 2);
                    nk::widget::slider(w, 0, object_color.b, 1, 0.001);
                    nk::widget::slider(w, 0, light_color.b, 1, 0.001);
                  });

              kmap.on(input::key::A, ignore([&] {
                        std::cout << "color: " << light_color.r << " "
                                  << light_color.g << " " << light_color.b
                                  << std::endl;
                      }));

              glm::mat4 model{1.0};
              object_program.use(cam, model, object_color, light_color);
              object_vao.bind();
              gl::draw_arrays(gl::drawing_mode::triangles,
                              gl::index{0},
                              gl::element_count{36});

              model = glm::translate(model, light_position);
              model = glm::scale(model, glm::vec3(0.2));

              light_program.use(cam, model, light_color);

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