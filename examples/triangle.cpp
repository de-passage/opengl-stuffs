#include "key_mapper.hpp"
#include "load_shaders.hpp"
#include "make_window.hpp"
#include "opengl.hpp"
#include "structured_buffers.hpp"

void triangle(dpsg::window& wdw) {
  using namespace dpsg;

  auto shader = load(vs_filename{"shaders/basic_with_color.vs"},
                     fs_filename{"shaders/basic.fs"})
                    .value();

  // NOLINTNEXTLINE
  constexpr float vertices[] = {
      // positions        // colors
      0.0F,
      0.5F,
      0.0F,
      0.0F,
      0.0F,
      1.0F,  // top
      -0.5F,
      -0.5F,
      0.0F,
      0.0F,
      1.0F,
      0.0F,  // bottom leFt
      0.5F,
      -0.5F,
      0.0F,
      1.0F,
      0.0F,
      0.0F,  // bottom right
  };

  using packed_layout = packed<group<3>, group<3>>;
  fixed_size_structured_buffer b(packed_layout{}, vertices);
  b.enable();

  shader.use();
  gl::clear_color(gl::g{0.3F}, gl::r{0.2F}, gl::b{0.3F});  // NOLINT

  wdw.render_loop([&] {
    gl::clear(gl::buffer_bit::color);

    b.draw();
  });
}

int main() {
  return windowed(triangle);
}
