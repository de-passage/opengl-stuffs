
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
inline bool button(T& ctx, const char* title) noexcept {
  return nk_button_label(&ctx.ctx(), title) == nk_true;
}

template <class T, std::enable_if_t<detail::is_window_v<T>, int> = 0>
inline bool button(T& ctx,
                   const nk_style_button& style,
                   const char* title) noexcept {
  return nk_button_label_styled(&ctx.ctx(), &style, title) == nk_true;
}

template <class T, std::enable_if_t<detail::is_window_v<T>, int> = 0>
inline bool button(T& ctx, nk_color color) noexcept {
  return nk_button_color(&ctx.ctx(), color);
}

template <class T, std::enable_if_t<detail::is_window_v<T>, int> = 0>
inline bool option(T& ctx, const char* label, bool selected) noexcept {
  return nk_option_label(&ctx.ctx(), label, selected ? nk_true : nk_false) ==
         nk_true;
}

template <class T, std::enable_if_t<detail::is_window_v<T>, int> = 0>
inline void label(T& ctx,
                  const char* label,
                  nk_text_alignment text_align = NK_TEXT_LEFT) noexcept {
  nk_label(&ctx.ctx(), label, text_align);
}

template <class T, std::enable_if_t<detail::is_window_v<T>, int> = 0>
inline bool slider(T& ctx,
                   float min,
                   float& value,
                   float max,
                   float step) noexcept {
  return nk_slider_float(&ctx.ctx(), min, &value, max, step) == nk_true;
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
            float value{0.5};
            enum { EASY, HARD } op{EASY};

            wdw.load_font("./assets/fonts/DroidSans.ttf", 14);
            wdw.set_input_mode(cursor_mode::hidden);

            wdw.render_loop([&](nk::context& ctx) {
              gl::clear(gl::buffer_bit::color);

              struct nk_rect bounds = nk_rect(50, 50, 220, 220);

              auto window_succeeded = ctx.with_window(
                  "some title",
                  bounds,
                  nk::panel_flags::title | nk::panel_flags::border |
                      nk::panel_flags::closable | nk::panel_flags::movable |
                      nk::panel_flags::scalable,
                  [&](nk::window w) {
                    w.row_static(30, 80, 1);

                    if (button(w, "Button")) {
                      std::cout << "Button pressed" << std::endl;
                    }

                    w.row_dynamic(30, 2);

                    /* fixed widget window ratio width */
                    if (option(w, "easy", op == EASY)) {
                      op = EASY;
                    }
                    if (option(w, "hard", op == HARD)) {
                      op = HARD;
                    }

                    /* custom widget pixel width */
                    w.with_row(NK_STATIC, 30, 2, [&](nk::row r) {
                      r.push(50);
                      label(r, "Volume:", NK_TEXT_LEFT);
                      r.push(110);
                      slider(r, 0, value, 1.0, 0.01);
                    });
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