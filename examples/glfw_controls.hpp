#ifndef GUARD_GLFW_CONTROLS_HEADER
#define GUARD_GLFW_CONTROLS_HEADER

#include "common.hpp"
#include "control_scheme.hpp"
#include "glm/trigonometric.hpp"
#include "input/keys.hpp"
#include "traversecpp/traverse.hpp"

#include "./utils.hpp"

#include <chrono>
#include <utility>

namespace glfw_controls {

struct repeated {};
struct one_time {};
namespace actions {
enum class direction { forward, backward, left, right, up, down };

template <direction Direction>
struct move {
  constexpr explicit inline move(float speed = 0.04) noexcept : speed{speed} {}
  constexpr static inline direction direction{Direction};
  float speed;

  template <class Camera>
  constexpr auto operator()(Camera& camera) const noexcept {
    constexpr auto advance = [](Camera& camera, float speed) {
      return ignore([&camera, speed]() { camera.advance(speed); });
    };
    constexpr auto climb = [](Camera& camera, float speed) {
      return ignore([&camera, speed]() { camera.climb(speed); });
    };
    constexpr auto strafe = [](Camera& camera, float speed) {
      return ignore([&camera, speed]() { camera.strafe(speed); });
    };
    if constexpr (move::direction == direction::forward) {
      return advance(camera, speed);
    }
    else if constexpr (move::direction == direction::backward) {
      return advance(camera, -speed);
    }
    else if constexpr (move::direction == direction::up) {
      return climb(camera, speed);
    }
    else if constexpr (move::direction == direction::down) {
      return climb(camera, -speed);
    }
    else if constexpr (move::direction == direction::left) {
      return strafe(camera, -speed);
    }
    else if constexpr (move::direction == direction::right) {
      return strafe(camera, speed);
    }
  }
};

template <direction Axis>
struct rotate {
  constexpr explicit inline rotate(float speed = 0.02) : speed{speed} {}
  constexpr static inline direction axis{Axis};
  float speed;

  template <class Camera>
  constexpr auto operator()(Camera& camera) const noexcept {
    constexpr auto rotate_horizontally = [](Camera& camera, float speed) {
      return ignore([&camera, speed] { camera.rotate(speed, 0); });
    };
    constexpr auto rotate_vertically = [](Camera& camera, float speed) {
      return ignore([&camera, speed] { camera.rotate(0, speed); });
    };

    if constexpr (axis == direction::left) {
      return rotate_horizontally(camera, -speed);
    }
    else if constexpr (axis == direction::right) {
      return rotate_horizontally(camera, speed);
    }
    else if constexpr (axis == direction::up) {
      return rotate_vertically(camera, speed);
    }
    else if constexpr (axis == direction::down) {
      return rotate_vertically(camera, -speed);
    }
    else {
      static_assert(std::is_same_v<Camera, void>, "Unhandled direction");
    }
  }
};

constexpr static inline auto reset_camera = [](auto& cam) {
  return ignore([&] {
    using namespace dpsg;
    constexpr radians default_yaw{to_radians(degrees{-90})};
    constexpr radians default_pitch{0};
    constexpr radians default_fov{to_radians(degrees{45})};
    cam.reset(default_yaw, default_pitch, default_fov);
  });
};

constexpr static inline auto track_cursor = [](auto& camera) {
  return camera_tracks_cursor{camera};
};

constexpr static inline auto zoom_camera = [](auto& camera) {
  return ignore(
      [&]([[maybe_unused]] double x, double y) { camera.zoom(y * 0.01); });
};

constexpr static inline auto close_window = []([[maybe_unused]] auto&) {
  return close;
};

constexpr static inline auto toggle_cursor_tracking = [](auto& camera) {
  return [b = true, &camera](auto& window) mutable {
    if (b) {
      window.set_cursor_pos_callback(nullptr);
    }
    else {
      window.set_cursor_pos_callback(camera_tracks_cursor{camera});
    }
    b = !b;
  };
};

}  // namespace actions

namespace inputs {
struct key {
  constexpr key(dpsg::input::key k) noexcept : value{k} {}
  dpsg::input::key value;

  template <class Tag, class Op, class W>
  constexpr void operator()([[maybe_unused]] Tag tag,
                            Op&& operation,
                            W&& wdw) const noexcept {
    if constexpr (std::is_same_v<Tag, repeated>) {
      wdw.while_(value, std::forward<Op>(operation));
    }
    else if constexpr (std::is_same_v<Tag, one_time>) {
      wdw.on(value, std::forward<Op>(operation));
    }
  }
};

constexpr static inline auto cursor_movement =
    []([[maybe_unused]] auto marker, auto&& op, auto& window) {
      window.set_cursor_pos_callback(std::forward<decltype(op)>(op));
    };

constexpr static inline auto mouse_scroll =
    []([[maybe_unused]] auto marker, auto&& op, auto& window) {
      window.set_scroll_callback(std::forward<decltype(op)>(op));
    };
}  // namespace inputs

namespace detail {
constexpr static inline auto bind_inputs =
    [](auto&& input, auto marker, auto&& action, auto& camera, auto& window) {
      using action_t = decltype(action);
      using input_t = decltype(input);
      std::forward<input_t>(input)(
          marker, std::forward<action_t>(action)(camera), window);
    };
}  // namespace detail

template <class... Ss, class C, class W>
void bind_control_scheme(const control::control_scheme<Ss...>& scheme,
                         C& camera,
                         W& window) noexcept {
  dpsg::traverse(scheme, detail::bind_inputs, camera, window);
}

namespace detail {
template <class T>
using adapt =
    std::conditional_t<std::is_same_v<std::decay_t<T>, dpsg::input::key>,
                       inputs::key,
                       std::decay_t<T>>;
}

template <class Action, class... Args>
struct repeat : control::input<repeated, Action, Args...> {
  using base = control::input<repeated, Action, Args...>;
  template <class A, class... As>
  constexpr explicit repeat(A&& a, As&&... as)
      : base{std::forward<A>(a), std::forward<As>(as)...} {}
};
template <class A, class... As>
repeat(A&&, As&&...) -> repeat<std::decay_t<A>, detail::adapt<As>...>;

template <class Action, class... Args>
struct input : control::input<one_time, Action, Args...> {
  using base = control::input<one_time, Action, Args...>;
  template <class A, class... As>
  constexpr explicit input(A&& a, As&&... as)
      : base{std::forward<A>(a), std::forward<As>(as)...} {}
};
template <class A, class... As>
input(A&&, As&&...) -> input<std::decay_t<A>, detail::adapt<As>...>;

constexpr static inline auto free_camera_movement = [] {
  using namespace control;
  using key = dpsg::input::key;
  using namespace actions;
  using d = direction;
  using namespace inputs;
  return control_scheme{repeat{move<d::forward>{}, key::W, key::up},
                        repeat{move<d::backward>{}, key::S, key::down},
                        repeat{move<d::left>{}, key::A, key::left},
                        repeat{move<d::right>{}, key::D, key::right},
                        repeat{move<d::up>{}, key::R},
                        repeat{move<d::down>{}, key::F}};
}();

constexpr static inline auto reset_camera =
    input{actions::reset_camera, dpsg::input::key::G};

constexpr static inline auto free_camera_rotation = control::control_scheme{
    input{actions::track_cursor, inputs::cursor_movement},
    input{actions::toggle_cursor_tracking, dpsg::input::key::space}};

constexpr static inline auto kb_camera_rotation = [] {
  using namespace control;
  using namespace actions;
  using d = direction;
  using key = dpsg::input::key;
  return control_scheme{repeat{rotate<d::left>{}, key::Q},
                        repeat{rotate<d::right>{}, key::E}};
}();

constexpr static inline auto zoom =
    input{actions::zoom_camera, inputs::mouse_scroll};

constexpr static inline auto free_camera =
    control::combine(free_camera_movement,
                     free_camera_rotation,
                     zoom,
                     reset_camera,
                     kb_camera_rotation);

constexpr static inline auto close_window =
    input{actions::close_window, dpsg::input::key::escape};

constexpr static inline auto standard_controls =
    control::combine(free_camera, close_window);
}  // namespace glfw_controls

#endif  // GUARD_GLFW_CONTROLS_HEADER