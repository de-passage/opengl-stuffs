#ifndef GUARD_DPSG_INPUT_KEYS_HEADER
#define GUARD_DPSG_INPUT_KEYS_HEADER

#include "GLFW/glfw3.h"

namespace dpsg {

namespace input {
enum class status {
  released = GLFW_RELEASE,
  pressed = GLFW_PRESS,
  repeat = GLFW_REPEAT
};

enum class key {
  unknown = GLFW_KEY_UNKNOWN,
  space = GLFW_KEY_SPACE,
  apostrophe = GLFW_KEY_APOSTROPHE,
  comma = GLFW_KEY_COMMA,
  minus = GLFW_KEY_MINUS,
  period = GLFW_KEY_PERIOD,
  slash = GLFW_KEY_SLASH,
  num0 = GLFW_KEY_0,
  num1 = GLFW_KEY_1,
  num2 = GLFW_KEY_2,
  num3 = GLFW_KEY_3,
  num4 = GLFW_KEY_4,
  num5 = GLFW_KEY_5,
  num6 = GLFW_KEY_6,
  num7 = GLFW_KEY_7,
  num8 = GLFW_KEY_8,
  num9 = GLFW_KEY_9,
  semicolon = GLFW_KEY_SEMICOLON,
  equal = GLFW_KEY_EQUAL,
  A = GLFW_KEY_A,
  B = GLFW_KEY_B,
  C = GLFW_KEY_C,
  D = GLFW_KEY_D,
  E = GLFW_KEY_E,
  F = GLFW_KEY_F,
  G = GLFW_KEY_G,
  H = GLFW_KEY_H,
  I = GLFW_KEY_I,
  J = GLFW_KEY_J,
  K = GLFW_KEY_K,
  L = GLFW_KEY_L,
  M = GLFW_KEY_M,
  N = GLFW_KEY_N,
  O = GLFW_KEY_O,
  P = GLFW_KEY_P,
  Q = GLFW_KEY_Q,
  R = GLFW_KEY_R,
  S = GLFW_KEY_S,
  T = GLFW_KEY_T,
  U = GLFW_KEY_U,
  V = GLFW_KEY_V,
  W = GLFW_KEY_W,
  X = GLFW_KEY_X,
  Y = GLFW_KEY_Y,
  Z = GLFW_KEY_Z,
  left_bracket = GLFW_KEY_LEFT_BRACKET,
  backslash = GLFW_KEY_BACKSLASH,
  right_bracket = GLFW_KEY_RIGHT_BRACKET,
  grave_accent = GLFW_KEY_GRAVE_ACCENT,
  world_1 = GLFW_KEY_WORLD_1,
  world_2 = GLFW_KEY_WORLD_2,
  escape = GLFW_KEY_ESCAPE,
  enter = GLFW_KEY_ENTER,
  tab = GLFW_KEY_TAB,
  backspace = GLFW_KEY_BACKSPACE,
  insert = GLFW_KEY_INSERT,
  del = GLFW_KEY_DELETE,
  right = GLFW_KEY_RIGHT,
  left = GLFW_KEY_LEFT,
  down = GLFW_KEY_DOWN,
  up = GLFW_KEY_UP,
  page_up = GLFW_KEY_PAGE_UP,
  page_down = GLFW_KEY_PAGE_DOWN,
  home = GLFW_KEY_HOME,
  end = GLFW_KEY_END,
  caps_lock = GLFW_KEY_CAPS_LOCK,
  scroll_lock = GLFW_KEY_SCROLL_LOCK,
  num_lock = GLFW_KEY_NUM_LOCK,
  print_screen = GLFW_KEY_PRINT_SCREEN,
  pause = GLFW_KEY_PAUSE,
  F1 = GLFW_KEY_F1,
  F2 = GLFW_KEY_F2,
  F3 = GLFW_KEY_F3,
  F4 = GLFW_KEY_F4,
  F5 = GLFW_KEY_F5,
  F6 = GLFW_KEY_F6,
  F7 = GLFW_KEY_F7,
  F8 = GLFW_KEY_F8,
  F9 = GLFW_KEY_F9,
  F10 = GLFW_KEY_F10,
  F11 = GLFW_KEY_F11,
  F12 = GLFW_KEY_F12,
  F13 = GLFW_KEY_F13,
  F14 = GLFW_KEY_F14,
  F15 = GLFW_KEY_F15,
  F16 = GLFW_KEY_F16,
  F17 = GLFW_KEY_F17,
  F18 = GLFW_KEY_F18,
  F19 = GLFW_KEY_F19,
  F20 = GLFW_KEY_F20,
  F21 = GLFW_KEY_F21,
  F22 = GLFW_KEY_F22,
  F23 = GLFW_KEY_F23,
  F24 = GLFW_KEY_F24,
  F25 = GLFW_KEY_F25,
  kp_0 = GLFW_KEY_KP_0,
  kp_1 = GLFW_KEY_KP_1,
  kp_2 = GLFW_KEY_KP_2,
  kp_3 = GLFW_KEY_KP_3,
  kp_4 = GLFW_KEY_KP_4,
  kp_5 = GLFW_KEY_KP_5,
  kp_6 = GLFW_KEY_KP_6,
  kp_7 = GLFW_KEY_KP_7,
  kp_8 = GLFW_KEY_KP_8,
  kp_9 = GLFW_KEY_KP_9,
  kp_decimal = GLFW_KEY_KP_DECIMAL,
  kp_divide = GLFW_KEY_KP_DIVIDE,
  kp_multiply = GLFW_KEY_KP_MULTIPLY,
  kp_subtract = GLFW_KEY_KP_SUBTRACT,
  kp_add = GLFW_KEY_KP_ADD,
  kp_enter = GLFW_KEY_KP_ENTER,
  kp_equal = GLFW_KEY_KP_EQUAL,
  left_shift = GLFW_KEY_LEFT_SHIFT,
  left_control = GLFW_KEY_LEFT_CONTROL,
  left_alt = GLFW_KEY_LEFT_ALT,
  left_super = GLFW_KEY_LEFT_SUPER,
  right_shift = GLFW_KEY_RIGHT_SHIFT,
  right_control = GLFW_KEY_RIGHT_CONTROL,
  right_alt = GLFW_KEY_RIGHT_ALT,
  right_super = GLFW_KEY_RIGHT_SUPER,
  menu = GLFW_KEY_MENU,
  last = GLFW_KEY_LAST
};
} // namespace input

} // namespace dpsg

#endif // GUARD_DPSG_INPUT_KEYS_HEADER