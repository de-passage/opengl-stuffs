#include <glad/glad.h>

#include "examples/make_window.hpp"

#include "examples/moving_polygon.hpp"
#include "examples/textures.hpp"

#include <exception>
#include <iostream>

int main() {
  using namespace dpsg;

  ExecutionStatus r = ExecutionStatus::Failure;

  try {
    r = make_window(moving_polygon);

  } catch (std::exception &e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unhandled error occured" << std::endl;
  }

  return static_cast<int>(r);
}
