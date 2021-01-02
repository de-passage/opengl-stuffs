#define GLM_FORCE_SILENT_WARNINGS

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

void camera(dpsg::window& wdw, key_mapper& kmap) {
  // clang-format off
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
  // clang-format on

  // camera vecs
  constexpr glm::vec3 default_camera_position = glm::vec3(0.0f, 0.0f, 3.0f);
  constexpr glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
  constexpr glm::vec3 default_camera_front = glm::vec3(0.0f, 0.0f, -1.0f);

  using namespace dpsg;
  using namespace dpsg::input;
  using namespace std::literals::chrono_literals;

  gl::enable(gl::capability::depth_test);

  // Shader program
  auto prog = load(vs_filename("shaders/projected.vs"),
                   fs_filename("shaders/two_textures_mixed.fs"))
                  .value();
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
  float aspect_ratio = static_cast<float>(SCR_WIDTH.value) /
                       static_cast<float>(SCR_HEIGHT.value);
  constexpr double min_fov{1};
  constexpr double max_fov{45};
  constexpr double default_fov{40};
  double fov{default_fov};
  const auto project = [&projection_u, &fov](float aspect_ratio) {
    glm::mat4 projection{glm::perspective(
        glm::radians(static_cast<float>(fov)), aspect_ratio, 0.1F, 100.F)};
    projection_u.bind(projection);
  };
  wdw.set_framebuffer_size_callback(ignore([&aspect_ratio](width w, height h) {
    aspect_ratio = static_cast<float>(w.value) / static_cast<float>(h.value);
    glViewport(0, 0, w.value, h.value);
  }));

  glm::vec3 camera_front = default_camera_front;
  const auto camera_horizontal = [&camera_front, &camera_up] {
    return glm::normalize(glm::cross(camera_front, camera_up));
  };

  // Input
  constexpr auto interval = 10ms;
  constexpr float camera_speed = .04;
  auto camera_position{default_camera_position};
  input_timer timer{[&] { kmap.trigger_pressed_callbacks(wdw); }, interval};

  const auto move_forward =
      ignore([&] { camera_position += camera_speed * camera_front; });
  const auto move_backward =
      ignore([&] { camera_position -= camera_speed * camera_front; });
  const auto strafe_left =
      ignore([&] { camera_position -= camera_speed * camera_horizontal(); });
  const auto strafe_right =
      ignore([&] { camera_position += camera_speed * camera_horizontal(); });

  kmap.while_(key::up, move_forward);
  kmap.while_(key::W, move_forward);
  kmap.while_(key::down, move_backward);
  kmap.while_(key::S, move_backward);
  kmap.while_(key::left, strafe_left);
  kmap.while_(key::A, strafe_left);
  kmap.while_(key::right, strafe_right);
  kmap.while_(key::D, strafe_right);

  constexpr double default_yaw{-90};
  constexpr double default_pitch{};
  constexpr double max_pitch{89.0};
  constexpr double min_pitch{-max_pitch};
  double yaw{default_yaw};
  double pitch{default_pitch};

  kmap.on(key::R, ignore([&] {
            yaw = default_yaw;
            pitch = default_pitch;
            fov = default_fov;
            camera_front = default_camera_front;
            camera_position = default_camera_position;
          }));

  wdw.set_input_mode(cursor_mode::disabled);
  double last_x{0};
  double last_y{0};
  wdw.set_cursor_pos_callback([&](window& wdw, double x, double y) {
    last_x = x;
    last_y = y;
    const auto callback = [&](double x, double y) {
      constexpr float sensitivity = 0.1F;
      double x_offset = (x - last_x) * sensitivity;
      double y_offset = (last_y - y) * sensitivity;
      last_x = x;
      last_y = y;
      yaw += x_offset;
      pitch = std::clamp(pitch + y_offset, min_pitch, max_pitch);

      const glm::vec3 direction{
          static_cast<float>(cos(glm::radians(yaw)) * cos(glm::radians(pitch))),
          static_cast<float>(sin(glm::radians(pitch))),
          static_cast<float>(sin(glm::radians(yaw)) *
                             cos(glm::radians(pitch)))};
      camera_front = glm::normalize(direction);
    };
    callback(x, y);
    wdw.set_cursor_pos_callback(ignore(callback));
  });

  wdw.set_scroll_callback(ignore([&]([[maybe_unused]] double x, double y) {
    fov = std::clamp(fov - y, min_fov, max_fov);
  }));

  // Render loop
  gl::clear_color({0.2F, 0.3F, 0.3F});  // NOLINT
  wdw.render_loop([&] {
    gl::clear(gl::buffer_bit::color | gl::buffer_bit::depth);
    project(aspect_ratio);

    const glm::mat4 view =
        glm::lookAt(camera_position, camera_position + camera_front, camera_up);
    view_u.bind(view);

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
  windowed(camera);
}