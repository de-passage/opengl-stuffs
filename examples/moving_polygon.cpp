#include "key_mapper.hpp"
#include "load_shaders.hpp"
#include "make_window.hpp"
#include "opengl.hpp"
#include "structured_buffers.hpp"

void moving_polygon(dpsg::window &wdw, key_mapper &kmap) {
  using namespace dpsg;
  using namespace dpsg::input;

  auto shader = load(vs_filename{"shaders/positionable_triangle.vs"},
                     fs_filename{"shaders/yellow.fs"});

  // NOLINTNEXTLINE
  constexpr float vertices[] = {
      // positions         // colors
      0.0F,  0.5F,  0.0F, 0.0F, 0.0F, 1.0F, // top
      -0.5F, -0.5F, 0.0F, 0.0F, 1.0F, 0.0F, // bottom leFt
      0.5F,  -0.5F, 0.0F, 1.0F, 0.0F, 0.0F, // bottom right
  };
  using packed_layout = packed<group<3>, group<3>>;
  fixed_size_structured_buffer b(packed_layout{}, vertices);
  b.enable();

  auto offset_uniform =
      shader.uniform_location<gl::vec_t<2, float>>("xyOffset").value();
  float y_offset{0};
  float x_offset{0};
  const auto move = [&](float &f) {
    return [&, f = std::ref(f)](float offset) {
      return ignore([&, offset] {
        f += offset;
        offset_uniform.bind(x_offset, y_offset);
      });
    };
  };
  const auto move_x = move(x_offset);
  const auto move_y = move(y_offset);
  kmap.on(key::up, move_y(+0.1F));
  kmap.on(key::down, move_y(-0.1F));
  kmap.on(key::right, move_x(+0.1F));
  kmap.on(key::left, move_x(-0.1F));

  shader.use();
  gl::clear_color(gl::g{0.3F}, gl::r{0.2F}, gl::b{0.3F});

  wdw.render_loop([&] {
    gl::clear(gl::buffer_bit::color);

    b.draw(gl::drawing_mode::triangle_strip);
  });
}

int main() { return windowed(moving_polygon); }
