#ifndef GUARD_DPSG_CAMERA_HEADER
#define GUARD_DPSG_CAMERA_HEADER

#include "common.hpp"

namespace dpsg {

template <class Traits> class camera : Traits {
public:
  using value_type = typename Traits::value_type;
  using vec_type = typename Traits::vec_type;
  using mat_type = typename Traits::mat_type;

private:
  using Traits::cross;
  using Traits::normalize;
  using Traits::perspective;
  using Traits::rotate;
  using Traits::translate;

  aspect_ratio _aspect_ratio;
  radians _fov;
  value_type _z_near;
  value_type _z_far;

  mat_type _projection;

  [[nodiscard]] constexpr mat_type _compute_projection() const {
    return perspective(_fov, _aspect_ratio, _z_near, _z_far);
  }

public:
  constexpr inline explicit camera(aspect_ratio ar)
      : _aspect_ratio{ar}, _fov{45}, _z_near{0.1}, _z_far{100},
        _projection{compute_projection()} {}

  constexpr value_type aspect_ratio() const { return _aspect_ratio; }

  constexpr void aspect_ratio(width w, height h) { _aspect_ratio = w / h; }

  constexpr void aspect_ratio(struct aspect_ratio ar) { _aspect_ratio = ar; }

  const mat_type &projection() const { return _projection; }
};
} // namespace dpsg

#endif // GUARD_DPSG_CAMERA_HEADER