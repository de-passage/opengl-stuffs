#include "glad/glad.h"

#include "glfw_context.hpp"
#include "nk_glfw.hpp"
#include "opengl.hpp"
#include "window.hpp"
#include "window/hints.hpp"
#include "with_window.hpp"

int main() {
  using namespace dpsg;
  auto r = within_glfw_context([] {
    return with_window<nk_glfw::window>(
        window_hint::context_version(3, 3),
        SCR_WIDTH,
        SCR_HEIGHT,
        title{"Lighting"},
        [](auto& wdw) {
          wdw.render_loop(
              [&](nk::context& ctx) { gl::clear(gl::buffer_bit::color); });
        });
  });
  return static_cast<int>(r);
}