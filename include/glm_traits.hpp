#ifndef GUARD_DPSG_GLM_TRAITS_HEADER
#define GUARD_DPSG_GLM_TRAITS_HEADER

#include "common.hpp"

#include "glm/geometric.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

namespace dpsg {

struct glm_traits {
  using value_type = float;
  using vec_type = glm::vec3;
  using mat_type = glm::mat4;

  inline static mat_type perspective(radians f, aspect_ratio ar, float z_near,
                                     float z_far) {
    return glm::perspective(f.value, ar.value, z_near, z_far);
  }

  inline static vec_type normalize(const vec_type &input) {
    return glm::normalize(input);
  }

  inline static vec_type cross(const vec_type &lhs, const vec_type &rhs) {
    return glm::cross(lhs, rhs);
  }
};
} // namespace dpsg

#endif // GUARD_DPSG_GLM_TRAITS_HEADER