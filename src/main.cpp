#include <glad/glad.h>

#include "examples/make_window.hpp"

#include "examples/textures.hpp"

#include <array>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

int main() {
  using namespace dpsg;
  using namespace dpsg::input;

  ExecutionStatus r = ExecutionStatus::Failure;

  try {
    r = make_window(texture_example);

  } catch (std::exception &e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unhandled error occured" << std::endl;
  }

  return static_cast<int>(r);
}
