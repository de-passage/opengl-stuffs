#ifndef GUARD_NK_ENUMS_HEADER
#define GUARD_NK_ENUMS_HEADER

#include "./config.hpp"

namespace nk {

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
};  // enum class panel_flags

[[nodiscard]] constexpr inline panel_flags operator|(panel_flags lhs,
                                                     panel_flags rhs) {
  return static_cast<panel_flags>(static_cast<nk_flags>(lhs) |
                                  static_cast<nk_flags>(rhs));
}

}  // namespace nk

#endif  // GUARD_NK_ENUMS_HEADER