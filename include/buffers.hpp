#ifndef GUARD_DPSG_BUFFERS_HEADER
#define GUARD_DPSG_BUFFERS_HEADER

#include "glad/glad.h"

#include <utility>

namespace dpsg {
template <std::size_t N> class buffer_array_impl {
public:
  buffer_array_impl() noexcept {
    glGenBuffers(N, static_cast<unsigned int *>(values));
  }
  buffer_array_impl(const buffer_array_impl &) = delete;
  buffer_array_impl(buffer_array_impl &&a) noexcept {
    for (std::size_t i; i < N; ++i) {
      values[i] = std::exchange(a.values[i], 0);
    }
  }

  buffer_array_impl &operator=(buffer_array_impl &&a) noexcept {

    for (std::size_t i; i < N; ++i) {
      values[i] = std::exchange(a.values[i], 0);
    }
    return *this;
  }
  buffer_array_impl &operator=(const buffer_array_impl &) = delete;
  ~buffer_array_impl() noexcept {
    glDeleteBuffers(N, static_cast<unsigned int *>(values));
  }

  unsigned int operator[](std::size_t s) const noexcept { return values[s]; }

protected:
  unsigned int values[N]; // NOLINT
};

template <std::size_t N> class buffer_array : public buffer_array_impl<N> {};

enum class buffer_type {
  array = GL_ARRAY_BUFFER,
  copy_read = GL_COPY_READ_BUFFER,
  copy_write = GL_COPY_WRITE_BUFFER,
  element_array = GL_ELEMENT_ARRAY_BUFFER,
  pixel_pack = GL_PIXEL_PACK_BUFFER,
  pixel_unpack = GL_PIXEL_UNPACK_BUFFER,
  texture = GL_TEXTURE_BUFFER,
  transform_feedback = GL_TRANSFORM_FEEDBACK_BUFFER,
  uniform = GL_UNIFORM_BUFFER
};

class buffer : buffer_array_impl<1> {
public:
  explicit operator unsigned int() const noexcept { return values[0]; }
  [[nodiscard]] unsigned int id() const noexcept { return values[0]; }
  void bind(buffer_type bt) const { glBindBuffer(static_cast<int>(bt), id()); }
  static void unbind(buffer_type bt) { glBindBuffer(static_cast<int>(bt), 0); }
};

class vertex_buffer : buffer {
public:
  void bind() const { buffer::bind(buffer_type::array); }

  using buffer::id;
  using buffer::unbind;
  using buffer::operator unsigned int;
};

class element_buffer : buffer {
public:
  void bind() const { buffer::bind(buffer_type::element_array); }

  using buffer::id;
  using buffer::unbind;
  using buffer::operator unsigned int;
};

template <std::size_t N> class vertex_array_impl {
public:
  vertex_array_impl() noexcept {
    glGenVertexArrays(N, static_cast<unsigned int *>(values));
  }
  vertex_array_impl(const vertex_array_impl &) = delete;
  vertex_array_impl(vertex_array_impl &&a) noexcept {
    for (std::size_t i; i < N; ++i) {
      values[i] = std::exchange(a.values[i], 0);
    }
  }

  vertex_array_impl &operator=(vertex_array_impl &&a) noexcept {

    for (std::size_t i; i < N; ++i) {
      values[i] = std::exchange(a.values[i], 0);
    }

    return *this;
  }

  vertex_array_impl &operator=(const vertex_array_impl &) = delete;
  ~vertex_array_impl() noexcept {
    glDeleteVertexArrays(N, static_cast<unsigned int *>(values));
  }

  unsigned int operator[](std::size_t s) const noexcept { return values[s]; }

protected:
  unsigned int values[N]; // NOLINT
};

template <std::size_t N> class vertex_array_n : public vertex_array_impl<N> {};

class vertex_array : vertex_array_impl<1> {
public:
  explicit operator unsigned int() const noexcept { return values[0]; }
  [[nodiscard]] unsigned int id() const noexcept { return values[0]; }
  void bind() const { glBindVertexArray(id()); }
  static void unbind() { glBindVertexArray(0); }
};

} // namespace dpsg

#endif // GUARD_DPSG_BUFFERS_HEADER