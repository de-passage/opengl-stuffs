#ifndef GUARD_EXAMPLE_UTILS_HPP
#define GUARD_EXAMPLE_UTILS_HPP

#include "common.hpp"
#include <utility>

namespace dpsg {
class window;
} // namespace dpsg

void close(dpsg::window &w);

template <class F> auto ignore(F &&f) {
  return [f = std::forward<F>(f)]([[maybe_unused]] dpsg::window &unused,
                                  auto &&... args) {
    f(std::forward<decltype(args)>(args)...);
  };
}

void resize([[maybe_unused]] dpsg::window &unused, dpsg::width w,
            dpsg::height h);

#endif // GUARD_EXAMPLE_UTILS_HPP