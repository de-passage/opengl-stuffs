#include "layout.hpp"
#include "load_shaders.hpp"
#include "make_window.hpp"
#include "shaders.hpp"
#include "structured_buffers.hpp"

#include "opengl.hpp"

void cube(dpsg::window &window) {
  using namespace dpsg;

  auto prog = load(vs_filename{"shaders/ortho_with_offset.vs"},
                   fs_filename("shaders/basic.fs"));

  gl::enable(gl::capability::cull_face);
  gl::cull_face(gl::cull_mode::back);
  gl::front_face(gl::face_mode::clockwise);

  // NOLINTNEXTLINE
  constexpr float vertex_data[] = {
      // Vertices
      0.25f, 0.25f, 0.75f, 1.0f,  // NOLINT
      0.25f, -0.25f, 0.75f, 1.0f, // NOLINT
      -0.25f, 0.25f, 0.75f, 1.0f, // NOLINT

      0.25f, -0.25f, 0.75f, 1.0f,  // NOLINT
      -0.25f, -0.25f, 0.75f, 1.0f, // NOLINT
      -0.25f, 0.25f, 0.75f, 1.0f,  // NOLINT

      0.25f, 0.25f, -0.75f, 1.0f,  // NOLINT
      -0.25f, 0.25f, -0.75f, 1.0f, // NOLINT
      0.25f, -0.25f, -0.75f, 1.0f, // NOLINT

      0.25f, -0.25f, -0.75f, 1.0f,  // NOLINT
      -0.25f, 0.25f, -0.75f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -0.75f, 1.0f, // NOLINT

      -0.25f, 0.25f, 0.75f, 1.0f,   // NOLINT
      -0.25f, -0.25f, 0.75f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -0.75f, 1.0f, // NOLINT

      -0.25f, 0.25f, 0.75f, 1.0f,   // NOLINT
      -0.25f, -0.25f, -0.75f, 1.0f, // NOLINT
      -0.25f, 0.25f, -0.75f, 1.0f,  // NOLINT

      0.25f, 0.25f, 0.75f, 1.0f,   // NOLINT
      0.25f, -0.25f, -0.75f, 1.0f, // NOLINT
      0.25f, -0.25f, 0.75f, 1.0f,  // NOLINT

      0.25f, 0.25f, 0.75f, 1.0f,   // NOLINT
      0.25f, 0.25f, -0.75f, 1.0f,  // NOLINT
      0.25f, -0.25f, -0.75f, 1.0f, // NOLINT

      0.25f, 0.25f, -0.75f, 1.0f, // NOLINT
      0.25f, 0.25f, 0.75f, 1.0f,  // NOLINT
      -0.25f, 0.25f, 0.75f, 1.0f, // NOLINT

      0.25f, 0.25f, -0.75f, 1.0f,  // NOLINT
      -0.25f, 0.25f, 0.75f, 1.0f,  // NOLINT
      -0.25f, 0.25f, -0.75f, 1.0f, // NOLINT

      0.25f, -0.25f, -0.75f, 1.0f, // NOLINT
      -0.25f, -0.25f, 0.75f, 1.0f, // NOLINT
      0.25f, -0.25f, 0.75f, 1.0f,  // NOLINT

      0.25f, -0.25f, -0.75f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -0.75f, 1.0f, // NOLINT
      -0.25f, -0.25f, 0.75f, 1.0f,  // NOLINT

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

  using seq_layout = sequenced<group<4>, group<4>>;
  fixed_size_structured_buffer buffer(seq_layout{}, vertex_data);

  prog.use();
  auto offset = prog.uniform_location<gl::vec_t<2, float>>("offset").value();
  offset.bind(0.5F, 0.25F);
  window.render_loop([&] {
    gl::clear(gl::buffer_bit::color);
    buffer.draw();
  });
}

int main() { return windowed(cube); }