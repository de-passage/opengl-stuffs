#ifndef GUARD_DPSG_TEXTURE_HEADER
#define GUARD_DPSG_TEXTURE_HEADER

#include "opengl.hpp"

#include <cassert>
#include <optional>

namespace dpsg {
class texture {

public:
  template <class Descriptor> explicit texture(Descriptor &&i) noexcept {
    gl::gen_texture(_id);
    assert(_id.value > 0);

    gl::bind_texture(gl::texture_target::t2d, _id);

    gl::tex_parameter(gl::texture_target::t2d, gl::wrap_target::s,
                      gl::wrap_mode::repeat);

    gl::tex_parameter(gl::texture_target::t2d, gl::wrap_target::t,
                      gl::wrap_mode::repeat);

    gl::tex_parameter(gl::texture_target::t2d, gl::min_filter::linear);
    gl::tex_parameter(gl::texture_target::t2d, gl::mag_filter::linear);

    gl::tex_image_2D(gl::texture_image_target::t2d, gl::internal_format{GL_RGB},
                     gl::width{i.width()}, gl::height{i.height()},
                     gl::image_format::rgb, i.texture());

    gl::generate_mipmap(gl::texture_target::t2d);
  }
  texture() = default;
  texture(const texture &) = delete;
  texture(texture &&txt) noexcept
      : _id(std::exchange(txt._id, gl::texture_id{0})) {}
  texture &operator=(const texture &) = delete;
  texture &operator=(texture &&texture) noexcept {
    _id = std::exchange(texture._id, gl::texture_id{0});
    return *this;
  }
  ~texture() noexcept { gl::delete_texture(_id); }

  void bind() const noexcept { gl::bind_texture(gl::texture_target::t2d, _id); }
  [[nodiscard]] gl::texture_id id() const noexcept { return _id; }

private:
  gl::texture_id _id{};
};

} // namespace dpsg

#endif // GUARD_DPSG_TEXTURE_HEADER