#ifndef GUARD_DPSG_STBI_WRAPPER_HEADER
#define GUARD_DPSG_STBI_WRAPPER_HEADER

#include "common.hpp"
#include "load_shaders.hpp"
#include "opengl.hpp"
#include "stb_image.h"
#include "texture.hpp"

namespace dpsg {

template <gl::image_format Format> struct stbi_trait {
  [[nodiscard]] static inline gl::image_format image_format() noexcept {
    return Format;
  }
};

using texture_rgba = stbi_trait<gl::image_format::rgba>;
using texture_rgb = stbi_trait<gl::image_format::rgb>;

template <class Traits> class stbi_wrapper : public Traits {
public:
  inline stbi_wrapper(stbi_uc *texture, unsigned int width, unsigned int height,
                      int channels) noexcept
      : _texture(texture), _width{width}, _height{height}, _channels(channels) {
  }
  stbi_wrapper(const stbi_wrapper &w) = delete;
  inline stbi_wrapper(stbi_wrapper &&w) noexcept
      : _texture(std::exchange(w._texture, nullptr)),
        _width(std::exchange(w._width, gl::width{0})),
        _height(std::exchange(w._height, gl::height{0})),
        _channels(std::exchange(w._channels, 0)) {}

  stbi_wrapper &operator=(const stbi_wrapper &w) = delete;
  inline stbi_wrapper &operator=(stbi_wrapper &&w) noexcept {
    using std::swap;
    _width = std::exchange(w._width, gl::width{0});
    _height = std::exchange(w._height, gl::height{0});
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
  [[nodiscard]] inline gl::width width() const noexcept { return _width; }
  [[nodiscard]] inline gl::height height() const noexcept { return _height; }
  [[nodiscard]] inline unsigned int channels() const noexcept {
    return _channels;
  }

private:
  stbi_uc *_texture;
  gl::width _width;
  gl::height _height;
  int _channels;

  inline void _clean() noexcept {
    stbi_image_free(static_cast<void *>(_texture));
  }
};

DPSG_LAZY_STR_WRAPPER_IMPL(texture_filename) // NOLINT

template <class TextureTraits, class TextureOptions, class T>
std::optional<texture_2d> load(const texture_filename<T> &filename,
                               TextureOptions &&options,
                               int requested_channels = 0) {
  int h{};
  int w{};
  int c{};
  stbi_set_flip_vertically_on_load(true);
  stbi_uc *ptr = stbi_load(filename.c_str(), &w, &h, &c, requested_channels);
  if (ptr == nullptr) {
    return {};
  }
  return texture_2d{
      stbi_wrapper<TextureTraits>{ptr, static_cast<unsigned int>(w),
                                  static_cast<unsigned int>(h), c},
      std::forward<TextureOptions>(options)};
}

template <class TextureTraits, class T>
std::optional<texture_2d> load(const texture_filename<T> &filename,
                               int requested_channels = 0) {
  return load<TextureTraits>(filename, texture_options::no_options,
                             requested_channels);
}

} // namespace dpsg

#endif // GUARD_DPSG_STBI_WRAPPER_HEADER