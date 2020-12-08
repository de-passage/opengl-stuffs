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

void camera(dpsg::window &wdw, key_mapper &kmap) {

  // NOLINTNEXTLINE
  constexpr float vertices[] = {
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // NOLINT
      0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, // NOLINT
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, // NOLINT
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, // NOLINT
      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, // NOLINT
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // NOLINT

      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, // NOLINT
      0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, // NOLINT
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // NOLINT
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // NOLINT
      -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, // NOLINT
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, // NOLINT

      -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, // NOLINT
      -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, // NOLINT
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // NOLINT
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // NOLINT
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, // NOLINT
      -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, // NOLINT

      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // NOLINT
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, // NOLINT
      0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, // NOLINT
      0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, // NOLINT
      0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, // NOLINT
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // NOLINT

      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // NOLINT
      0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, // NOLINT
      0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, // NOLINT
      0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, // NOLINT
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, // NOLINT
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // NOLINT

      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, // NOLINT
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, // NOLINT
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // NOLINT
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // NOLINT
      -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, // NOLINT
      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f  // NOLINT
  };

  // NOLINTNEXTLINE
  glm::vec3 cube_positions[] = {
      glm::vec3(0.0f, 0.0f, 0.0f),     // NOLINT
      glm::vec3(2.0f, 5.0f, -15.0f),   // NOLINT
      glm::vec3(-1.5f, -2.2f, -2.5f),  // NOLINT
      glm::vec3(-3.8f, -2.0f, -12.3f), // NOLINT
      glm::vec3(2.4f, -0.4f, -3.5f),   // NOLINT
      glm::vec3(-1.7f, 3.0f, -7.5f),   // NOLINT
      glm::vec3(1.3f, -2.0f, -2.5f),   // NOLINT
      glm::vec3(1.5f, 2.0f, -2.5f),    // NOLINT
      glm::vec3(1.5f, 0.2f, -1.5f),    // NOLINT
      glm::vec3(-1.3f, 1.0f, -1.5f)    // NOLINT
  };

  // camera vecs
  constexpr glm::vec3 default_camera_position = glm::vec3(0.0f, 0.0f, 3.0f);
  constexpr glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
  constexpr glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

  using namespace dpsg;
  using namespace dpsg::input;
  using namespace std::literals::chrono_literals;

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
  float aspect_ratio = static_cast<float>(SCR_WIDTH.value) /
                       static_cast<float>(SCR_HEIGHT.value);
  const auto project = [&projection_u](float aspect_ratio) {
    glm::mat4 projection{
        glm::perspective(glm::radians(45.F), aspect_ratio, 0.1F, 100.F)};
    projection_u.bind(projection);
  };
  project(aspect_ratio);
  wdw.set_framebuffer_size_callback([&aspect_ratio,
                                     &project]([[maybe_unused]] window &unused,
                                               width w, height h) {
    aspect_ratio = static_cast<float>(w.value) / static_cast<float>(h.value);
    project(aspect_ratio);
  });

  // Input
  constexpr auto interval = 10ms;
  constexpr float off = 0.01;
  auto camera_position{default_camera_position};
  input_timer timer{[&] { kmap.trigger_pressed_callbacks(wdw); }, interval};

  gl::clear_color({0.2F, 0.3F, 0.3F}); // NOLINT
  wdw.render_loop([&] {
    gl::clear(gl::buffer_bit::color | gl::buffer_bit::depth);

    const glm::mat4 view =
        glm::lookAt(camera_position, camera_position + camera_front, camera_up);
    view_u.bind(view);

    for (std::size_t i = 0; i < std::size(cube_positions); ++i) {
      const float angle{20.F * i};
      const glm::mat4 model{
          glm::rotate(glm::translate(glm::mat4(1.F), cube_positions[i]),
                      glm::radians(angle), glm::vec3(1.F, 0.3F, 0.5F))};

      model_u.bind(model);
      buffer.draw();
    }

    timer.trigger();
  });
}

int main() { windowed(camera); }