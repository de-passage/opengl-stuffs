#include "opengl.hpp"

#include "glm/gtc/type_ptr.hpp"
#include "glm/mat2x2.hpp"
#include "glm/mat2x3.hpp"
#include "glm/mat2x4.hpp"
#include "glm/mat3x2.hpp"
#include "glm/mat3x3.hpp"
#include "glm/mat3x4.hpp"
#include "glm/mat4x2.hpp"
#include "glm/mat4x3.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"

namespace dpsg::gl {

inline void uniform(uniform_location loc, const glm::mat2& mat) noexcept {
  glUniformMatrix2fv(loc.value, 1, GL_FALSE, glm::value_ptr(mat));
}

inline void uniform(uniform_location loc, const glm::mat2x3& mat) noexcept {
  glUniformMatrix2x3fv(loc.value, 1, GL_FALSE, glm::value_ptr(mat));
}

inline void uniform(uniform_location loc, const glm::mat2x4& mat) noexcept {
  glUniformMatrix2x4fv(loc.value, 1, GL_FALSE, glm::value_ptr(mat));
}

inline void uniform(uniform_location loc, const glm::mat3& mat) noexcept {
  glUniformMatrix3fv(loc.value, 1, GL_FALSE, glm::value_ptr(mat));
}

inline void uniform(uniform_location loc, const glm::mat3x2& mat) noexcept {
  glUniformMatrix3x2fv(loc.value, 1, GL_FALSE, glm::value_ptr(mat));
}

inline void uniform(uniform_location loc, const glm::mat3x4& mat) noexcept {
  glUniformMatrix3x4fv(loc.value, 1, GL_FALSE, glm::value_ptr(mat));
}

inline void uniform(uniform_location loc, const glm::mat4x2& mat) noexcept {
  glUniformMatrix4x2fv(loc.value, 1, GL_FALSE, glm::value_ptr(mat));
}

inline void uniform(uniform_location loc, const glm::mat4x3& mat) noexcept {
  glUniformMatrix4x3fv(loc.value, 1, GL_FALSE, glm::value_ptr(mat));
}

inline void uniform(uniform_location loc, const glm::mat4& mat) noexcept {
  glUniformMatrix4fv(loc.value, 1, GL_FALSE, glm::value_ptr(mat));
}

inline void uniform(uniform_location loc, const glm::vec1& vec) noexcept {
  glUniform1fv(loc.value, 1, glm::value_ptr(vec));
}

inline void uniform(uniform_location loc, const glm::vec2& vec) noexcept {
  glUniform2fv(loc.value, 1, glm::value_ptr(vec));
}

inline void uniform(uniform_location loc, const glm::vec3& vec) noexcept {
  glUniform3fv(loc.value, 1, glm::value_ptr(vec));
}

inline void uniform(uniform_location loc, const glm::vec4& vec) noexcept {
  glUniform4fv(loc.value, 1, glm::value_ptr(vec));
}
}  // namespace dpsg::gl