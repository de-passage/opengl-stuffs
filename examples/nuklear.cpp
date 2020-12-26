
#include <stdlib.h>
#include "glad/glad.h"

#include "GLFW/glfw3.h"

#include "meta/mixin.hpp"
#include "nuklear/buffer.hpp"
#include "nuklear/context.hpp"
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
#include "with_window.hpp"

#include <chrono>
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

namespace nk_glfw {

struct mouse_button_callback {
  template <class B>
  struct type : B {
   private:
    using base = B;
    using real_t = dpsg::real_type_t<base>;
    constexpr static inline std::chrono::system_clock::duration double_click_lo{
        std::chrono::milliseconds{20}};
    constexpr static inline std::chrono::system_clock::duration double_click_hi{
        std::chrono::milliseconds{200}};

    static inline void callback(GLFWwindow* win,
                                int button,
                                int action,
                                [[maybe_unused]] int mods) {
      type* w = static_cast<type*>(glfwGetWindowUserPointer(win));
      if (button != GLFW_MOUSE_BUTTON_LEFT) {
        return;
      }

      auto position = w->cursor_position();
      if (action == GLFW_PRESS) {
        auto dt = std::chrono::system_clock::now() - w->last_button_click;
        if (dt > double_click_lo && dt < double_click_hi) {
          w->is_double_click_down = true;
          w->double_click_pos = position;
        }
        w->last_button_click = std::chrono::system_clock::now();
      }
      else {
        w->is_double_click_down = false;
      }
    }

   protected:
    explicit type(GLFWwindow* win) noexcept(
        std::is_nothrow_constructible_v<B, GLFWwindow*>)
        : B(win) {
      glfwSetMouseButtonCallback(win, &type::callback);
    }

    bool is_double_click_down{false};
    dpsg::position double_click_pos{};
    std::chrono::system_clock::time_point last_button_click;
  };
};

template <std::size_t TextMax>
struct char_callback_s {
  template <class B>
  struct type : B {
   protected:
    constexpr static inline std::size_t text_max_len{TextMax};
    char text[text_max_len]{0};
    std::size_t text_len{0};

    explicit type(GLFWwindow* win) noexcept(
        std::is_nothrow_constructible_v<B, GLFWwindow*>)
        : B(win) {
      glfwSetCharCallback(win, &type::callback);
    }

   private:
    using base = B;
    using real_t = dpsg::real_type_t<base>;

    static inline void callback(GLFWwindow* win, unsigned int codepoint) {
      type* w = static_cast<type*>(glfwGetWindowUserPointer(win));
      if (w->text_len < TextMax) {
        w->text[w->text_len++] = codepoint;
      }
    }
  };
};

using char_callback = char_callback_s<256>;

struct scroll_callback {
  template <class B>
  struct type : B {
   protected:
    explicit type(GLFWwindow* win) noexcept(
        std::is_nothrow_constructible_v<B, GLFWwindow*>)
        : B(win) {
      glfwSetScrollCallback(win, &type::callback);
    }

    double scroll_x{};
    double scroll_y{};

   private:
    using base = B;
    using real_t = dpsg::real_type_t<base>;

    static inline void callback(GLFWwindow* win,
                                double x_scroll,
                                double y_scroll) noexcept {
      auto* w = static_cast<type*>(glfwGetWindowUserPointer(win));
      w->scroll_x = x_scroll;
      w->scroll_y = y_scroll;
    }
  };
};

struct context_bridge {
  template <class B>
  struct type : B {
   protected:
    explicit type(GLFWwindow* win) noexcept(
        std::is_nothrow_constructible_v<B, GLFWwindow*>)
        : B(win) {}

   public:
    void handle_input(nk::context& ctx) noexcept {
      ctx.handle_input([&](nk::input_handler handler) {
        for (std::size_t i = 0; i < this->text_max_len; ++i) {
          handler.unicode(this->text[i]);
        }

        if (handler.mouse().grab == nk_true) {
          this->set_input_mode(dpsg::cursor_mode::hidden);
        }
        else if (handler.mouse().ungrab == nk_true) {
          this->set_input_mode(dpsg::cursor_mode::normal);
        }

        using dpsg::input::key;
        using dpsg::input::status;
        handler.key(NK_KEY_DEL, this->get_key(key::del) == status::pressed);
        handler.key(NK_KEY_ENTER, this->get_key(key::enter) == status::pressed);
        handler.key(NK_KEY_TAB, this->get_key(key::tab) == status::pressed);
        handler.key(NK_KEY_BACKSPACE,
                    this->get_key(key::backspace) == status::pressed);
        handler.key(NK_KEY_UP, this->get_key(key::up) == status::pressed);
        handler.key(NK_KEY_DOWN, this->get_key(key::down) == status::pressed);
        handler.key(NK_KEY_TEXT_START,
                    this->get_key(key::home) == status::pressed);
        handler.key(NK_KEY_TEXT_END,
                    this->get_key(key::end) == status::pressed);
        handler.key(NK_KEY_SCROLL_START,
                    this->get_key(key::home) == status::pressed);
        handler.key(NK_KEY_SCROLL_END,
                    this->get_key(key::end) == status::pressed);
        handler.key(NK_KEY_SCROLL_DOWN,
                    this->get_key(key::page_down) == status::pressed);
        handler.key(NK_KEY_SCROLL_UP,
                    this->get_key(key::page_up) == status::pressed);
        handler.key(NK_KEY_SHIFT,
                    this->get_key(key::left_shift) == status::pressed ||
                        this->get_key(key::right_shift) == status::pressed);

        if (this->get_key(key::left_control) == status::pressed ||
            this->get_key(key::right_control) == status::pressed) {
          handler.key(NK_KEY_COPY, this->get_key(key::C) == status::pressed);
          handler.key(NK_KEY_PASTE, this->get_key(key::V) == status::pressed);
          handler.key(NK_KEY_CUT, this->get_key(key::X) == status::pressed);
          handler.key(NK_KEY_TEXT_UNDO,
                      this->get_key(key::Z) == status::pressed);
          handler.key(NK_KEY_TEXT_REDO,
                      this->get_key(key::R) == status::pressed);
          handler.key(NK_KEY_TEXT_WORD_LEFT,
                      this->get_key(key::left) == status::pressed);
          handler.key(NK_KEY_TEXT_WORD_RIGHT,
                      this->get_key(key::right) == status::pressed);
          handler.key(NK_KEY_TEXT_LINE_START,
                      this->get_key(key::B) == status::pressed);
          handler.key(NK_KEY_TEXT_LINE_END,
                      this->get_key(key::E) == status::pressed);
        }
        else {
          handler.key(NK_KEY_LEFT, this->get_key(key::left) == status::pressed);
          handler.key(NK_KEY_RIGHT,
                      this->get_key(key::right) == status::pressed);
          handler.key(NK_KEY_COPY, false);
          handler.key(NK_KEY_PASTE, false);
          handler.key(NK_KEY_CUT, false);
          handler.key(NK_KEY_SHIFT, false);
        }
        auto pos = this->cursor_position();
        handler.motion(static_cast<int>(pos.x.value),
                       static_cast<int>(pos.y.value));
        if (handler.mouse().grabbed == nk_true) {
          this->set_cursor_position(dpsg::x{handler.mouse().prev.x},
                                    dpsg::y{handler.mouse().prev.y});
          handler.mouse().pos.x = handler.mouse().prev.x;
          handler.mouse().pos.y = handler.mouse().prev.y;
        }

        using dpsg::input::mouse;
        handler.button(NK_BUTTON_LEFT,
                       static_cast<int>(pos.x.value),
                       static_cast<int>(pos.y.value),
                       this->get_mouse_button(mouse::left) == status::pressed);
        handler.button(
            NK_BUTTON_MIDDLE,
            static_cast<int>(pos.x.value),
            static_cast<int>(pos.y.value),
            this->get_mouse_button(mouse::middle) == status::pressed);
        handler.button(NK_BUTTON_RIGHT,
                       static_cast<int>(pos.x.value),
                       static_cast<int>(pos.y.value),
                       this->get_mouse_button(mouse::right) == status::pressed);
        handler.button(NK_BUTTON_DOUBLE,
                       static_cast<int>(this->double_click_pos.x.value),
                       static_cast<int>(this->double_click_pos.y.value),
                       this->is_double_click_down);
        handler.scroll(nk_vec2(this->scroll_x, this->scroll_y));
        this->text_len = 0;
        this->scroll_x = 0;
        this->scroll_y = 0;
      });
    }
  };
};

struct embedded_context {
  template <class B>
  class type : public B {
   protected:
    template <class... Args>
    explicit type(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<B, Args...>)
        : B(std::forward<Args>(args)...) {}

   private:
    using real_t = dpsg::real_type_t<B>;
    nk::context ctx;
    dpsg::nk_gl3_backend backend;

   public:
    template <class F>
    void render_loop(F f) noexcept(noexcept(f(ctx))) {
      B::render_loop([f = std::move(f), this] {
        using namespace dpsg;
        this->handle_input(ctx);
        f(ctx);

        auto dims = this->framebuffer_size();
        gl::width w{static_cast<gl::uint_t>(dims.width.value)};
        gl::height h{static_cast<gl::uint_t>(dims.height.value)};
        auto window_size = this->window_size();
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
    }

    template <class... Args>
    void load_font(const char* path, float height, Args&&... args) noexcept {
      backend.load_font(ctx, path, height, std::forward<Args>(args)...);
    }
  };
};

struct glad_loader {
  template <class B>
  struct type : B {
    template <class... Args>
    explicit type(Args&&... args) : B(std::forward<Args>(args)...) {
      this->make_context_current();

      static bool already_initialized = false;
      if (!already_initialized) {
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(  // NOLINT
                glfwGetProcAddress))) {
          throw std::runtime_error("Failed to initialize GLAD");
        }
        already_initialized = true;
      }
    }
  };
};

using window = dpsg::base_window<embedded_context,
                                 glad_loader,
                                 context_bridge,
                                 scroll_callback,
                                 char_callback,
                                 mouse_button_callback>;
}  // namespace nk_glfw

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

            wdw.render_loop([&](nk::context& ctx) {
              gl::clear(gl::buffer_bit::color);

              auto window_succeeded = ctx.with_window(
                  "some title",
                  nk_rect(50, 50, 220, 220),
                  nk::panel_flags::title | nk::panel_flags::border |
                      nk::panel_flags::closable | nk::panel_flags::movable,
                  [&](nk::window w) {
                    w.row_static(30, 320, 1);

                    if (button_label(w, "Button")) {
                      std::cout << "Button pressed" << std::endl;
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