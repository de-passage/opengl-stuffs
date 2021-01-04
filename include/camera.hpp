#ifndef GUARD_DPSG_CAMERA_HEADER
#define GUARD_DPSG_CAMERA_HEADER

#include "common.hpp"
#include "opengl.hpp"

#include <algorithm>

namespace dpsg {

template <class Traits>
class camera : Traits {
 public:
  using value_type = typename Traits::value_type;
  using vec_type = typename Traits::vec_type;
  using mat_type = typename Traits::mat_type;
  using z_far = basic_z_far<value_type>;
  using z_near = basic_z_near<value_type>;
  using radians = basic_radians<value_type>;
  using degrees = basic_degrees<value_type>;

  constexpr static inline radians max_pitch{89};
  constexpr static inline radians min_pitch{-max_pitch.value};
  constexpr static inline radians default_yaw{to_radians(degrees{-90})};
  constexpr static inline radians default_pitch{0};
  constexpr static inline radians default_fov{to_radians(degrees{45})};
  constexpr static inline z_far default_z_far{100};
  constexpr static inline z_near default_z_near{.1};
  constexpr static inline radians max_fov{to_radians(degrees{45})};
  constexpr static inline radians min_fov{to_radians(degrees{1})};
  constexpr static inline vec_type default_position{0, 0, 3};

 private:
  using Traits::cross;
  using Traits::look_at;
  using Traits::normalize;
  using Traits::perspective;

  aspect_ratio _aspect_ratio;
  radians _fov;
  z_near _z_near;
  z_far _z_far;
  radians _pitch;
  radians _yaw;

  vec_type _position{default_position};
  vec_type _front{0, 0, -1};
  vec_type _up{0, 1, 0};
  vec_type _world_up{0, 1, 0};
  vec_type _right{0, 1, 0};

  [[nodiscard]] constexpr mat_type _compute_projection() const noexcept {
    return perspective(_fov, _aspect_ratio, _z_near, _z_far);
  }

  [[nodiscard]] constexpr mat_type _compute_view() const noexcept {
    return look_at(_position, _position + _front, _up);
  }

  [[nodiscard]] constexpr vec_type _compute_front() const {
    return normalize(vec_type{cos(_yaw.value) * cos(_pitch.value),
                              _pitch.value,
                              sin(_yaw.value) * cos(_pitch.value)});
  }

  [[nodiscard]] constexpr vec_type _compute_right() const noexcept {
    return normalize(cross(_front, _world_up));
  }

  [[nodiscard]] constexpr vec_type _compute_up() const noexcept {
    return normalize(cross(_right, _front));
  }

  void _update_vecs() noexcept {
    _front = _compute_front();
    _right = _compute_right();
    _up = _compute_up();
  }

 public:
  constexpr inline explicit camera(aspect_ratio ar) noexcept
      : _aspect_ratio{ar},
        _fov{default_fov},
        _z_near{default_z_near},
        _z_far{default_z_far},
        _pitch{default_pitch},
        _yaw{default_yaw} {}

  constexpr value_type aspect_ratio() const noexcept { return _aspect_ratio; }

  constexpr void aspect_ratio(width w, height h) noexcept {
    aspect_ratio(w / h);
  }

  constexpr void aspect_ratio(struct aspect_ratio ar) noexcept {
    _aspect_ratio = ar;
  }

  inline mat_type projection() const noexcept { return _compute_projection(); }
  inline mat_type view() const noexcept { return _compute_view(); }

  inline mat_type projected_view() const noexcept {
    return projection() * view();
  }

  inline void advance(value_type offset) noexcept {
    _position += offset * _front;
  }

  inline void strafe(value_type offset) noexcept {
    _position += offset * normalize(cross(_front, _up));
  }

  inline void climb(value_type offset) noexcept { _position += offset * _up; }

  inline void reset(radians yaw, radians pitch, radians fov) noexcept {
    _yaw = yaw;
    _pitch = pitch;
    _fov = fov;
    _position = default_position;
    _update_vecs();
  }

  inline void rotate(value_type x_offset, value_type y_offset) noexcept {
    _yaw.value += x_offset;
    _pitch.value =
        std::clamp(_pitch.value + y_offset, min_pitch.value, max_pitch.value);
    _update_vecs();
  }

  inline void zoom(value_type offset) noexcept {
    _fov.value = std::clamp(_fov.value + offset, min_fov.value, max_fov.value);
  }

  inline const vec_type& position() const noexcept { return _position; }
  inline vec_type& position() noexcept { return _position; }
};
}  // namespace dpsg

#endif  // GUARD_DPSG_CAMERA_HEADER