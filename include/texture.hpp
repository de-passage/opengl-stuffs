#ifndef GUARD_DPSG_TEXTURE_HEADER
#define GUARD_DPSG_TEXTURE_HEADER

#include "opengl.hpp"

#include "common.hpp"
#include "load_shaders.hpp"
#include "stb_image.h"

#include <cassert>
#include <optional>

namespace dpsg {

class stbi_wrapper {
public:
  inline stbi_wrapper(stbi_uc *texture, int width, int height,
                      int channels) noexcept
      : _texture(texture), _width(width), _height(height), _channels(channels) {
  }
  stbi_wrapper(const stbi_wrapper &w) = delete;
  inline stbi_wrapper(stbi_wrapper &&w) noexcept
      : _texture(std::exchange(w._texture, nullptr)),
        _width(std::exchange(w._width, 0)),
        _height(std::exchange(w._height, 0)),
        _channels(std::exchange(w._channels, 0)) {}

  stbi_wrapper &operator=(const stbi_wrapper &w) = delete;
  inline stbi_wrapper &operator=(stbi_wrapper &&w) noexcept {
    using std::swap;
    _width = std::exchange(w._width, 0);
    _height = std::exchange(w._height, 0);
    _channels = std::exchange(w._channels, 0);
    swap(w._texture, _texture);
    w._clean();
    return *this;
  }

  inline ~stbi_wrapper() noexcept { _clean(); }

  [[nodiscard]] inline const stbi_uc *texture() const noexcept {
    return _texture;
  }
  [[nodiscard]] inline stbi_uc *texture() noexcept { return _texture; }
  [[nodiscard]] inline unsigned int width() const noexcept { return _width; }
  [[nodiscard]] inline unsigned int height() const noexcept { return _height; }
  [[nodiscard]] inline unsigned int channels() const noexcept {
    return _channels;
  }
  [[nodiscard]] static inline int pointer_type() noexcept {
    return GL_UNSIGNED_BYTE;
  }

private:
  stbi_uc *_texture;
  int _width;
  int _height;
  int _channels;

  inline void _clean() noexcept {
    stbi_image_free(static_cast<void *>(_texture));
  }
};

class texture {

public:
  template <class J> explicit texture(J &&i) noexcept {
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

DPSG_LAZY_STR_WRAPPER_IMPL(texture_filename) // NOLINT

template <class T>
std::optional<texture> load(const texture_filename<T> &filename,
                            int requested_channels = 0) {
  int h{};
  int w{};
  int c{};
  stbi_uc *ptr = stbi_load(filename.c_str(), &h, &w, &c, requested_channels);
  if (ptr == nullptr) {
    return {};
  }
  return texture{stbi_wrapper{ptr, h, w, c}};
}

} // namespace dpsg

#endif // GUARD_DPSG_TEXTURE_HEADER