#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear/nuklear.h"

#include "buffers.hpp"
#include "glfw_context.hpp"
#include "layout.hpp"
#include "meta/mixin.hpp"
#include "opengl.hpp"
#include "program.hpp"
#include "shaders.hpp"
#include "window.hpp"

#include <cstddef>
#include <iterator>
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

struct scroll_t {
  unsigned int x;
  unsigned int y;
};

namespace detail {

struct self_interface {
  template <class B>
  struct type : B {
   protected:
    using crtp_type = dpsg::real_type_t<B>;
    using crtp_const_type = std::add_const_t<dpsg::real_type_t<B>>;
    using crtp_ptr = crtp_type*;
    using crtp_cptr = crtp_const_type*;
    using vec2_t = struct nk_vec2;
    using rect_t = struct nk_rect;

    [[nodiscard]] constexpr crtp_ptr self() noexcept {
      return static_cast<crtp_ptr>(this);
    }

    [[nodiscard]] constexpr crtp_cptr self() const noexcept {
      return static_cast<crtp_ptr>(this);
    }

    [[nodiscard]] constexpr auto* ctx() const noexcept {
      return &self()->ctx();
    }

    [[nodiscard]] constexpr auto* ctx() noexcept { return &self()->ctx(); }
  };
};

struct window_interface {
  template <class B>
  class type : public B {
    using base = B;

   public:
    using panel_t = struct nk_panel*;
    using command_buffer_t = struct nk_command_buffer*;
    using vec2_t = typename B::vec2_t;
    using rect_t = typename B::rect_t;

    [[nodiscard]] inline panel_t panel() noexcept {
      return nk_window_get_panel(base::ctx());
    }

    [[nodiscard]] inline rect_t content_region() noexcept {
      return nk_window_get_content_region(base::ctx());
    }

    [[nodiscard]] inline vec2_t content_region_min() noexcept {
      return nk_window_get_content_region_min(base::ctx());
    }

    [[nodiscard]] inline vec2_t content_region_max() noexcept {
      return nk_window_get_content_region_max(base::ctx());
    }

    [[nodiscard]] inline vec2_t content_region_size() noexcept {
      return nk_window_get_content_region_size(base::ctx());
    }

    [[nodiscard]] inline command_buffer_t canvas() noexcept {
      return nk_window_get_canvas(base::ctx());
    }

    [[nodiscard]] inline scroll_t scroll() noexcept {
      scroll_t s{};
      nk_window_get_scroll(base::ctx(), &s.x, &s.y);
      return s;
    }

    [[nodiscard]] inline unsigned int x_scroll() noexcept {
      unsigned int x{};
      nk_window_get_scroll(base::ctx(), &x, nullptr);
      return x;
    }

    [[nodiscard]] inline unsigned int y_scroll() noexcept {
      unsigned int y{};
      nk_window_get_scroll(base::ctx(), nullptr, &y);
      return y;
    }

    [[nodiscard]] inline bool has_focus() noexcept {
      return nk_window_has_focus(base::ctx()) == nk_true;
    }

    [[nodiscard]] inline bool is_hovered() noexcept {
      return nk_window_is_hovered(base::ctx()) == nk_true;
    }

    inline void set_scroll(unsigned int x, unsigned int y) noexcept {
      nk_window_set_scroll(base::ctx(), x, y);
    }
  };
};

struct input_interface {
  template <class B>
  class type : public B {
    using base = B;

   public:
    using vec2_t = typename base::vec2_t;

    inline void motion(int x, int y) noexcept {
      nk_input_motion(base::ctx(), x, y);
    }

    inline void key(nk_keys key, bool down) noexcept {
      nk_input_key(base::ctx(), key, down ? nk_true : nk_false);
    }

    inline void button(nk_buttons button, int x, int y, bool down) noexcept {
      nk_input_button(base::ctx(), button, x, y, down ? nk_true : nk_false);
    }

    inline void scroll(vec2_t val) noexcept {
      nk_input_scroll(base::ctx(), val);
    }

    inline void character(char c) noexcept { nk_input_char(base::ctx(), c); }

    inline void glyph(const nk_glyph glyph) noexcept {
      nk_input_glyph(base::ctx(), glyph);
    }

    inline void unicode(nk_rune rune) noexcept {
      nk_input_unicode(base::ctx(), rune);
    }
  };
};

struct window_query_interface {
  template <class B>
  class type : public B {
    using base = B;

   public:
    using vec2_t = typename base::vec2_t;
    using rect_t = typename base::rect_t;

    [[nodiscard]] inline bool is_collapsed(const char* id) noexcept {
      return nk_window_is_collapsed(base::ctx(), id) == nk_true;
    }

    [[nodiscard]] inline bool is_closed(const char* id) noexcept {
      return nk_window_is_closed(base::ctx(), id) == nk_true;
    }

    [[nodiscard]] inline bool is_hidden(const char* id) noexcept {
      return nk_window_is_hidden(base::ctx(), id) == nk_true;
    }

    [[nodiscard]] inline bool is_active(const char* id) noexcept {
      return nk_window_is_active(base::ctx(), id) == nk_true;
    }

    [[nodiscard]] inline bool is_any_hovered() noexcept {
      return nk_window_is_any_hovered(base::ctx()) == nk_true;
    }

    [[nodiscard]] inline bool is_any_active() noexcept {
      return nk_item_is_any_active(base::ctx()) == nk_true;
    }

    inline void set_bounds(const char* id, rect_t rect) noexcept {
      nk_window_set_bounds(base::ctx(), id, rect);
    }

    inline void set_position(const char* id, vec2_t pos) noexcept {
      nk_window_set_position(base::ctx(), id, pos);
    }

    inline void set_size(const char* id, vec2_t size) noexcept {
      nk_window_set_size(base::ctx(), id, size);
    }

    inline void set_focus(const char* id) noexcept {
      nk_window_set_focus(base::ctx(), id);
    }

    inline void close(const char* id) noexcept {
      nk_window_close(base::ctx(), id);
    }

    inline void collapse(const char* id, nk_collapse_states states) noexcept {
      nk_window_collapse(base::ctx(), id, states);
    }

    inline void collapse_if(const char* id,
                            nk_collapse_states states,
                            int cond) noexcept {
      nk_window_collapse_if(base::ctx(), id, states, cond);
    }

    inline void show(const char* id, nk_show_states states) noexcept {
      nk_window_show(base::ctx(), id, states);
    }

    inline void show_if(const char* id,
                        nk_show_states states,
                        int cond) noexcept {
      nk_window_show_if(base::ctx(), id, states, cond);
    }

    inline scroll_t get_group_scroll(const char* id) {
      scroll_t scr{};
      nk_group_get_scroll(base::ctx(), id, &scr.x, &scr.y);
      return scr;
    }

    inline unsigned int get_group_x_scroll() {
      unsigned int i{};
      nk_group_get_scroll(base::ctx(), id, &i, nullptr);
      return i;
    }

    inline unsigned int get_group_y_scroll() {
      unsigned int i{};
      nk_group_get_scroll(base::ctx(), id, nullptr, &i);
      return i;
    }
  };
};
}  // namespace detail

class row;
class space;
class group;

namespace detail {
struct layout_interface {
  template <class B>
  class type : public B {
   protected:
    using base = B;
    using rect_t = typename base::rect_t;

   public:
    inline void set_min_row_height(float height) noexcept {
      nk_layout_set_min_row_height(base::ctx(), height);
    }

    inline void reset_min_row_height() noexcept {
      nk_layout_reset_min_row_height(base::ctx());
    }

    [[nodiscard]] inline rect_t widget_bounds() noexcept {
      return nk_layout_widget_bounds(base::ctx());
    }

    [[nodiscard]] inline float ratio_from_pixel(float pixel_width) noexcept {
      return nk_layout_ratio_from_pixel(base::ctx(), pixel_width);
    }

    inline void row_dynamic(float height, int cols) noexcept {
      nk_layout_row_dynamic(base::ctx(), height, cols);
    }

    inline void row_static(float height, int item_width, int cols) noexcept {
      nk_layout_row_static(base::ctx(), height, item_width, cols);
    }

    inline void row_begin(nk_layout_format format,
                          float row_height,
                          int cols) noexcept {
      nk_layout_row_static(base::ctx(), format, row_height, cols);
    }

    inline void row_end() noexcept { nk_layout_row_end(base::ctx()); }

    template <class F>
    void
    with_row(nk_layout_format fmt, float row_height, int cols, F&& f) noexcept(
        noexcept(std::forward<F>(f)(std::declval<class row>())));

    inline void row(nk_layout_format fmt,
                    float height,
                    int cols,
                    const float* ratio) noexcept {
      nk_layout_row(base::ctx(), fmt, height, cols, ratio);
    }

    inline void row_template_begin(float row_height) noexcept {
      nk_layout_row_template_begin(base::ctx(), row_height);
    }

    inline void row_template_end() noexcept {
      nk_layout_row_template_end(base::ctx());
    }

    inline void row_template_push_variable(float min_width) noexcept {
      nk_layout_row_template_push_variable(base::ctx(), min_width);
    }

    inline void row_template_push_static(float width) noexcept {
      nk_layout_row_template_push_static(base::ctx(), width);
    }

    inline void row_template_push_dynamic() noexcept {
      nk_layout_row_template_push_dynamic(base::ctx());
    }

    inline void space_begin(nk_layout_format fmt,
                            float height,
                            int widget_count) noexcept {
      nk_layout_space_begin(base::ctx(), fmt, height, widget_count);
    }

    inline void space_end() noexcept { nk_layout_space_end(base::ctx()); }

    template <class F>
    inline void with_space(
        nk_layout_format fmt,
        float height,
        int widget_count,
        F&& f) noexcept(noexcept(std::
                                     forward<F>(f)(
                                         std::declval<class space>())));

    inline void group_begin(const char* title, nk_flags flags) noexcept {
      nk_group_begin(base::ctx(), title, flags);
    }

    inline void group_begin(const char* id,
                            const char* title,
                            nk_flags flags) noexcept {
      nk_group_begin_titled(base::ctx(), id, title, flags);
    }

    inline void group_scrolled_begin(nk_uint* x_offset,
                                     nk_uint* y_offset,
                                     const char* title,
                                     nk_flags flags) noexcept {
      nk_group_scrolled_offset_begin(
          base::ctx(), x_offset, y_offset, title, flags);
    }

    inline void group_scrolled_begin(nk_scroll* offset,
                                     const char* title,
                                     nk_flags flags) noexcept {
      nk_group_scrolled_begin(base::ctx(), offset, title, flags);
    }

    inline void group_end() noexcept { nk_group_end(base::ctx()); }
    inline void group_scrolled_end() noexcept {
      nk_group_scrolled_end(base::ctx());
    }

   private:
    template <class F>
    constexpr static inline bool noexcept_begin_group = noexcept(
        std::forward<F>(std::declval<F>())(std::declval<class group>()));

    template <class F>
    void instanciate_group(F&& f) noexcept(
        noexcept(std::forward<F>(f)(std::declval<class group>())));

    template <class F,
              class... Args,
              class G = void (type::*)(std::decay_t<Args>...)>
    void begin_group_impl(F&& f,
                          G g,
                          void (type::*h)(),
                          Args&&... args) noexcept(noexcept_begin_group<F>) {
      (this->*g)(std::forward<Args>(args)...);
      try {
        instanciate_group(std::forward<F>(f));
      }
      catch (...) {
        (this->*h)();
        throw;
      }
      group_end();
    }

   public:
    template <class F>
    inline void with_group(const char* title,
                           nk_flags flags,
                           F&& f) noexcept(noexcept(noexcept_begin_group<F>)) {
      begin_group_impl(std::forward<F>(f),
                       &type::group_begin,
                       &type::group_end,
                       title,
                       flags);
    }

    template <class F>
    inline void with_group(nk_uint* x_offset,
                           nk_uint* y_offset,
                           const char* title,
                           nk_flags flags,
                           F&& f) noexcept(noexcept(noexcept_begin_group<F>)) {
      begin_group_impl(std::forward<F>(f),
                       &type::group_scrolled_begin,
                       &type::group_scrolled_end,
                       x_offset,
                       y_offset,
                       title,
                       flags);
    }

    template <class F>
    inline void with_group(const char* id,
                           const char* title,
                           nk_flags flags,
                           F&& f) noexcept(noexcept_begin_group<F>) {
      begin_group_impl(std::forward<F>(f),
                       &type::group_begin,
                       &type::group_end,
                       id,
                       title,
                       flags);
    }

    template <class F>
    inline void with_group(nk_scroll* scroll,
                           const char* title,
                           nk_flags flags,
                           F&& f) noexcept(noexcept(noexcept_begin_group<F>)) {
      begin_group_impl(std::forward<F>(f),
                       &type::group_scrolled_begin,
                       &type::group_scrolled_end,
                       scroll,
                       title,
                       flags);
    }
  };
};
struct row_interface {
  template <class T>
  class type : public T {
    using base = T;

   public:
    inline void push(float value) noexcept {
      nk_layout_row_push(base::ctx(), value);
    }
  };
};

struct space_interface {
  template <class B>
  struct type : B {
    using base = B;
    using rect_t = typename B::rect_t;
    using vec2_t = typename B::vec2_t;

    inline void push(rect_t rect) noexcept {
      nk_layout_space_push(base::ctx(), rect);
    }

    inline rect_t bounds() noexcept {
      return nk_layout_space_bounds(base::ctx());
    }

    inline vec2_t space_to_screen(vec2_t vec) noexcept {
      return nk_layout_space_to_screen(base::ctx(), vec);
    }

    inline vec2_t space_to_local(vec2_t vec) noexcept {
      return nk_layout_space_to_local(base::ctx(), vec);
    }

    inline rect_t space_to_screen(rect_t vec) noexcept {
      return nk_layout_space_rect_to_screen(base::ctx(), vec);
    }

    inline rect_t space_to_local(rect_t vec) noexcept {
      return nk_layout_space_rect_to_local(base::ctx(), vec);
    }
  };
};

struct group_interface {
  template <class B>
  struct type : B {
    using base = B;
  };
};
}  // namespace detail

namespace detail {
template <class T>
using input_mixin =
    dpsg::mixin<T, input_interface, window_query_interface, self_interface>;
}  // namespace detail

// NOLINTNEXTLINE(fuchsia-multiple-inheritance) implementation detail
class input_handler : public detail::input_mixin<input_handler> {
 public:
  [[nodiscard]] constexpr nk_context& ctx() const { return *_ctx; }

 private:
  friend class context;
  constexpr explicit input_handler(nk_context* ctx) noexcept : _ctx{ctx} {}
  nk_context* _ctx;
};

namespace detail {

template <class T, class S>
using sub_mixin = dpsg::mixin<T,
                              window_query_interface,
                              window_interface,
                              layout_interface,
                              S,
                              self_interface>;
template <class T>
using row_mixin = sub_mixin<T, row_interface>;

template <class T>
using space_mixin = sub_mixin<T, space_interface>;

template <class T>
using group_mixin = sub_mixin<T, group_interface>;
}  // namespace detail

class row : public detail::row_mixin<row> {
  [[nodiscard]] constexpr nk_context& ctx() const { return *_ctx; }

 private:
  template <class B>
  friend class detail::layout_interface::type;
  template <class B>
  friend struct detail::self_interface::type;
  constexpr explicit row(nk_context* ctx) noexcept : _ctx{ctx} {}
  nk_context* _ctx;
};

class space : public detail::space_mixin<row> {
  [[nodiscard]] constexpr nk_context& ctx() const { return *_ctx; }

 private:
  template <class B>
  friend class detail::layout_interface::type;
  template <class B>
  friend struct detail::self_interface::type;
  constexpr explicit space(nk_context* ctx) noexcept : _ctx{ctx} {}
  nk_context* _ctx;
};

class group : public detail::group_mixin<group> {
  [[nodiscard]] constexpr nk_context& ctx() const { return *_ctx; }

 private:
  template <class B>
  friend class detail::layout_interface::type;
  template <class B>
  friend struct detail::self_interface::type;
  constexpr explicit group(nk_context* ctx) noexcept : _ctx{ctx} {}
  nk_context* _ctx;
};

namespace detail {
template <class B>
template <class F>
void layout_interface::type<B>::with_row(
    nk_layout_format fmt,
    float row_height,
    int cols,
    F&& f) noexcept(noexcept(std::forward<F>(f)(std::declval<class row>()))) {
  row_begin(fmt, row_height, cols);
  try {
    using r = class row;
    std::forward<F>(f)(r{base::ctx()});
  }
  catch (...) {
    row_end();
    throw;
  }
  row_end();
}

template <class B>
template <class F>
void layout_interface::type<B>::with_space(
    nk_layout_format fmt,
    float height,
    int widget_count,
    F&& f) noexcept(noexcept(std::forward<F>(f)(std::declval<class space>()))) {
  space_begin(fmt, height, widget_count);
  try {
    using s = class space;
    std::forward<F>(f)(s{base::ctx()});
  }
  catch (...) {
    space_end();
    throw;
  }
  space_end();
}

template <class B>
template <class F>
void layout_interface::type<B>::instanciate_group(F&& f) noexcept(
    noexcept(std::forward<F>(f)(std::declval<class group>()))) {
  using g = class group;
  std::forward<F>(f)(g{base::ctx()});
}
}  // namespace detail

namespace detail {
template <class T>
using window_mixin = dpsg::mixin<T,
                                 window_interface,
                                 window_query_interface,
                                 layout_interface,
                                 self_interface>;
}  // namespace detail

class window : public detail::window_mixin<window> {
 public:
  [[nodiscard]] constexpr nk_context& ctx() const { return *_ctx; }

 private:
  friend class context;
  constexpr explicit window(nk_context* ctx) noexcept : _ctx{ctx} {}
  nk_context* _ctx;
};

enum class panel_flags : nk_flags {
  none = 0,
  border = NK_WINDOW_BORDER,
  movable = NK_WINDOW_MOVABLE,
  scalable = NK_WINDOW_SCALABLE,
  closable = NK_WINDOW_CLOSABLE,
  minimizable = NK_WINDOW_MINIMIZABLE,
  no_scrollbar = NK_WINDOW_NO_SCROLLBAR,
  title = NK_WINDOW_TITLE,
  scroll_auto_hide = NK_WINDOW_SCROLL_AUTO_HIDE,
  background = NK_WINDOW_BACKGROUND,
  scale_left = NK_WINDOW_SCALE_LEFT,
  no_input = NK_WINDOW_NO_INPUT,
};

[[nodiscard]] constexpr inline panel_flags operator|(panel_flags lhs,
                                                     panel_flags rhs) {
  return static_cast<panel_flags>(static_cast<nk_flags>(lhs) |
                                  static_cast<nk_flags>(rhs));
}

namespace detail {
template <class T>
using context_mixin =
    dpsg::mixin<T, input_interface, window_query_interface, self_interface>;
}  // namespace detail

// NOLINTNEXTLINE(fuchsia-multiple-inheritance) implementation detail
class context : public detail::context_mixin<context> {
 public:
  using rect_t = struct nk_rect;
  using type = nk_context;
  using reference = nk_context&;
  using const_reference = const nk_context&;

  inline explicit context(const nk_user_font* font = nullptr) noexcept {
    nk_init_default(&_ctx, font);
  }

  context(const context&) = delete;
  context& operator=(const context&) = delete;

  inline context(context&& ctx) noexcept : _ctx(std::exchange(ctx._ctx, {})) {}
  inline context& operator=(context&& ctx) noexcept {
    context(std::move(ctx)).swap(*this);
    return *this;
  }

  inline void swap(context& ctx) {
    using std::swap;
    swap(_ctx, ctx._ctx);
  }

  inline ~context() noexcept { nk_free(&_ctx); }

  [[nodiscard]] inline constexpr const_reference ctx() const noexcept {
    return _ctx;
  }

  [[nodiscard]] inline constexpr reference ctx() noexcept { return _ctx; }

  inline void clear() noexcept { nk_clear(&_ctx); }

  inline void input_begin() noexcept { nk_input_begin(&_ctx); }

  inline void input_end() noexcept { nk_input_end(&_ctx); }

  template <class F>
  inline void handle_input(F&& f) noexcept(
      noexcept(std::forward<F>(f)(std::declval<input_handler>()))) {
    input_begin();
    try {
      std::forward<F>(f)(input_handler{&_ctx});
    }
    catch (...) {
      input_end();
      throw;
    }
    input_end();
  }

  class const_iterator {
    using difference_type = std::ptrdiff_t;
    using value_type = const nk_command;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    const_iterator& operator++() noexcept {
      _cmd = nk__next(_ctx, _cmd);
      return *this;
    }

    [[nodiscard]] constexpr inline bool operator==(
        const const_iterator& iter) const noexcept {
      return _cmd == iter._cmd;
    }

    [[nodiscard]] constexpr inline bool operator!=(
        const const_iterator& iter) const noexcept {
      return (*this == iter);
    }

    reference operator*() const noexcept { return *_cmd; }

    pointer operator->() const noexcept { return _cmd; }

   private:
    inline constexpr const_iterator(nk_context* ctx, pointer cmd) noexcept
        : _ctx(ctx), _cmd{cmd} {}

    nk_context* _ctx;
    pointer _cmd;
    friend class context;
  };

  inline const_iterator begin() noexcept {
    const nk_command* cmd = nk__begin(&_ctx);
    return const_iterator{&_ctx, cmd};
  }

  static inline const_iterator end() noexcept {
    return const_iterator{nullptr, nullptr};
  }

  inline bool begin_window(const char* title,
                           rect_t bounds,
                           nk_flags flags) noexcept {
    return nk_begin(&_ctx, title, bounds, flags) == nk_true;
  }

  inline bool begin_window(const char* identifier,
                           const char* title,
                           rect_t bounds,
                           nk_flags flags) noexcept {
    return nk_begin_titled(&_ctx, identifier, title, bounds, flags) == nk_true;
  }

  inline void end_window() noexcept { nk_end(&_ctx); }

  template <class F>
  inline bool with_window(
      const char* title,
      rect_t bounds,
      panel_flags flags,
      F&& f) noexcept(noexcept(std::forward<F>(f)(std::declval<window>()))) {
    if (begin_window(title, bounds, static_cast<nk_flags>(flags))) {
      try {
        std::forward<F>(f)(window{&_ctx});
      }
      catch (...) {
        end_window();
        throw;
      }
      end_window();
      return true;
    }
    return false;
  }

  template <class F>
  inline bool with_window(
      const char* identifier,
      const char* title,
      rect_t bounds,
      panel_flags flags,
      F&& f) noexcept(noexcept(std::forward<F>(f)(std::declval<window>()))) {
    if (begin_window(identifier, title, bounds, static_cast<nk_flags>(flags))) {
      try {
        std::forward<F>(f)(window{&_ctx});
      }
      catch (...) {
        end_window();
        throw;
      }
      end_window();
      return true;
    }
    return false;
  }

  inline nk_window* find_window(const char* identifier) {
    return nk_window_find(&_ctx, identifier);
  }

 private:
  type _ctx{};
};

}  // namespace nk

namespace dpsg {
class nk_gl3_backend {
 public:
  nk_gl3_backend()
      : _prog{_generate().value()},
        _texture_unif{gl::get_uniform_location(_prog.id(), "Texture")},
        _projection_unif(gl::get_uniform_location(_prog.id(), "ProjMtx")) {
    _init_nk();
  }
  nk_gl3_backend(const nk_gl3_backend&) = delete;
  nk_gl3_backend(nk_gl3_backend&&) noexcept = default;
  nk_gl3_backend& operator=(const nk_gl3_backend&) = delete;
  nk_gl3_backend& operator=(nk_gl3_backend&&) noexcept = default;

  ~nk_gl3_backend() noexcept {
    nk_buffer_free(&_cmds);
    gl::delete_texture(_texture);
  }

  template <class T>
  inline void upload_atlas(const T* image, gl::width w, gl::height h) {
    gl::gen_texture(_texture);
    gl::bind_texture(gl::texture_target::_2d, _texture);
    gl::tex_parameter(gl::texture_target::_2d, gl::min_filter::linear);
    gl::tex_parameter(gl::texture_target::_2d, gl::mag_filter::linear);
    gl::tex_image_2D(
        gl::texture_image_target::_2d, w, h, gl::image_format::rgba, image);
  }

  void load_font(nk_context* ctx,
                 const char* filepath,
                 float height,
                 const struct nk_font_config* cfg = nullptr) {
    nk_font_atlas atlas{};
    nk_font_atlas_init_default(&atlas);
    nk_font_atlas_begin(&atlas);
    auto* val = nk_font_atlas_add_from_file(&atlas, filepath, height, cfg);

    const void* image{};
    int w{};
    int h{};
    image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    upload_atlas(static_cast<const gl::ubyte_t*>(image),
                 gl::width{static_cast<gl::uint_t>(w)},
                 gl::height{static_cast<gl::uint_t>(h)});
    nk_font_atlas_end(
        &atlas, nk_handle_id(static_cast<int>(_texture.value)), &_null);
    if (atlas.default_font != nullptr)
      nk_style_set_font(ctx, &atlas.default_font->handle);

    nk_style_load_all_cursors(ctx, static_cast<nk_cursor*>(atlas.cursors));
    nk_style_set_font(ctx, &val->handle);
  }

  void render(nk_context* ctx,
              gl::width window_width,
              gl::height window_height,
              gl::byte_size max_vertex_buffer,
              gl::byte_size max_element_buffer,
              nk_anti_aliasing anti_aliasing,
              struct nk_vec2 fb_scale) {
    gl::enable(gl::capability::blend);

    // clang-format off
    gl::mat_t<4, 4> ortho = {
        2.F, 0.F, 0.F, 0.F,
        0.F, -2.F, 0.F, 0.F,
        0.F, 0.F, -1.F, 0.F,
        -1.F, 1.F, 0.F, 1.F,
    };
    // clang-format on

    ortho[{0, 0}] /= static_cast<gl::float_t>(window_width.value);
    ortho[{1, 1}] /= static_cast<gl::float_t>(window_height.value);

    gl::blend_equation(gl::blend_mode::add);
    gl::blend_func(gl::blend_factor::src_alpha,
                   gl::blend_factor::one_minus_src_alpha);
    gl::disable(gl::capability::cull_face);
    gl::disable(gl::capability::depth_test);
    gl::enable(gl::capability::scissor_test);
    gl::active_texture(gl::texture_name::_0);
    _prog.use();
    gl::uniform(_texture_unif, 0U);
    gl::uniform(_projection_unif, ortho);

    _vao.bind();
    _vbo.bind();
    _ebo.bind();
    gl::buffer_data(
        gl::buffer_type::array, max_vertex_buffer, gl::data_hint::stream_draw);
    gl::buffer_data(gl::buffer_type::element_array,
                    max_element_buffer,
                    gl::data_hint::stream_draw);
    void* vertices =
        gl::map_buffer(gl::buffer_type::array, gl::access::write_only);
    void* elements =
        gl::map_buffer(gl::buffer_type::element_array, gl::access::write_only);
    nk_convert_config config{};
    config.vertex_layout =
        static_cast<const nk_draw_vertex_layout_element*>(vertex_layout);
    config.vertex_size = sizeof(vertex);
    config.vertex_alignment = alignof(vertex);
    config.null = _null;
    config.circle_segment_count = 22;
    config.curve_segment_count = 22;
    config.arc_segment_count = 22;
    config.global_alpha = 1.0f;
    config.shape_AA = anti_aliasing;
    config.line_AA = anti_aliasing;

    nk_buffer vbuf;
    nk_buffer ebuf;
    nk_buffer_init_fixed(&vbuf, vertices, max_vertex_buffer.value);
    nk_buffer_init_fixed(&ebuf, elements, max_element_buffer.value);
    nk_convert(ctx, &_cmds, &vbuf, &ebuf, &config);

    gl::unmap_buffer(gl::buffer_type::array);
    gl::unmap_buffer(gl::buffer_type::element_array);

    const nk_draw_command* cmd{nullptr};
    gl::offset offset{0};
    nk_draw_foreach(cmd, ctx, &_cmds) {
      if (cmd->elem_count == 0) {
        continue;
      }
      gl::bind_texture(
          gl::texture_target::_2d,
          gl::texture_id{static_cast<gl::uint_t>(cmd->texture.id)});
      gl::scissor(
          gl::x{static_cast<gl::int_t>(cmd->clip_rect.x * fb_scale.x)},
          gl::y{(static_cast<gl::int_t>(window_height.value) -
                 static_cast<gl::int_t>(cmd->clip_rect.y + cmd->clip_rect.h)) *
                static_cast<gl::int_t>(fb_scale.y)},
          gl::width{static_cast<gl::uint_t>(cmd->clip_rect.w * fb_scale.x)},
          gl::height{static_cast<gl::uint_t>(cmd->clip_rect.h * fb_scale.y)});
      gl::draw_elements<gl::ushort_t>(
          gl::drawing_mode::triangles,
          gl::element_count{static_cast<gl::int_t>(cmd->elem_count)},
          offset);
      offset.value += cmd->elem_count;
    }
    nk_clear(ctx);
    nk_buffer_clear(&_cmds);
  }

 private:
  explicit nk_gl3_backend(program prog) noexcept : _prog{std::move(prog)} {
    _init_nk();
    _vao.bind();
    _vbo.bind();
    _ebo.bind();
    gl::vertex_attrib_pointer<float>(
        gl::index{0},
        gl::element_count{2},
        gl::byte_stride{sizeof(vertex)},
        gl::byte_offset{offsetof(vertex, position)});
    gl::vertex_attrib_pointer<float>(gl::index{1},
                                     gl::element_count{2},
                                     gl::byte_stride{sizeof(vertex)},
                                     gl::byte_offset{offsetof(vertex, uv)});
    gl::vertex_attrib_pointer<nk_byte>(
        gl::index{2},
        gl::element_count{4},
        gl::normalized::yes,
        gl::byte_stride{sizeof(vertex)},
        gl::byte_offset{offsetof(vertex, color)});
  }

  program _prog;
  nk_buffer _cmds;
  nk_draw_null_texture _null;
  vertex_array _vao;
  element_buffer _ebo;
  vertex_buffer _vbo;
  gl::texture_id _texture{0};
  gl::uniform_location _texture_unif{-1};
  gl::uniform_location _projection_unif{-1};

  static constexpr inline vs_source vertex_shader_source{
      "#version 330 core"
      "uniform mat4 ProjMtx;\n"
      "layout(location = 0) in vec2 Position;\n"
      "layout(location = 1) in vec2 TexCoord;\n"
      "layout(location = 2) in vec4 Color;\n"
      "out vec2 Frag_UV;\n"
      "out vec4 Frag_Color;\n"
      "void main() {\n"
      "   Frag_UV = TexCoord;\n"
      "   Frag_Color = Color;\n"
      "   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
      "}\n"};
  static constexpr inline fs_source fragment_shader_source{
      "#version 330 core"
      "precision mediump float;\n"
      "uniform sampler2D Texture;\n"
      "in vec2 Frag_UV;\n"
      "in vec4 Frag_Color;\n"
      "out vec4 Out_Color;\n"
      "void main(){\n"
      "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
      "}\n"};
  struct vertex {
    float position[2];  // NOLINT
    float uv[2];        // NOLINT
    nk_byte color[4];   // NOLINT
  };

  // NOLINTNEXTLINE
  constexpr static nk_draw_vertex_layout_element vertex_layout[] = {
      {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, offsetof(vertex, position)},
      {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, offsetof(vertex, uv)},
      {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, offsetof(vertex, color)},
      {NK_VERTEX_LAYOUT_END}};
  using vs_layout =
      packed<gl::vec_t<2, float>, gl::vec_t<2, float>, gl::vec_t<4, nk_byte>>;

  // NOLINTNEXTLINE(bugprone-exception-escape) spurious
  static inline result<program, gl_error> _generate() noexcept {
    return dpsg::vertex_shader::create(vertex_shader_source)
        // NOLINTNEXTLINE(bugprone-exception-escape) spurious
        .then([](vertex_shader&& vs) noexcept {
          return dpsg::fragment_shader::create(fragment_shader_source)
              .then([&vs](fragment_shader&& fs) noexcept {
                return result<program, gl_error>{
                    program::create(std::move(vs), std::move(fs))};
              });
        });
  }

  void _init_nk() noexcept { nk_buffer_init_default(&_cmds); }

 public:
  // NOLINTNEXTLINE(bugprone-exception-escape) spurious
  inline static result<nk_gl3_backend, gl_error> create() noexcept {
    return _generate().map([](program&& prog) noexcept {
      return nk_gl3_backend{std::move(prog)};
    });
  }
};  // namespace dpsg

}  // namespace dpsg

namespace nk {
template <class Backend>
class nuklear_context : Backend {
 public:
 private:
};

}  // namespace nk

int main() {
  dpsg::within_glfw_context([] {
    nk::context ctx;
    ctx.with_window(
        "title",
        nk_rect(0, 0, 10, 10),
        nk::panel_flags::border,
        [](nk::window window) {
          auto p = window.panel();
          auto c1 = window.content_region_min();
          auto c2 = window.content_region();
          auto c3 = window.content_region_max();
          auto c4 = window.canvas();
          auto s = window.scroll();
          auto sx = window.x_scroll();
          auto sy = window.y_scroll();
          auto b = window.has_focus() || window.is_any_hovered();
          auto b2 = window.is_collapsed("const char *id") ||
                    window.is_closed("const char *id") ||
                    window.is_hidden("const char *id") ||
                    window.is_active("") || window.is_any_active();
          window.set_position("", nk_vec2(1, 2));
          window.set_size("", nk_vec2(1, 2));
          window.set_bounds("", nk_rect(1, 2, 1, 2));
          window.set_focus("const char *id");
          window.set_scroll(1, 2);
          window.close("");
          window.collapse("", NK_MAXIMIZED);
          window.collapse_if("", NK_MAXIMIZED, 0);
          window.show("", nk_show_states::NK_SHOWN);
          window.show_if("", nk_show_states::NK_SHOWN, 0);
          window.set_min_row_height(0.F);
          window.reset_min_row_height();
          auto bounds = window.widget_bounds();
          float pf = window.ratio_from_pixel(1);
          window.row_dynamic(pf, 0);
          window.row_static(0.F, 0, 0);
          window.with_row(
              nk_layout_format::NK_DYNAMIC, 0.F, 0, [](nk::row row) {
                row.push(0.F);
              });
          window.row_template_begin(0.F);
          window.row_template_end();
          window.row_template_push_static(bounds.x);
          window.row_template_push_variable(c1.x);
          window.row_template_push_static(0.F);
          window.with_space(
              nk_layout_format::NK_DYNAMIC,
              c1.y,
              *p->offset_x,
              [&](nk::space sp) {
                sp.push(c4->clip);
                struct nk_rect r =
                    sp.space_to_screen(sp.space_to_local(nk_rect(0, 0, 0, 0)));
                struct nk_vec2 v =
                    sp.space_to_screen(sp.space_to_local(nk_vec2(0, 0)));
              });
          window.with_group("", "", NK_MAXIMIZED, [](nk::group) {});
          window.with_group("", NK_MAXIMIZED, [](nk::group) {});
          unsigned int i{};
          nk_scroll scr{};
          window.with_group(&i, &i, "", NK_MAXIMIZED, [](nk::group) {});
          window.with_group(&scr, "", NK_MAXIMIZED, [](nk::group) {});
        });
    ctx.handle_input([](nk::input_handler input) { input.motion(1, 1); });
  });

  return 0;
}