#ifndef GUARD_DPSG_LAYOUT_HEADER
#define GUARD_DPSG_LAYOUT_HEADER

#include "opengl.hpp"

#include <type_traits>
#include <utility>

namespace dpsg {
template <std::size_t N> using group = gl::vec_t<N, void>;
template <class... Ts> struct packed {};
template <class... Ts> struct sequenced {};
template <class T, class L> struct layout {};
template <class T, std::size_t N> struct vertex_indices {};

namespace detail {

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
  constexpr static inline gl::element_count count{(Args + ...)};
  using layout_type = packed<group<Args>...>;
  using value_type = std::remove_cv_t<std::remove_reference_t<T>>;

  template <std::size_t N> static void set_attrib_pointer() {
    set_attrib_pointer_impl<N>(std::make_index_sequence<sizeof...(Args)>{});
  }

  static void enable() {
    enable_impl(std::make_index_sequence<sizeof...(Args)>{});
  }

  static void disable() {
    disable_impl(std::make_index_sequence<sizeof...(Args)>{})
  }

private:
  template <std::size_t N, std::size_t... Is>
  static void
  set_attrib_pointer_impl([[maybe_unused]] std::index_sequence<Is...> indices) {
    // NOLINTNEXTLINE
    (gl::vertex_attrib_pointer<value_type>(
         gl::index{Is}, gl::element_count{detail::at_v<Is, Args...>},
         gl::stride{count.value}, gl::offset{detail::sum_to_v<Is, Args...>}),
     ...);
  }

  template <std::size_t... Is>
  static void enable_impl([[maybe_unused]] std::index_sequence<Is...> indices) {
    (gl::enable_vertex_attrib_array(Is), ...);
  }

  template <std::size_t... Is>
  static void
  disable_impl([[maybe_unused]] std::index_sequence<Is...> indices) {
    (gl::disable_vertex_attrib_array(Is), ...);
  }
};

template <class T, std::size_t... Args>
struct layout<T, sequenced<group<Args>...>> {
  constexpr static inline gl::element_count count{(Args + ...)};
  using layout_type = sequenced<group<Args>...>;
  using value_type = std::remove_cv_t<std::remove_reference_t<T>>;

  template <std::size_t N> static void set_attrib_pointer() {
    set_attrib_pointer_impl<N>(std::make_index_sequence<sizeof...(Args)>{});
  }

  static void enable() {
    enable_impl(std::make_index_sequence<sizeof...(Args)>{});
  }

  static void disable() {
    disable_impl(std::make_index_sequence<sizeof...(Args)>{})
  }

private:
  template <std::size_t N, std::size_t... Is>
  static void
  set_attrib_pointer_impl([[maybe_unused]] std::index_sequence<Is...> indices) {
    // NOLINTNEXTLINE
    (gl::vertex_attrib_pointer<value_type>(
         gl::index{Is}, gl::element_count{detail::at_v<Is, Args...>},
         gl::stride{0}, gl::offset{Is * N / sizeof...(Args)}),
     ...);
  }

  template <std::size_t... Is>
  static void enable_impl([[maybe_unused]] std::index_sequence<Is...> indices) {
    (gl::enable_vertex_attrib_array(Is), ...);
  }

  template <std::size_t... Is>
  static void
  disable_impl([[maybe_unused]] std::index_sequence<Is...> indices) {
    (gl::disable_vertex_attrib_array(Is), ...);
  }
};
} // namespace dpsg
#endif // GUARD_DPSG_LAYOUT_HEADER