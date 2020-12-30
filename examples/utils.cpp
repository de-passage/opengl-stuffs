#include "glad/glad.h"

#include "common.hpp"
#include "utils.hpp"
#include "with_window.hpp"

#include <stdexcept>

void close(dpsg::window& w) {
  w.should_close(true);
};

void resize([[maybe_unused]] dpsg::window& unused,
            dpsg::width w,
            dpsg::height h) {
  glViewport(0, 0, w.value, h.value);
}

void error_check(std::string pos) {
  if (auto e = dpsg::gl::get_error()) {
    throw std::runtime_error(std::string{"gl error ("} + std::move(pos) +
                             "): " + e.to_string());
  }
}

bool glad_loader::already_initialized = false;

void glad_loader::initialize_glad() noexcept(false) {
  if (!already_initialized) {
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(  // NOLINT
            glfwGetProcAddress))) {
      throw std::runtime_error("Failed to initialize GLAD");
    }
    already_initialized = true;
  }
}