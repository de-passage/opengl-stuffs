#include "glad/glad.h"

#include "glfw_context.hpp"
#include "load_shaders.hpp"
#include "nk_glfw.hpp"
#include "opengl.hpp"
#include "utility.hpp"
#include "window.hpp"
#include "window/hints.hpp"
#include "with_window.hpp"

int main() {
  using namespace dpsg;
  ExecutionStatus r = dpsg::ExecutionStatus::Failure;
  try {
    r = within_glfw_context([] {
      return with_window<nk_glfw::window>(
          window_hint::context_version(3, 3),
          SCR_WIDTH,
          SCR_HEIGHT,
          title{"Lighting"},
          [](auto& wdw) {
            auto program =
                load(vs_filename{"doesn't exist"}, fs_filename{"doesn't exist"})
                    .value();
            wdw.render_loop(
                [&](nk::context& ctx) { gl::clear(gl::buffer_bit::color); });
          });
    });
  }
  catch (std::exception& e) {
    std::cout << "Exception caught at top level:'\n" << e.what() << std::endl;
  }
  return static_cast<int>(r);
}