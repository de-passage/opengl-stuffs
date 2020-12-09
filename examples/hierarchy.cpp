#include "make_window.hpp"
#include "opengl.hpp"

void hierarchy(dpsg::window &wdw) {
  using namespace dpsg;

  gl::enable(gl::capability::depth_test);

  wdw.render_loop(
      [&] { gl::clear(gl::buffer_bit::color | gl::buffer_bit::depth); });
}

int main() { return windowed(hierarchy); }