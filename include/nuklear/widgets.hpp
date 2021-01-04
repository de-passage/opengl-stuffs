#ifndef GUARD_NK_WIDGETS_HPP
#define GUARD_NK_WIDGETS_HPP

#include "./interfaces.hpp"
#include "meta/mixin.hpp"

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

template <class T, std::enable_if_t<detail::is_window_v<T>, int> = 0>
inline bool slider(T& ctx, int min, int& value, int max, int step) noexcept {
  return nk_slider_int(&ctx.ctx(), min, &value, max, step) == nk_true;
}

}  // namespace nk::widget

#endif  // GUARD_NK_WIDGETS_HPP