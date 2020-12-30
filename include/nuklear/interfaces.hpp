#ifndef GUARD_NK_INTERFACES_HEADER
#define GUARD_NK_INTERFACES_HEADER

#include "./config.hpp"
#include "meta/mixin.hpp"

namespace nk {

class row;
class space;
class group;

struct scroll_t {
  unsigned int x;
  unsigned int y;
};  // struct scroll_t

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
      return static_cast<crtp_cptr>(this);
    }

    [[nodiscard]] constexpr auto* ctx() const noexcept {
      return &self()->ctx();
    }

    [[nodiscard]] constexpr auto* ctx() noexcept { return &self()->ctx(); }

    [[nodiscard]] constexpr auto& style() noexcept { return &ctx()->style; }

    [[nodiscard]] constexpr const auto& style() const noexcept {
      return &ctx()->style;
    }
  };  // struct type
};    // struct self_interface

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
  };  // struct type
};    // struct window_interface

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

    [[nodiscard]] inline nk_mouse& mouse() noexcept {
      return base::ctx()->input.mouse;
    }

    [[nodiscard]] inline const nk_mouse& mouse() const noexcept {
      return base::ctx()->input.mouse;
    }
  };  // struct type
};    // struct window_interface

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

    inline nk_window* find_window(const char* identifier) {
      return nk_window_find(base::ctx(), identifier);
    }
  };  // struct type
};    // struct window_query_interface

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
    inline void begin_group_impl(
        F&& f,
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
  };  // struct type
};    // struct layout_interface

struct row_interface {
  template <class T>
  class type : public T {
    using base = T;

   public:
    inline void push(float value) noexcept {
      nk_layout_row_push(base::ctx(), value);
    }
  };  // struct type
};    // struct row_interface

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
  };  // struct type
};    // struct space_interface

struct group_interface {
  template <class B>
  struct type : B {
    using base = B;
  };  // struct type
};    // struct group_interface

namespace detail {
template <class T>
using input_mixin =
    dpsg::mixin<T, input_interface, window_query_interface, self_interface>;

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

template <class T>
using window_mixin = dpsg::mixin<T,
                                 window_interface,
                                 window_query_interface,
                                 layout_interface,
                                 self_interface>;
}  // namespace detail

class input_handler : public detail::input_mixin<input_handler> {
 public:
  [[nodiscard]] constexpr nk_context& ctx() const { return *_ctx; }

 private:
  friend class context;
  constexpr explicit input_handler(nk_context* ctx) noexcept : _ctx{ctx} {}
  nk_context* _ctx;
};  // class input_handler

class row : public detail::row_mixin<row> {
  [[nodiscard]] constexpr nk_context& ctx() const { return *_ctx; }

 private:
  template <class B>
  friend class layout_interface::type;
  template <class B>
  friend struct self_interface::type;
  constexpr explicit row(nk_context* ctx) noexcept : _ctx{ctx} {}
  nk_context* _ctx;
};  // class row

class space : public detail::space_mixin<row> {
  [[nodiscard]] constexpr nk_context& ctx() const { return *_ctx; }

 private:
  template <class B>
  friend class layout_interface::type;
  template <class B>
  friend struct self_interface::type;
  constexpr explicit space(nk_context* ctx) noexcept : _ctx{ctx} {}
  nk_context* _ctx;
};  // class space

class group : public detail::group_mixin<group> {
  [[nodiscard]] constexpr nk_context& ctx() const { return *_ctx; }

 private:
  template <class B>
  friend class layout_interface::type;
  template <class B>
  friend struct self_interface::type;
  constexpr explicit group(nk_context* ctx) noexcept : _ctx{ctx} {}
  nk_context* _ctx;
};  // class group

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
}  // function layout_interface::type::with_row

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
}  // function layout_interface::type::with_space

template <class B>
template <class F>
void layout_interface::type<B>::instanciate_group(F&& f) noexcept(
    noexcept(std::forward<F>(f)(std::declval<class group>()))) {
  using g = class group;
  std::forward<F>(f)(g{base::ctx()});
}  // function layout_interface::type::instanciate_group

class window : public detail::window_mixin<window> {
 public:
  [[nodiscard]] constexpr nk_context& ctx() const { return *_ctx; }

 private:
  friend class context;
  constexpr explicit window(nk_context* ctx) noexcept : _ctx{ctx} {}
  nk_context* _ctx;
};  // class window

}  // namespace nk

#endif  // GUARD_NK_INTERFACES_HEADER