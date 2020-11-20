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

  buffer_array_impl &operator=(buffer_array_impl &&) noexcept {

    for (std::size_t i; i < N; ++i) {
      values[i] = std::exchange(a.values[i], 0);
    }
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

class buffer : buffer_array_impl<1> {
public:
  explicit operator unsigned int() const { return values[0]; }
};
} // namespace dpsg

#endif // GUARD_DPSG_BUFFERS_HEADER