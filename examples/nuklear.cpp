#include "make_window.hpp"
#include "nk_glfw.hpp"
#include "nuklear/nuklear++.hpp"

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