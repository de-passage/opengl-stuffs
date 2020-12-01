#include "moving_polygon.hpp"

#include "buffers.hpp"
#include "key_mapper.hpp"
#include "load_shaders.hpp"
#include "utils.hpp"
#include "common.hpp"

#include <type_traits>

namespace dpsg {
template <std::size_t N>
struct group : std::integral_constant<std::size_t, N> {
  static_assert(N >= 1 && N <= 4, "Unacceptable OpenGL vec size");
};
template <class... Ts> struct packed {};
template <class... Ts> struct sequenced {};
template <class T, class L> struct layout {};

namespace detail {
template<class T>
struct deduce_gl_enum;
template<> struct deduce_gl_enum<float> {
  constexpr static inline int value = GL_FLOAT;
};

template<class T> constexpr static inline int deduce_gl_enum_v = deduce_gl_enum<T>::value;

template<std::size_t I, auto S, auto ...Ss> struct at : at<I - 1, Ss...> {};
template<auto S, auto ...Ss> struct at<0, S, Ss...> {
  constexpr static inline auto value  = S;
};

template<std::size_t I, auto ...Ss> constexpr static inline auto at_v = at<I, Ss...>::value;

struct empty{};
template<std::size_t I, auto...Ss> struct sum_to;
template<std::size_t I, auto S, auto ...Ss> struct sum_to<I, S, Ss...> {
  constexpr static inline auto value = S + sum_to<I - 1, Ss...>::value;
};
template<auto S, auto ...Ss> struct sum_to<0, S, Ss...> {
  constexpr static inline auto value = 0;
};
template<auto ...Ss> struct sum_to<0, Ss...> {
  constexpr static inline auto value = 0;
};
template<std::size_t I, auto ...Ss> constexpr static inline auto sum_to_v = sum_to<I, Ss...>::value;

} // namespace detail

template <class T, std::size_t... Args>
struct layout<T, packed<group<Args>...>> {
  constexpr static inline std::size_t count = (Args + ...);
  using layout_type = packed<group<Args>...>;
  using value_type = std::remove_cv_t<std::remove_reference_t<T>>;

  template <std::size_t N> static void set_attrib_pointer() {
    set_attrib_pointer_impl<N>(std::make_index_sequence<sizeof...(Args)>{});
  }

private:
  template <std::size_t N, std::size_t... Is>
  static void set_attrib_pointer_impl([[maybe_unused]] std::index_sequence<Is...> indices) {
    // NOLINTNEXTLINE
    (glVertexAttribPointer(Is, detail::at_v<Is, Args...>, detail::deduce_gl_enum_v<value_type>, GL_FALSE, count * sizeof(value_type), reinterpret_cast<void*>(sizeof(value_type) * detail::sum_to_v<Is, Args...>)), ...);
    (glEnableVertexAttribArray(Is), ...);
  }
};

template <class Layout> struct structured_buffer {
  using layout_type = typename Layout::layout_type;
  using value_type = typename Layout::value_type;
  constexpr static inline std::size_t layout_count = Layout::count;

  protected:
  template<class L>
  using same_layout = std::is_same<layout_type, std::remove_cv_t<std::remove_reference_t<L>>>;
  template<class Input>
  using same_type = std::is_same<value_type, std::remove_cv_t<std::remove_reference_t<Input>>>;

  public:
  template<class Input, class L, std::size_t N, std::enable_if_t<std::conjunction_v<same_type<Input>, same_layout<L>> , int> = 0>
  // NOLINTNEXTLINE
  structured_buffer([[maybe_unused]] L l, Input (&i)[N]) : structured_buffer(i) {}

  template <class Input, std::size_t N,
            std::enable_if_t<
                same_type<Input>::value,
                int> = 0>
                // NOLINTNEXTLINE
  explicit structured_buffer(Input (&i)[N]) {
    static_assert(N % layout_count == 0,
                  "Invalid array dimension: the input array size must be a multiple "
                  "of the layout element count");
    vao.bind();
    vbo.bind();
    vbo.set_data(i);
    Layout::template set_attrib_pointer<N>();
  }

  [[nodiscard]] const vertex_array& get_vertex_array() const { return vao; }
  [[nodiscard]] const vertex_buffer& get_vertex_buffer() const { return vbo; }

private:
  vertex_buffer vbo;
  vertex_array vao;
};

namespace detail {
template<class T, class L> using decayed_layout = layout<std::remove_cv_t<std::remove_reference_t<T>>, std::remove_cv_t<std::remove_reference_t<L>>>;
} // namespace detail

template<class Input, class Layout, std::size_t N> 
// NOLINTNEXTLINE
structured_buffer(Layout, Input (&)[N]) -> structured_buffer<detail::decayed_layout<Input, Layout>>;

template<class Layout, std::size_t N> struct fixed_size_structured_buffer : structured_buffer<Layout> {
  private:
  using base = structured_buffer<Layout>;
  constexpr static inline std::size_t element_count = N;
  constexpr static inline std::size_t layout_count = Layout::count;
  constexpr static inline std::size_t buffer_count = layout_count * element_count;

  template<std::size_t S> struct eq_buffer_count : std::integral_constant<bool, S == buffer_count> {};
  template<std::size_t S, class T>
  using match_type_and_count = std::conjunction<eq_buffer_count<S>, typename base::template same_type<T>>;
  template<std::size_t S, class T, class L>
  using deduceable_layout = std::conjunction<match_type_and_count<S, T>, typename base::template same_layout<L>>;

  public:
  template<class Input, std::size_t M, std::enable_if_t<match_type_and_count<M, Input>::value, int> = 0>
  // NOLINTNEXTLINE
  explicit fixed_size_structured_buffer(Input (&data)[M]) : base{data} {
  }

  template<class Input, std::size_t M, class L, 
    std::enable_if_t<deduceable_layout<M, Input, L>::value, int> = 0>
    // NOLINTNEXTLINE
  fixed_size_structured_buffer([[maybe_unused]] L layout, Input (&data)[M]) : base{layout, data} {}

  void draw_array(drawing_mode mode = drawing_mode::triangles, std::size_t first = 0, std::size_t count = element_count) const {
    assert(first + count <= element_count);
    base::get_vertex_array().bind();
    glDrawArrays(static_cast<int>(mode), first, element_count);
  }
};

template<std::size_t N, class Input, class Layout>
//NOLINTNEXTLINE
fixed_size_structured_buffer(Layout, Input (&data)[N]) -> fixed_size_structured_buffer<detail::decayed_layout<Input, Layout>, N / detail::decayed_layout<Input, Layout>::count>;
} // namespace dpsg

void moving_polygon(dpsg::window &wdw) {

using namespace dpsg;
using namespace dpsg::input;
  key_mapper kmap;
  kmap.on(key::escape, close);
  wdw.set_key_callback(kmap);

  auto shader = load(vs_filename{"shaders/basic_with_color.vs"}, fs_filename{"shaders/basic.fs"});

  constexpr float vertices[] = {
      // positions         // colors
      0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
      -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
      0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
  };    
  using packed_layout = packed<group<3>, group<3>>;
  fixed_size_structured_buffer b(packed_layout{}, vertices);

  shader.use();
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  wdw.render_loop([&] {
    glClear(GL_COLOR_BUFFER_BIT);

    b.draw_array();
  });
}