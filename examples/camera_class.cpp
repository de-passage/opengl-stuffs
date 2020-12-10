#include "common.hpp"
#define GLM_FORCE_SILENT_WARNINGS

#include "camera.hpp"
#include "glm_traits.hpp"
#include "input_timer.hpp"
#include "load_shaders.hpp"
#include "make_window.hpp"
#include "stbi_wrapper.hpp"
#include "structured_buffers.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "opengl.hpp"
#include "opengl/glm.hpp"

#include <algorithm>

void camera_class(dpsg::window& wdw, key_mapper& kmap) {
  // NOLINTNEXTLINE
  constexpr float vertices[] = {
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  // NOLINT
      0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,  // NOLINT
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,  // NOLINT
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,  // NOLINT
      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f,  // NOLINT
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,  // NOLINT

      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,  // NOLINT
      0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,  // NOLINT
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  // NOLINT
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  // NOLINT
      -0.5f, 0.5f,  0.5f,  0.0f, 1.0f,  // NOLINT
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,  // NOLINT

      -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,  // NOLINT
      -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,  // NOLINT
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // NOLINT
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // NOLINT
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,  // NOLINT
      -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,  // NOLINT

      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // NOLINT
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,  // NOLINT
      0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,  // NOLINT
      0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,  // NOLINT
      0.5f,  -0.5f, 0.5f,  0.0f, 0.0f,  // NOLINT
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // NOLINT

      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // NOLINT
      0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,  // NOLINT
      0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,  // NOLINT
      0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,  // NOLINT
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,  // NOLINT
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // NOLINT

      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f,  // NOLINT
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,  // NOLINT
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // NOLINT
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  // NOLINT
      -0.5f, 0.5f,  0.5f,  0.0f, 0.0f,  // NOLINT
      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f   // NOLINT
  };

  // NOLINTNEXTLINE
  glm::vec3 cube_positions[] = {
      glm::vec3(0.0f, 0.0f, 0.0f),      // NOLINT
      glm::vec3(2.0f, 5.0f, -15.0f),    // NOLINT
      glm::vec3(-1.5f, -2.2f, -2.5f),   // NOLINT
      glm::vec3(-3.8f, -2.0f, -12.3f),  // NOLINT
      glm::vec3(2.4f, -0.4f, -3.5f),    // NOLINT
      glm::vec3(-1.7f, 3.0f, -7.5f),    // NOLINT
      glm::vec3(1.3f, -2.0f, -2.5f),    // NOLINT
      glm::vec3(1.5f, 2.0f, -2.5f),     // NOLINT
      glm::vec3(1.5f, 0.2f, -1.5f),     // NOLINT
      glm::vec3(-1.3f, 1.0f, -1.5f)     // NOLINT
  };

  using namespace dpsg;
  using namespace dpsg::input;
  using namespace std::literals::chrono_literals;

  using camera = dpsg::camera<dpsg::traits::glm>;

  constexpr radians default_yaw{to_radians(degrees{-90})};
  constexpr radians default_pitch{0};
  constexpr radians default_fov{to_radians(degrees{45})};

  gl::enable(gl::capability::depth_test);

  // Shader program
  auto prog = load(vs_filename("shaders/projected.vs"),
                   fs_filename("shaders/two_textures_mixed.fs"));
  prog.use();

  auto texture1_u = prog.uniform_location<sampler2D>("texture1").value();
  auto texture2_u = prog.uniform_location<sampler2D>("texture2").value();
  auto view_u = prog.uniform_location<glm::mat4>("view").value();
  auto model_u = prog.uniform_location<glm::mat4>("model").value();
  auto projection_u = prog.uniform_location<glm::mat4>("projection").value();

  // Textures
  auto tex1 =
      load<texture_rgb>(texture_filename("assets/container.jpg")).value();
  auto tex2 =
      load<texture_rgba>(texture_filename("assets/awesomeface.png")).value();

  texture1_u.bind(tex1, gl::texture_name::_0);
  texture2_u.bind(tex2, gl::texture_name::_1);

  // Buffers
  using layout = packed<group<3>, group<2>>;
  fixed_size_structured_buffer buffer{layout{}, vertices};
  buffer.enable();

  // Projection management
  aspect_ratio aspect_ratio = SCR_WIDTH / SCR_HEIGHT;
  camera cam{aspect_ratio};

  wdw.set_framebuffer_size_callback(ignore([&cam](width w, height h) {
    cam.aspect_ratio(w, h);
    glViewport(0, 0, w.value, h.value);
  }));

  // Input
  constexpr auto interval = 10ms;
  constexpr float camera_speed = .04;
  input_timer timer{[&] { kmap.trigger_pressed_callbacks(wdw); }, interval};

  const auto move_forward = ignore([&] { cam.advance(camera_speed); });
  const auto move_backward = ignore([&] { cam.advance(-camera_speed); });
  const auto strafe_left = ignore([&] { cam.strafe(-camera_speed); });
  const auto strafe_right = ignore([&] { cam.strafe(camera_speed); });

  kmap.while_(key::up, move_forward);
  kmap.while_(key::W, move_forward);
  kmap.while_(key::down, move_backward);
  kmap.while_(key::S, move_backward);
  kmap.while_(key::left, strafe_left);
  kmap.while_(key::A, strafe_left);
  kmap.while_(key::right, strafe_right);
  kmap.while_(key::D, strafe_right);

  kmap.on(key::R,
          ignore([&] { cam.reset(default_yaw, default_pitch, default_fov); }));

  wdw.set_input_mode(cursor_mode::disabled);
  double last_x{0};
  double last_y{0};
  wdw.set_cursor_pos_callback([&](window& wdw, double x, double y) {
    last_x = x;
    last_y = y;
    const auto callback = [&](double x, double y) {
      constexpr float sensitivity = glm::radians(0.1F);
      double x_offset = (x - last_x) * sensitivity;
      double y_offset = (last_y - y) * sensitivity;
      last_x = x;
      last_y = y;
      cam.rotate(x_offset, y_offset);
    };
    callback(x, y);
    wdw.set_cursor_pos_callback(ignore(callback));
  });

  wdw.set_scroll_callback(
      ignore([&]([[maybe_unused]] double x, double y) { cam.zoom(y); }));

  // Render loop
  gl::clear_color({0.2F, 0.3F, 0.3F});  // NOLINT
  wdw.render_loop([&] {
    gl::clear(gl::buffer_bit::color | gl::buffer_bit::depth);
    projection_u.bind(cam.projection());

    view_u.bind(cam.view());

    for (std::size_t i = 0; i < std::size(cube_positions); ++i) {
      const float angle{20.F * i};
      const glm::mat4 model{
          glm::rotate(glm::translate(glm::mat4(1.F), cube_positions[i]),
                      glm::radians(angle),
                      glm::vec3(1.F, 0.3F, 0.5F))};

      model_u.bind(model);
      buffer.draw();
    }

    timer.trigger();
  });
}

int main() {
  windowed(camera_class);
}
