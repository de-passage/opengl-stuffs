#include "moving_polygon.hpp"

#include "common.hpp"
#include "key_mapper.hpp"
#include "load_shaders.hpp"
#include "structured_buffers.hpp"
#include "utils.hpp"

#include <type_traits>

void moving_polygon(dpsg::window &wdw) {
  using namespace dpsg;
  using namespace dpsg::input;

  key_mapper kmap;
  kmap.on(key::escape, close);
  wdw.set_key_callback(kmap);

  auto shader = load(vs_filename{"shaders/basic_with_color.vs"}, fs_filename{"shaders/basic.fs"});

  // NOLINTNEXTLINE
  constexpr float vertices[] = {
      // positions         // colors
      0.0F,  0.5F, 0.0F,  0.0F, 0.0F, 1.0F,    // top 
      -0.5F, -0.5F, 0.0F,  0.0F, 1.0F, 0.0F,   // bottom leFt
      0.5F, -0.5F, 0.0F,  1.0F, 0.0F, 0.0F,   // bottom right
      0.0F, -0.05F, 0.0F, 0.F, 0.F, 0.F       // bottom
  };    
  using packed_layout = packed<group<3>, group<3>>;
  fixed_size_structured_buffer b(packed_layout{}, vertices);

  shader.use();
  glClearColor(0.2F, 0.3F, 0.3F, 1.0F);

  wdw.render_loop([&] {
    glClear(GL_COLOR_BUFFER_BIT);

    b.draw_array(drawing_mode::triangle_strip);
  });
}