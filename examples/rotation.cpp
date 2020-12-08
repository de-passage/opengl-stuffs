#define GLM_FORCE_SILENT_WARNINGS

#include "common.hpp"
#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
#include "load_shaders.hpp"
#include "make_window.hpp"
#include "opengl.hpp"
#include "opengl/glm.hpp"
#include "stbi_wrapper.hpp"
#include "structured_buffers.hpp"
#include "texture.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

// NOLINTNEXTLINE
constexpr glm::vec3 cube_positions[] = {
    glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),   // NOLINT
    glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f), // NOLINT
    glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),   // NOLINT
    glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),    // NOLINT
    glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};  // NOLINT

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
    -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, // NOLINT
};

void rotation(dpsg::window &window) {
  using namespace dpsg;
  gl::enable(gl::capability::depth_test);
  auto prog = load(vs_filename("shaders/projected.vs"),
                   fs_filename("shaders/two_textures_mixed.fs"));

  prog.use();
  auto tex1 =
      load<texture_rgb>(texture_filename("assets/container.jpg")).value();
  auto tex2 =
      load<texture_rgba>(texture_filename("assets/awesomeface.png")).value();

  auto texture1_u = prog.uniform_location<sampler2D>("texture1").value();
  auto texture2_u = prog.uniform_location<sampler2D>("texture2").value();

  auto projection_u = prog.uniform_location<glm::mat4>("projection").value();
  auto view_u = prog.uniform_location<glm::mat4>("view").value();
  auto model_u = prog.uniform_location<glm::mat4>("model").value();

  texture1_u.bind(tex1, gl::texture_name::_0);
  texture2_u.bind(tex2, gl::texture_name::_1);

  gl::clear_color({0.2, 0.3, 0.3});

  using my_layout = packed<group<3>, group<2>>;

  fixed_size_structured_buffer buff{my_layout{}, vertices};
  buff.enable();

  float aspect_ratio = static_cast<float>(SCR_WIDTH.value) /
                       static_cast<float>(SCR_HEIGHT.value);

  const auto project = [&aspect_ratio, &projection_u] {
    glm::mat4 projection{
        glm::perspective(glm::radians(45.F), aspect_ratio, 0.1F, 100.F)};
    projection_u.bind(projection);
  };
  const auto reshape = [&aspect_ratio,
                        &project]([[maybe_unused]] dpsg::window &wdw, width w,
                                  height h) {
    glViewport(0, 0, w.value, h.value);
    aspect_ratio = static_cast<float>(w.value) / static_cast<float>(h.value);
    project();
  };

  window.set_framebuffer_size_callback(reshape);

  project();
  glm::mat4 view{glm::translate(glm::mat4(1.F), glm::vec3(0.F, 0.F, -3.F))};
  view_u.bind(view);

  window.render_loop([&] {
    gl::clear(gl::buffer_bit::color | gl::buffer_bit::depth);
    for (std::size_t i = 0; i < std::size(cube_positions); ++i) {
      glm::mat4 model{glm::translate(glm::mat4(1.F), cube_positions[i])};
      const float angle = 20.F * i;
      model = rotate(model, glm::radians(angle), glm::vec3(1.F, 1.F, 0.5F));
      model_u.bind(model);
      buff.draw();
    }
  });
}

int main() { return windowed(rotation); }