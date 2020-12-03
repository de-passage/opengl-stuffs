#ifndef GUARD_DPSG_TEXTURE_HEADER
#define GUARD_DPSG_TEXTURE_HEADER

#include "opengl.hpp"

#include <cassert>
#include <optional>

namespace dpsg {

namespace texture_traits {
template <gl::texture_target Target> struct base_traits {
  constexpr inline static gl::texture_target texture_target = Target;

  template <class... Args> static void set_parameter(Args &&... args) noexcept {
    gl::tex_parameter(texture_target, std::forward<Args>(args)...);
  }

  static void bind(gl::texture_id id) noexcept {
    gl::bind_texture(texture_target, id);
  }

  static void generate_mipmap() noexcept {
    gl::generate_mipmap(texture_target);
  }
};

struct _2d : base_traits<gl::texture_target::_2d> {
  template <class... Args>
  static void generate_image(Args &&... args) noexcept {
    gl::tex_image_2D(gl::texture_image_target::_2d,
                     std::forward<Args>(args)...);
  }
};

} // namespace texture_traits
template <class Traits> class basic_texture : private Traits {
  using Traits::generate_image;
  using Traits::generate_mipmap;
  using Traits::set_parameter;

public:
  template <class Image> explicit basic_texture(Image &&i) noexcept {
    gl::gen_texture(_id);
    assert(_id.value > 0);

    bind();

    set_parameter(gl::wrap_target::s, gl::wrap_mode::repeat);

    set_parameter(gl::wrap_target::t, gl::wrap_mode::repeat);

    set_parameter(gl::min_filter::linear);
    set_parameter(gl::mag_filter::linear);

    generate_image(i.width(), i.height(), i.image_format(), i.texture());

    generate_mipmap();
  }

  basic_texture() = default;
  basic_texture(const basic_texture &) = delete;
  basic_texture(basic_texture &&txt) noexcept
      : _id(std::exchange(txt._id, gl::texture_id{0})) {}
  basic_texture &operator=(const basic_texture &) = delete;
  basic_texture &operator=(basic_texture &&texture) noexcept {
    _id = std::exchange(texture._id, gl::texture_id{0});
    return *this;
  }
  ~basic_texture() noexcept { gl::delete_texture(_id); }

  void bind() const noexcept { Traits::bind(_id); }
  [[nodiscard]] gl::texture_id id() const noexcept { return _id; }

private:
  gl::texture_id _id{};
};

using texture_2d = basic_texture<texture_traits::_2d>;

} // namespace dpsg

#endif // GUARD_DPSG_TEXTURE_HEADER