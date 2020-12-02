#include "common.hpp"
#include "key_mapper.hpp"
#include "load_shaders.hpp"
#include "make_window.hpp"
#include "opengl.hpp"
#include "structured_buffers.hpp"
#include "utils.hpp"

#include <type_traits>

void moving_polygon(dpsg::window &wdw) {
  using namespace dpsg;
  using namespace dpsg::input;

  key_mapper kmap;
  wdw.set_key_callback(std::ref(kmap));
  kmap.on(key::escape, close);

  auto shader = load(vs_filename{"shaders/basic_with_color.vs"},
                     fs_filename{"shaders/basic.fs"});

  // NOLINTNEXTLINE
  constexpr float vertices[] = {
      // positions         // colors
      0.0F,  0.5F,   0.0F, 0.0F, 0.0F, 1.0F, // top
      -0.5F, -0.5F,  0.0F, 0.0F, 1.0F, 0.0F, // bottom leFt
      0.5F,  -0.5F,  0.0F, 1.0F, 0.0F, 0.0F, // bottom right
      0.0F,  -0.05F, 0.0F, 0.F,  0.F,  0.F   // bottom
  };
  using packed_layout = packed<group<3>, group<3>>;
  fixed_size_structured_buffer b(packed_layout{}, vertices);

  shader.use();
  gl::clear_color(gl::g{0.3F}, gl::r{0.2F}, gl::b{0.3F});

  wdw.render_loop([&] {
    gl::clear(gl::buffer_bit::color);

    b.draw_array(gl::drawing_mode::triangle_strip);
  });
}

int main() { return windowed(moving_polygon); }
