#include "load_shaders.hpp"
#include "make_window.hpp"
#include "opengl.hpp"
#include "shaders.hpp"
#include "structured_buffers.hpp"

void matrices(dpsg::window &window) {
  using namespace dpsg;

  // NOLINTNEXTLINE
  constexpr float vertex_data[] = {
      // Vertices
      0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT
      0.25f, -0.25f, -1.25f, 1.0f, // NOLINT
      -0.25f, 0.25f, -1.25f, 1.0f, // NOLINT

      0.25f, -0.25f, -1.25f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -1.25f, 1.0f, // NOLINT
      -0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT

      0.25f, 0.25f, -2.75f, 1.0f,  // NOLINT
      -0.25f, 0.25f, -2.75f, 1.0f, // NOLINT
      0.25f, -0.25f, -2.75f, 1.0f, // NOLINT

      0.25f, -0.25f, -2.75f, 1.0f,  // NOLINT
      -0.25f, 0.25f, -2.75f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -2.75f, 1.0f, // NOLINT

      -0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -1.25f, 1.0f, // NOLINT
      -0.25f, -0.25f, -2.75f, 1.0f, // NOLINT

      -0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -2.75f, 1.0f, // NOLINT
      -0.25f, 0.25f, -2.75f, 1.0f,  // NOLINT

      0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT
      0.25f, -0.25f, -2.75f, 1.0f, // NOLINT
      0.25f, -0.25f, -1.25f, 1.0f, // NOLINT

      0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT
      0.25f, 0.25f, -2.75f, 1.0f,  // NOLINT
      0.25f, -0.25f, -2.75f, 1.0f, // NOLINT

      0.25f, 0.25f, -2.75f, 1.0f,  // NOLINT
      0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT
      -0.25f, 0.25f, -1.25f, 1.0f, // NOLINT

      0.25f, 0.25f, -2.75f, 1.0f,  // NOLINT
      -0.25f, 0.25f, -1.25f, 1.0f, // NOLINT
      -0.25f, 0.25f, -2.75f, 1.0f, // NOLINT

      0.25f, -0.25f, -2.75f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -1.25f, 1.0f, // NOLINT
      0.25f, -0.25f, -1.25f, 1.0f,  // NOLINT

      0.25f, -0.25f, -2.75f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -2.75f, 1.0f, // NOLINT
      -0.25f, -0.25f, -1.25f, 1.0f, // NOLINT

      // Colors
      0.0f, 0.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 0.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 0.0f, 1.0f, 1.0f, // NOLINT

      0.0f, 0.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 0.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 0.0f, 1.0f, 1.0f, // NOLINT

      0.8f, 0.8f, 0.8f, 1.0f, // NOLINT
      0.8f, 0.8f, 0.8f, 1.0f, // NOLINT
      0.8f, 0.8f, 0.8f, 1.0f, // NOLINT

      0.8f, 0.8f, 0.8f, 1.0f, // NOLINT
      0.8f, 0.8f, 0.8f, 1.0f, // NOLINT
      0.8f, 0.8f, 0.8f, 1.0f, // NOLINT

      0.0f, 1.0f, 0.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 0.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 0.0f, 1.0f, // NOLINT

      0.0f, 1.0f, 0.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 0.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 0.0f, 1.0f, // NOLINT

      0.5f, 0.5f, 0.0f, 1.0f, // NOLINT
      0.5f, 0.5f, 0.0f, 1.0f, // NOLINT
      0.5f, 0.5f, 0.0f, 1.0f, // NOLINT

      0.5f, 0.5f, 0.0f, 1.0f, // NOLINT
      0.5f, 0.5f, 0.0f, 1.0f, // NOLINT
      0.5f, 0.5f, 0.0f, 1.0f, // NOLINT

      1.0f, 0.0f, 0.0f, 1.0f, // NOLINT
      1.0f, 0.0f, 0.0f, 1.0f, // NOLINT
      1.0f, 0.0f, 0.0f, 1.0f, // NOLINT

      1.0f, 0.0f, 0.0f, 1.0f, // NOLINT
      1.0f, 0.0f, 0.0f, 1.0f, // NOLINT
      1.0f, 0.0f, 0.0f, 1.0f, // NOLINT

      0.0f, 1.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 1.0f, 1.0f, // NOLINT

      0.0f, 1.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 1.0f, 1.0f, // NOLINT
  };
  auto prog = load(vs_filename("shaders/matrix_perspective.vs"),
                   fs_filename("shaders/basic.fs"));

  gl::enable(gl::capability::cull_face);
  gl::cull_face(gl::cull_mode::back);
  gl::front_face(gl::face_mode::clockwise);

  using seq_layout = sequenced<group<4>, group<4>>;
  fixed_size_structured_buffer buffer(seq_layout{}, vertex_data);
  buffer.enable();

  auto offset_u = prog.uniform_location<gl::vec_t<2, float>>("offset").value();
  auto perspective_u =
      prog.uniform_location<gl::mat_t<4, 4>>("perspective").value();

  gl::mat_t<4, 4> perspective;

  constexpr float frustum_scale = 1.0;
  constexpr float z_near = 1.0;
  constexpr float z_far = 3.0;

  perspective[{0, 0}] = frustum_scale;
  perspective[{1, 1}] = frustum_scale;
  perspective[{2, 2}] = (z_near * z_far) / (z_near - z_far);
  perspective[{2, 3}] = -1.0;
  perspective[{3, 2}] = (2 * z_near * z_far) / (z_near - z_far);

  prog.use();

  offset_u.bind(0.5, 0.5);
  perspective_u.bind(perspective);

  const auto reshape = [&perspective,
                        &perspective_u]([[maybe_unused]] dpsg::window &wdw,
                                        width w, height h) {
    perspective[{0, 0}] = frustum_scale / (static_cast<float>(w.value) /
                                           static_cast<float>(h.value));
    perspective_u.bind(perspective);
    glViewport(0, 0, w.value, h.value);
  };
  window.set_framebuffer_size_callback(reshape);

  window.render_loop([&] {
    gl::clear(gl::buffer_bit::color);
    buffer.draw();
  });
}

int main() { return windowed(matrices); }