
#include "make_window.hpp"
#include "nk_glfw.hpp"
#include "nuklear/nuklear++.hpp"

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
      with_window<nk_glfw::window>(
          window_hint::context_version(3, 3),
          window_hint::opengl_profile(profile::core),
          width{800},
          height{600},
          title{"Nuklear"},
          [](nk_glfw::window& wdw) {
            wdw.load_font("./assets/fonts/DroidSans.ttf", 14);
            wdw.set_input_mode(cursor_mode::hidden);

            gl::clear_color(gl::r{1}, gl::g{1}, gl::b{1}, gl::a{0});
            wdw.render_loop([&](nk::context& ctx) {
              gl::clear(gl::buffer_bit::color);

              auto* some_title_window = ctx.find_window("some title");
              struct nk_rect bounds = (some_title_window != nullptr)
                                          ? some_title_window->bounds
                                          : nk_rect(50, 50, 220, 220);

              auto window_succeeded = ctx.with_window(
                  "some title",
                  bounds,
                  nk::panel_flags::title | nk::panel_flags::border |
                      nk::panel_flags::closable | nk::panel_flags::movable |
                      nk::panel_flags::scalable,
                  [&](nk::window w) {
                    auto* some_title_window = ctx.find_window("some title");
                    w.row_static(30, 100, 2);

                    if (button_label(w, "Button")) {
                      std::cout << "Button pressed" << std::endl;
                    }

                    if (button_color(w, nk_color{255, 0, 0, 255})) {
                      std::cout << "Button colored" << std::endl;
                    }
                  });

              if (!window_succeeded) {
                wdw.should_close(true);
              }
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