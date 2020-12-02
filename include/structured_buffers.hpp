#ifndef GUARD_DPSG_STRUCTURED_BUFFERS_HEADER
#define GUARD_DPSG_STRUCTURED_BUFFERS_HEADER

#include "buffers.hpp"
#include "common.hpp"
#include "layout.hpp"
#include "opengl.hpp"
#include "utility.hpp"

#include <cassert>
#include <type_traits>
#include <utility>

namespace dpsg {
template <class Layout> struct structured_buffer {
  using layout_type = typename Layout::layout_type;
  using value_type = typename Layout::value_type;
  constexpr static inline gl::element_count layout_count = Layout::count;

protected:
  template <class L>
  using same_layout =
      std::is_same<layout_type, std::remove_cv_t<std::remove_reference_t<L>>>;
  template <class Input>
  using same_type =
      std::is_same<value_type,
                   std::remove_cv_t<std::remove_reference_t<Input>>>;

public:
  template <class Input, class L, std::size_t N,
            std::enable_if_t<
                std::conjunction_v<same_type<Input>, same_layout<L>>, int> = 0>
  // NOLINTNEXTLINE
  structured_buffer([[maybe_unused]] L l, Input (&i)[N])
      : structured_buffer(i) {}

  template <class Input, std::size_t N,
            std::enable_if_t<same_type<Input>::value, int> = 0>
  // NOLINTNEXTLINE
  explicit structured_buffer(Input (&i)[N]) {
    static_assert(
        N % layout_count.value == 0,
        "Invalid array dimension: the input array size must be a multiple "
        "of the layout element count");
    vao.bind();
    vbo.bind();
    vbo.set_data(i);
    Layout::template set_attrib_pointer<N>();
  }

  [[nodiscard]] const vertex_array &get_vertex_array() const { return vao; }
  [[nodiscard]] const vertex_buffer &get_vertex_buffer() const { return vbo; }

private:
  vertex_buffer vbo;
  vertex_array vao;
};

namespace detail {
template <class T, class L>
using decayed_layout = layout<std::remove_cv_t<std::remove_reference_t<T>>,
                              std::remove_cv_t<std::remove_reference_t<L>>>;
} // namespace detail

template <class Input, class Layout, std::size_t N>
// NOLINTNEXTLINE
structured_buffer(Layout, Input (&)[N])
    ->structured_buffer<detail::decayed_layout<Input, Layout>>;

template <class Layout, std::size_t N>
struct fixed_size_structured_buffer : structured_buffer<Layout> {
private:
  using base = structured_buffer<Layout>;
  constexpr static inline gl::element_count element_count{N};
  constexpr static inline gl::element_count layout_count{Layout::count};
  constexpr static inline gl::element_count buffer_count{layout_count.value *
                                                         element_count.value};

  template <std::size_t S>
  struct eq_buffer_count
      : std::integral_constant<bool, S == buffer_count.value> {};
  template <std::size_t S, class T>
  using match_type_and_count =
      std::conjunction<eq_buffer_count<S>,
                       typename base::template same_type<T>>;
  template <std::size_t S, class T, class L>
  using deduceable_layout =
      std::conjunction<match_type_and_count<S, T>,
                       typename base::template same_layout<L>>;

public:
  template <class Input, std::size_t M,
            std::enable_if_t<match_type_and_count<M, Input>::value, int> = 0>
  // NOLINTNEXTLINE
  explicit fixed_size_structured_buffer(Input (&data)[M]) : base{data} {}

  template <class Input, std::size_t M, class L,
            std::enable_if_t<deduceable_layout<M, Input, L>::value, int> = 0>
  // NOLINTNEXTLINE
  fixed_size_structured_buffer([[maybe_unused]] L layout, Input (&data)[M])
      : base{layout, data} {}

  void draw_array(gl::drawing_mode mode = gl::drawing_mode::triangles,
                  gl::position first = gl::position{0},
                  gl::element_count count =
                      fixed_size_structured_buffer::element_count) const {
    assert(first.value + count.value <= element_count.value);
    base::get_vertex_array().bind();
    gl::draw_arrays(mode, first, element_count);
  }
};

template <std::size_t N, class Input, class Layout>
// NOLINTNEXTLINE
fixed_size_structured_buffer(Layout, Input (&data)[N])
    ->fixed_size_structured_buffer<
        detail::decayed_layout<Input, Layout>,
        N / detail::decayed_layout<Input, Layout>::count.value>;

} // namespace dpsg

#endif // GUARD_DPSG_STRUCTURED_BUFFERS_HEADER