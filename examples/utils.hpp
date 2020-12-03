#ifndef GUARD_EXAMPLE_UTILS_HPP
#define GUARD_EXAMPLE_UTILS_HPP

#include <utility>

namespace dpsg {
class window;
} // namespace dpsg

void close(dpsg::window &w);

template <class F> auto ignore(F &&f) {
  return
      [f = std::forward<F>(f)]([[maybe_unused]] dpsg::window &unused) { f(); };
}

#endif // GUARD_EXAMPLE_UTILS_HPP