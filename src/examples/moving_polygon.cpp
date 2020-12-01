#include "moving_polygon.hpp"
#include "buffers.hpp"
#include "key_mapper.hpp"
#include "utils.hpp"
#include "load_shaders.hpp"
#include <type_traits>

using namespace dpsg;
using namespace dpsg::input;

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
  constexpr static inline std::size_t total_size = (Args + ...);
  using layout_type = packed<group<Args>...>;
  using value_type = std::remove_cv_t<std::remove_reference_t<T>>;

  template <std::size_t N> static void set_attrib_pointer() {
    set_attrib_pointer_impl<N>(std::make_index_sequence<sizeof...(Args)>{});
  }

private:
  template <std::size_t N, std::size_t... Is>
  static void set_attrib_pointer_impl([[maybe_unused]] std::index_sequence<Is...> indices) {
    (glVertexAttribPointer(Is, ::detail::at_v<Is, Args...>, ::detail::deduce_gl_enum_v<value_type>, GL_FALSE, total_size * sizeof(value_type), reinterpret_cast<void*>(sizeof(value_type) * ::detail::sum_to_v<Is, Args...>)), ...);
    (glEnableVertexAttribArray(Is), ...);
  }
};

template <class Layout> struct structured_buffer {
  using layout_type = typename Layout::layout_type;
  using value_type = typename Layout::value_type;

  private:
  template<class Input>
  using same_type = std::is_same<value_type, std::remove_cv_t<std::remove_reference_t<Input>>>;
  template<class L>
  using same_layout = std::is_same<layout_type, std::remove_cv_t<std::remove_reference_t<L>>>;

  public:
  template<class Input, class L, std::size_t N, std::enable_if_t<std::conjunction_v<same_type<Input>, same_layout<L>> , int> = 0>
  structured_buffer([[maybe_unused]] L l, Input (&i)[N]) : structured_buffer(i) {}

  template <class Input, std::size_t N,
            std::enable_if_t<
                same_type<Input>::value,
                int> = 0>
  explicit structured_buffer(Input (&i)[N]) {
    static_assert(N % Layout::total_size == 0,
                  "Invalid array dimension: the input size must be a multiple "
                  "of the layout size");
    vao.bind();
    vbo.bind();
    vbo.set_data(i);
    Layout::template set_attrib_pointer<N>();
  }

  vertex_buffer vbo;
  vertex_array vao;
};

template<class Input, class Layout, std::size_t N> 
structured_buffer(Layout, Input (&)[N]) -> structured_buffer<layout<Input, Layout>>;



void moving_polygon(window &wdw) {
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
  structured_buffer b(packed_layout{}, vertices);

  shader.use();
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  wdw.render_loop([&] {
    glClear(GL_COLOR_BUFFER_BIT);

    b.vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
  });
}