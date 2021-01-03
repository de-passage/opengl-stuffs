#ifndef GUARD_EXAMPLE_UTILS_HPP
#define GUARD_EXAMPLE_UTILS_HPP

#include <string>
#include <utility>
#include "common.hpp"
#include "glm/trigonometric.hpp"

constexpr inline static auto close = [](auto& w) { w.should_close(true); };

template <class F>
auto ignore(F&& f) {
  return [f = std::forward<F>(f)]([[maybe_unused]] auto& unused,
                                  auto&&... args) mutable {
    f(std::forward<decltype(args)>(args)...);
  };
}

struct resize_t {
  template <class W>
  void operator()([[maybe_unused]] W&& unused,
                  dpsg::width w,
                  dpsg::height h) const {
    resize_impl(w, h);
  }

 private:
  static void resize_impl(dpsg::width w, dpsg::height h);
} constexpr static inline resize;

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

template <class C>
struct camera_tracks_cursor {
  explicit camera_tracks_cursor(C& cam) : cam{cam} {}
  C& cam;

  template <class W>
  void operator()(W& wdw, double x, double y) {
    double last_x = x;
    double last_y = y;
    auto callback = [&cam = cam, last_x, last_y](double x, double y) mutable {
      constexpr float sensitivity = glm::radians(0.1F);
      double x_offset = (x - last_x) * sensitivity;
      double y_offset = (last_y - y) * sensitivity;
      last_x = x;
      last_y = y;
      cam.rotate(x_offset, y_offset);
    };
    callback(x, y);
    wdw.set_cursor_pos_callback(ignore(callback));
  }
};

constexpr static inline auto camera_resize = [](auto& cam) {
  return ignore([&cam](dpsg::width w, dpsg::height h) {
    cam.aspect_ratio(w, h);
    glViewport(0, 0, w.value, h.value);
  });
};

#endif  // GUARD_EXAMPLE_UTILS_HPP