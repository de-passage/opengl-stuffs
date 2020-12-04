#include "utils.hpp"
#include "common.hpp"
#include "window.hpp"

void close(dpsg::window &w) { w.should_close(true); };

void resize([[maybe_unused]] dpsg::window &unused, dpsg::width w,
            dpsg::height h) {
  glViewport(0, 0, w.value, h.value);
}