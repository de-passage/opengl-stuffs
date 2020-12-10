#ifndef GUARD_DPSG_FIXED_SIZE_ELEMENT_BUFFER_HEADER
#define GUARD_DPSG_FIXED_SIZE_ELEMENT_BUFFER_HEADER

#include "opengl.hpp"

#include "buffers.hpp"

#include <type_traits>

namespace dpsg {
template <typename T, std::size_t N>
struct fixed_size_element_buffer {
  static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>,
                "Element buffer objects must be unsigned integral types");
  using value_type = std::decay_t<T>;
  constexpr static inline gl::element_count element_count{N};

  template <
      class U,
      std::enable_if_t<std::is_same_v<std::decay_t<U>, value_type>, int> = 0>
  // NOLINTNEXTLINE
  explicit fixed_size_element_buffer(U (&values)[N]) noexcept {
    _ebo.bind();
    _ebo.set_data(values);
  }

  template <class... Args>
  inline void draw(Args&&... args) const noexcept {
    const gl::offset o = gl::detail::get<gl::offset>(args..., gl::offset{0});
    const gl::drawing_mode mode =
        gl::detail::get<gl::drawing_mode>(args..., gl::drawing_mode::triangles);
    const gl::element_count count = gl::detail::get<gl::element_count>(
        args..., gl::element_count{element_count});

    if constexpr (gl::detail::contains<gl::index>(args...)) {
      gl::draw_elements<value_type>(
          mode, count, o, gl::detail::get<gl::index>(args...));
    }
    else {
      gl::draw_elements<value_type>(mode, count, o);
    }
  }

 private:
  element_buffer _ebo;
};

template <typename T, std::size_t N>
// NOLINTNEXTLINE
fixed_size_element_buffer(T (&)[N])
    ->fixed_size_element_buffer<std::decay_t<T>, N>;
}  // namespace dpsg

#endif  // GUARD_DPSG_FIXED_SIZE_ELEMENT_BUFFER_HEADER