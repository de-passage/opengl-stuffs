
#include <stdlib.h>

#include "glad/glad.h"

#include "meta/mixin.hpp"
#include "nuklear/buffer.hpp"
#include "nuklear/enums.hpp"
#include "nuklear/font_atlas.hpp"
#include "nuklear/interfaces.hpp"
#include "nuklear/nuklear++.hpp"

#include "glfw_context.hpp"
#include "layout.hpp"
#include "nk_gl3_backend.hpp"
#include "opengl.hpp"
#include "program.hpp"
#include "shaders.hpp"
#include "utility.hpp"
#include "window.hpp"

#include <cstddef>
#include <exception>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace nk {
class user_font {
 public:
  using type = nk_user_font;
  using reference = type&;
  using const_reference = const type&;

  [[nodiscard]] constexpr const_reference font() const noexcept {
    return _font;
  }

  [[nodiscard]] constexpr reference font() noexcept { return _font; }

 private:
  struct nk_user_font _font;
};

}  // namespace nk
namespace nk::widget {

namespace detail {
template <class T>
constexpr static inline bool is_window_v =
    dpsg::contains_mixin_v<T, nk::window_interface>;
}  // namespace detail

template <class T, std::enable_if_t<detail::is_window_v<T>, int> = 0>
inline bool button_label(T& ctx, const char* title) noexcept {
  return nk_button_label(&ctx.ctx(), title) == nk_true;
}

template <class T, std::enable_if_t<detail::is_window_v<T>, int> = 0>
inline bool button_label(T& ctx,
                         const nk_style_button& style,
                         const char* title) noexcept {
  return nk_button_label_styled(&ctx.ctx(), &style, title) == nk_true;
}

template <class T, std::enable_if_t<detail::is_window_v<T>, int> = 0>
inline bool button_color(T& ctx, nk_color color) noexcept {
  return nk_button_color(&ctx.ctx(), color);
}

}  // namespace nk::widget

int main() {
  using namespace dpsg;
  using namespace nk::widget;
  try {
    within_glfw_context([] {
      with_window(
          window_hint::context_version(3, 3),
          window_hint::opengl_profile(profile::core),
          width{800},
          height{600},
          title{"Nuklear"},
          [](window& wdw) {
            wdw.make_context_current();

            if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(  // NOLINT
                    glfwGetProcAddress))) {
              throw std::runtime_error("Failed to initialize GLAD");
            }

            nk::context ctx;
            nk_gl3_backend backend;
            backend.load_font(ctx, "./assets/fonts/DroidSans.ttf", 14);

            wdw.render_loop([&] {
              gl::clear(gl::buffer_bit::color);

              auto window_succeeded = ctx.with_window(
                  "some title",
                  nk_rect(50, 50, 220, 220),
                  nk::panel_flags::title | nk::panel_flags::border |
                      nk::panel_flags::closable,
                  [&](nk::window w) {
                    w.row_static(30, 320, 1);

                    if (button_label(w, "Button")) {
                      std::cout << "Button pressed" << std::endl;
                    }
                  });

              if (!window_succeeded) {
                throw std::runtime_error("nk window failure");
              }

              auto dims = wdw.framebuffer_size();
              gl::width w{static_cast<gl::uint_t>(dims.width.value)};
              gl::height h{static_cast<gl::uint_t>(dims.height.value)};
              auto window_size = wdw.window_size();
              struct nk_vec2 scale;
              scale.x = static_cast<float>(w.value) /
                        static_cast<float>(window_size.width.value);
              scale.y = static_cast<float>(h.value) /
                        static_cast<float>(window_size.height.value);
              gl::viewport(w, h);
              backend.render(ctx,
                             w,
                             h,
                             MAX_VERTEX_MEMORY,
                             MAX_ELEMENT_MEMORY,
                             nk_anti_aliasing::NK_ANTI_ALIASING_ON,
                             scale);
              ctx.clear();
            });
          });
    });
  }
  catch (std::exception& e) {
    std::cerr << "Exception caught at top level:\n" << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}