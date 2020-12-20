#ifndef GUARD_NK_STYLE_HEADER
#define GUARD_NK_STYLE_HEADER

#include "config.hpp"
#include "meta/mixin.hpp"

namespace nk {

struct style_interface {
  template <class B>
  struct type : B {
    using base = B;
    using vec2_t = typename B::vec2_t;

    inline void default_style() noexcept { nk_style_default(base::ctx()); }

    inline void style_from_table(const nk_color& color) noexcept {
      nk_style_from_table(base::ctx(), &color);
    }

    inline void load_cursor(nk_style_cursor cursor_style,
                            const nk_cursor& cursor) noexcept {
      nk_style_load_cursor(base::ctx(), cursor_style, &cursor);
    }

    inline void load_all_cursors(nk_cursor* cursor) noexcept {
      nk_style_load_all_cursors(base::ctx(), cursor);
    }

    inline static const char* color_name(nk_style_colors color) {
      return nk_style_get_color_by_name(color);
    }

    inline void set_font(const nk_user_font& font) {
      nk_style_set_font(base::ctx(), &font);
    }

    inline bool set_cursor(nk_style_cursor cursor) noexcept {
      return nk_style_set_cursor(base::ctx(), cursor) == nk_true;
    }

    inline void show_cursor() noexcept { nk_style_show_cursor(base::ctx()); }

    inline void hide_cursor() noexcept { nk_style_hide_cursor(base::ctx()); }

    inline bool style_push(const nk_user_font& font) noexcept {
      return nk_style_push_font(base::ctx(), &font) == nk_true;
    }

    inline bool style_push(float* f1, float f2) noexcept {
      return nk_style_push_float(base::ctx(), f1, f2) == nk_true;
    }

    inline bool style_push(vec2_t* vec1, vec2_t vec2) noexcept {
      return nk_style_push_vec2(base::ctx(), vec1, vec2) == nk_true;
    }

    inline bool style_push(nk_style_item* item1, nk_style_item item2) noexcept {
      return nk_style_push_style_item(base::ctx(), item1, item2) == nk_true;
    }

    inline bool style_push(nk_flags* f1, nk_flags f2) noexcept {
      return nk_style_push_flags(base::ctx(), f1, f2) == nk_true;
    }

    inline bool style_push(nk_color* color1, nk_color color2) noexcept {
      return nk_style_push_color(base::ctx(), color1, color2) == nk_true;
    }

    inline bool style_pop_font() noexcept {
      return nk_style_pop_font(base::ctx()) == nk_true;
    }

    inline bool style_pop_float() noexcept {
      return nk_style_pop_float(base::ctx()) == nk_true;
    }

    inline bool style_pop_vec2() noexcept {
      return nk_style_pop_vec2(base::ctx()) == nk_true;
    }

    inline bool style_pop_style_item() noexcept {
      return nk_style_pop_style_item(base::ctx()) == nk_true;
    }

    inline bool style_pop_flags() noexcept {
      return nk_style_pop_flags(base::ctx()) == nk_true;
    }

    inline bool style_pop_color() noexcept {
      return nk_style_pop_color(base::ctx()) == nk_true;
    }
  };  // struct type
};    // struct style_interface

}  // namespace nk

#endif  // GUARD_NK_STYLE_HEADER