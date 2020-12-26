
#ifndef GUARD_DPSG_INPUT_MOUSE_HEADER
#define GUARD_DPSG_INPUT_MOUSE_HEADER

#include "GLFW/glfw3.h"

namespace dpsg ::input {
enum class mouse : int {
  button_1 = GLFW_MOUSE_BUTTON_1,
  button_2 = GLFW_MOUSE_BUTTON_2,
  button_3 = GLFW_MOUSE_BUTTON_3,
  button_4 = GLFW_MOUSE_BUTTON_4,
  button_5 = GLFW_MOUSE_BUTTON_5,
  button_6 = GLFW_MOUSE_BUTTON_6,
  button_7 = GLFW_MOUSE_BUTTON_7,
  button_8 = GLFW_MOUSE_BUTTON_8,
  left = GLFW_MOUSE_BUTTON_LEFT,
  right = GLFW_MOUSE_BUTTON_RIGHT,
  middle = GLFW_MOUSE_BUTTON_MIDDLE,
  last = GLFW_MOUSE_BUTTON_LAST,
};
}  // namespace dpsg::input
#endif  // GUARD_DPSG_INPUT_MOUSE_HEADER