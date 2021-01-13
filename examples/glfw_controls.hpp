#ifndef GUARD_GLFW_CONTROLS_HEADER
#define GUARD_GLFW_CONTROLS_HEADER

#include "camera.hpp"
#include "common.hpp"
#include "control_scheme.hpp"
#include "glm/trigonometric.hpp"
#include "input/keys.hpp"
#include "input_timer.hpp"
#include "key_mapper.hpp"
#include "traversecpp/traverse.hpp"
#include "window.hpp"

#include "./utils.hpp"

#include <chrono>
#include <utility>

namespace glfw_controls {

enum class direction { forward, backward, left, right, up, down };
enum class rotation_axis : unsigned char { x = 1, y = 2, both = x | y };

template <direction Direction>
struct move : control::action {
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

template <rotation_axis Axis>
struct rotate : control::action {
  constexpr explicit inline rotate(
      dpsg::degrees speed = dpsg::degrees{glm::degrees(0.1)})
      : speed{speed} {}
  constexpr static inline rotation_axis axis{Axis};
  dpsg::degrees speed;

  template <class Camera>
  constexpr auto operator()(Camera& camera) noexcept {
    return ignore([&camera] { camera.strafe(0); });
  }
};

struct key {
  constexpr explicit key(dpsg::input::key k) noexcept : value{k} {}
  dpsg::input::key value;

  template <class Tag, class Op, class W>
  constexpr void operator()([[maybe_unused]] Tag tag,
                            Op&& operation,
                            W&& wdw) const noexcept {
    if constexpr (std::is_same_v<Tag, control::repeated>) {
      wdw.while_(value, std::forward<Op>(operation));
    }
    else if constexpr (std::is_same_v<Tag, control::one_time>) {
      wdw.on(value, std::forward<Op>(operation));
    }
  }
};

constexpr static inline auto free_camera = [] {
  using namespace control;
  using k = dpsg::input::key;
  using d = direction;
  return control_scheme{repeat{move<d::forward>{}, key{k::W}, key{k::up}},
                        repeat{move<d::backward>{}, key{k::S}, key{k::down}},
                        repeat{move<d::left>{}, key{k::A}, key{k::left}},
                        repeat{move<d::right>{}, key{k::D}, key{k::right}},
                        repeat{move<d::up>{}, key{k::R}},
                        repeat{move<d::down>{}, key{k::F}}};
}();

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
                         dpsg::camera<C>& camera,
                         W& window) noexcept {
  dpsg::traverse(scheme, detail::bind_inputs, camera, window);
}

namespace mixin {
struct key_mapper {
  template <class B>
  struct type : B {
    using base = B;

   private:
    using kmap_t = basic_key_mapper<dpsg::real_type_t<B>>;
    constexpr static auto interval = std::chrono::milliseconds(10);
    kmap_t kmap;
    input_timer<std::function<void()>> timer;

   public:
    using key_pressed_callback = typename kmap_t::key_pressed_callback;
    template <class... Args>
    constexpr explicit type(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<base, Args...>)
        : base{std::forward<Args>(args)...},
          timer{[this] {
                  kmap.trigger_pressed_callbacks(
                      *static_cast<dpsg::real_type_t<B>*>(this));
                },
                interval} {
      base::set_key_callback(std::ref(kmap));
    }

    inline void on(dpsg::input::key k, key_pressed_callback cb) {
      kmap.on(k, std::move(cb));
    }

    inline void while_(dpsg::input::key k, key_pressed_callback cb) {
      kmap.while_(k, std::move(cb));
    }

    template <class F>
    inline auto render_loop(F&& f) noexcept(
        noexcept(base::render_loop(std::forward<F>(f)))) {
      base::render_loop([this, &f](auto&&... args) {
        std::forward<F>(f)(std::forward<decltype(args)>(args)...);
        timer.trigger();
        kmap.trigger_pressed_callbacks(
            *static_cast<dpsg::real_type_t<B>*>(this));
      });
    }
  };
};

}  // namespace mixin

}  // namespace glfw_controls

#endif  // GUARD_GLFW_CONTROLS_HEADER