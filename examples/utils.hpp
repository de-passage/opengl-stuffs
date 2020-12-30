#ifndef GUARD_EXAMPLE_UTILS_HPP
#define GUARD_EXAMPLE_UTILS_HPP

#include <string>
#include <utility>
#include "common.hpp"

namespace dpsg {
class window;
}  // namespace dpsg

void close(dpsg::window& w);

template <class F>
auto ignore(F&& f) {
  return [f = std::forward<F>(f)]([[maybe_unused]] dpsg::window& unused,
                                  auto&&... args) {
    f(std::forward<decltype(args)>(args)...);
  };
}

void resize([[maybe_unused]] dpsg::window& unused,
            dpsg::width w,
            dpsg::height h);

void error_check(std::string pos);

struct glad_loader {
 private:
  static bool already_initialized;
  static void initialize_glad() noexcept(false);

 public:
  template <class B>
  struct type : B {
    template <class... Args>
    explicit type(Args&&... args) : B(std::forward<Args>(args)...) {
      this->make_context_current();
      initialize_glad();
    }
  };
};
#endif  // GUARD_EXAMPLE_UTILS_HPP