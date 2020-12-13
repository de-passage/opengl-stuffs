#ifndef GUARD_DPSG_BUFFERS_HEADER
#define GUARD_DPSG_BUFFERS_HEADER

#include "glad/glad.h"

#include "opengl.hpp"

#include <utility>

namespace dpsg {
template <std::size_t N> class buffer_array_impl {
public:
  buffer_array_impl() noexcept { gl::gen_buffers(values); }
  buffer_array_impl(const buffer_array_impl &) = delete;
  buffer_array_impl(buffer_array_impl &&a) noexcept {
    for (std::size_t i = 0; i < N; ++i) {
      values[i] = std::exchange(a.values[i], gl::generic_buffer_id{0});
    }
  }

  buffer_array_impl &operator=(buffer_array_impl &&a) noexcept {

    for (std::size_t i = 0; i < N; ++i) {
      values[i] = std::exchange(a.values[i], gl::generic_buffer_id{0});
    }
    return *this;
  }
  buffer_array_impl &operator=(const buffer_array_impl &) = delete;
  ~buffer_array_impl() noexcept { gl::delete_buffers(values); }

  gl::generic_buffer_id operator[](std::size_t s) const noexcept {
    return values[s];
  }

protected:
  gl::generic_buffer_id values[N]; // NOLINT
};

template <std::size_t N> class buffer_array : public buffer_array_impl<N> {};

class buffer : buffer_array_impl<1> {
public:
  explicit operator unsigned int() const noexcept { return values[0].value; }
  [[nodiscard]] gl::generic_buffer_id id() const noexcept { return values[0]; }
  void bind(gl::buffer_type bt) const {
    gl::bind_buffer(bt, gl::generic_buffer_id{id()});
  }
  static void unbind(gl::buffer_type bt) {
    gl::bind_buffer(bt, gl::generic_buffer_id{0});
  }
  template <typename Type, std::size_t S>
  // NOLINTNEXTLINE
  static void set_data(gl::buffer_type type, Type (&data)[S],
                       gl::data_hint hint) {
    gl::buffer_data(type, data, hint);
  }
};

class vertex_buffer : buffer {
public:
  void bind() const { buffer::bind(gl::buffer_type::array); }
  template <typename T, std::size_t S>
  // NOLINTNEXTLINE
  void set_data(T (&data)[S],
                gl::data_hint h = gl::data_hint::static_draw) const {
    buffer::set_data(gl::buffer_type::array, data, h);
  }

  using buffer::id;
  using buffer::unbind;
  using buffer::operator unsigned int;
};

class element_buffer : buffer {
public:
  void bind() const { buffer::bind(gl::buffer_type::element_array); }
  template <typename T, std::size_t S>
  // NOLINTNEXTLINE
  void set_data(T (&data)[S],
                gl::data_hint h = gl::data_hint::static_draw) const {
    buffer::set_data(gl::buffer_type::element_array, data, h);
  }

  using buffer::id;
  using buffer::unbind;
  using buffer::operator unsigned int;
};

template <std::size_t N> class vertex_array_impl {
public:
  vertex_array_impl() noexcept { gl::gen_vertex_arrays(values); }
  vertex_array_impl(const vertex_array_impl &) = delete;
  vertex_array_impl(vertex_array_impl &&a) noexcept {
    for (std::size_t i = 0; i < N; ++i) {
      values[i] = std::exchange(a.values[i], gl::vertex_array_id{0});
    }
  }

  vertex_array_impl &operator=(vertex_array_impl &&a) noexcept {

    for (std::size_t i = 0; i < N; ++i) {
      values[i] = std::exchange(a.values[i], gL::vertex_array_id{0});
    }

    return *this;
  }

  vertex_array_impl &operator=(const vertex_array_impl &) = delete;
  ~vertex_array_impl() noexcept { gl::delete_vertex_arrays(values); }

  gl::vertex_array_id operator[](std::size_t s) const noexcept {
    return values[s];
  }

protected:
  gl::vertex_array_id values[N]; // NOLINT
};

template <std::size_t N> class vertex_array_n : public vertex_array_impl<N> {};

class vertex_array : vertex_array_impl<1> {
public:
  explicit operator unsigned int() const noexcept { return values[0].value; }
  [[nodiscard]] gl::vertex_array_id id() const noexcept { return values[0]; }
  void bind() const { gl::bind_vertex_array(id()); }
  static void unbind() { gl::bind_vertex_array(gl::vertex_array_id{0}); }
};

} // namespace dpsg

#endif // GUARD_DPSG_BUFFERS_HEADER