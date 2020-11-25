#ifndef GUARD_DPSG_TEXTURE_HEADER
#define GUARD_DPSG_TEXTURE_HEADER

#include "common.hpp"
#include "load_shaders.hpp"
#include "stb_image.h"

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
  [[nodiscard]] inline int width() const noexcept { return _width; }
  [[nodiscard]] inline int height() const noexcept { return _height; }
  [[nodiscard]] inline int channels() const noexcept { return _channels; }
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

template <class I> class texture : I {

public:
  template <class J> explicit texture(J &&i) noexcept : I{std::forward<I>(i)} {
    glGenTextures(1, &_id);

    glBindTexture(GL_TEXTURE_2D, _id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, I::width(), I::height(), 0, GL_RGB,
                 I::pointer_type(), I::texture());

    glGenerateMipmap(GL_TEXTURE_2D);
  }
  texture() = default;
  texture(const texture &) = delete;
  texture(texture &&txt) noexcept
      : I(static_cast<I &&>(txt)), _id(std::exchange(txt._id, 0)) {}
  texture &operator=(const texture &) = delete;
  texture &operator=(texture &&texture) noexcept {
    static_cast<I &>(*this) = static_cast<I &&>(texture);
    _id = std::exchange(txt._id, 0);
    return *this;
  }
  ~texture() noexcept { glDeleteTextures(1, &_id); }

  void bind() const noexcept { glBindTexture(GL_TEXTURE_2D, _id); }

private:
  unsigned int _id{};
};

DPSG_LAZY_STR_WRAPPER_IMPL(texture_filename) // NOLINT

template <class T>
std::optional<texture<stbi_wrapper>> load(const texture_filename<T> &texture,
                                          int requested_channels = 0) {
  int h{};
  int w{};
  int c{};
  stbi_uc *ptr = stbi_load(texture.c_str(), &h, &w, &c, requested_channels);
  if (ptr == nullptr) {
    return {};
  }
  return ::dpsg::texture<stbi_wrapper>{stbi_wrapper{ptr, h, w, c}};
}

} // namespace dpsg

#endif // GUARD_DPSG_TEXTURE_HEADER