#ifndef GUARD_DPSG_LAYOUT_HEADER
#define GUARD_DPSG_LAYOUT_HEADER

#include "glad/glad.h"

#include <type_traits>
#include <utility>

namespace dpsg {
template <std::size_t N> struct group : std::integral_constant<std::size_t, N> {
  static_assert(N >= 1 && N <= 4,
                "Invalid OpenGL vec size. Must be between 1 and 4.");
};
template <class... Ts> struct packed {};
template <class... Ts> struct sequenced {};
template <class T, class L> struct layout {};
template <class T, std::size_t N> struct vertex_indices {};

namespace detail {
template <class T> struct deduce_gl_enum;
template <> struct deduce_gl_enum<float> {
  constexpr static inline int value = GL_FLOAT;
};
template <> struct deduce_gl_enum<int> {
  constexpr static inline int value = GL_INT;
};
template <> struct deduce_gl_enum<unsigned int> {
  constexpr static inline int value = GL_UNSIGNED_INT;
};
template <> struct deduce_gl_enum<char> {
  constexpr static inline int value = GL_BYTE;
};
template <> struct deduce_gl_enum<unsigned char> {
  constexpr static inline int value = GL_UNSIGNED_BYTE;
};

template <class T>
constexpr static inline int deduce_gl_enum_v = deduce_gl_enum<T>::value;

template <std::size_t I, auto S, auto... Ss> struct at : at<I - 1, Ss...> {};
template <auto S, auto... Ss> struct at<0, S, Ss...> {
  constexpr static inline auto value = S;
};

template <std::size_t I, auto... Ss>
constexpr static inline auto at_v = at<I, Ss...>::value;

template <std::size_t I, auto... Ss> struct sum_to;
template <std::size_t I, auto S, auto... Ss> struct sum_to<I, S, Ss...> {
  constexpr static inline auto value = S + sum_to<I - 1, Ss...>::value;
};
template <auto S, auto... Ss> struct sum_to<0, S, Ss...> {
  constexpr static inline auto value = 0;
};
template <auto... Ss> struct sum_to<0, Ss...> {
  constexpr static inline auto value = 0;
};
template <std::size_t I, auto... Ss>
constexpr static inline auto sum_to_v = sum_to<I, Ss...>::value;

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
  static void
  set_attrib_pointer_impl([[maybe_unused]] std::index_sequence<Is...> indices) {
    // NOLINTNEXTLINE
    (glVertexAttribPointer(
         Is, detail::at_v<Is, Args...>, detail::deduce_gl_enum_v<value_type>,
         GL_FALSE, count * sizeof(value_type),
         reinterpret_cast<void *>(sizeof(value_type) *
                                  detail::sum_to_v<Is, Args...>)),
     ...);
    (glEnableVertexAttribArray(Is), ...);
  }
};
} // namespace dpsg
#endif // GUARD_DPSG_LAYOUT_HEADER