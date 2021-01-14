#ifndef GUARD_DPSG_CONTROL_SCHEME_HPP
#define GUARD_DPSG_CONTROL_SCHEME_HPP

#include <type_traits>
#include <utility>
#include "traversecpp/is_template_instance.hpp"
#include "traversecpp/traverse.hpp"

namespace control {
struct base_input {};
template <class T>
using is_input = std::is_base_of<base_input, std::decay_t<T>>;
template <class T>
constexpr static inline bool is_input_v = is_input<T>::value;

template <class Marker, class Action, class... Bindings>
struct input : base_input {
  static_assert(sizeof...(Bindings) > 0, "Actions need at least 1 binding");
  template <class A, class... Bs>
  constexpr explicit input([[maybe_unused]] Marker marker,
                           A&& action,
                           Bs&&... bindings) noexcept
      : action{std::forward<A>(action)},
        bindings{std::forward<Bs>(bindings)...} {}
  template <class A, class... Bs>
  constexpr explicit input(A&& action, Bs&&... bindings) noexcept
      : action{std::forward<A>(action)},
        bindings{std::forward<Bs>(bindings)...} {}
  Action action;
  std::tuple<Bindings...> bindings;

  constexpr static inline Marker marker{};

  template <
      class I,
      class F,
      class... Args,
      std::enable_if_t<std::is_base_of_v<input, std::decay_t<I>>, int> = 0>
  friend constexpr void dpsg_traverse(I&& input,
                                      F&& f,
                                      Args&&... args) noexcept {
    dpsg::traverse(std::forward<I>(input).bindings,
                   std::forward<F>(f),
                   std::forward<I>(input).marker,
                   std::forward<I>(input).action,
                   std::forward<Args>(args)...);
  }
};
template <class M, class A, class... Bs>
input(M, A&&, Bs&&...) -> input<M, std::decay_t<A>, std::decay_t<Bs>...>;

template <class... Args>
struct control_scheme {
  static_assert(std::conjunction_v<is_input<Args>...>);
  template <
      class... Args2,
      std::enable_if_t<std::conjunction_v<std::is_convertible<Args2, Args>...>,
                       int> = 0>
  constexpr explicit inline control_scheme(Args2&&... args) noexcept
      : controls{std::forward<Args2>(args)...} {}

  std::tuple<Args...> controls;

  template <class S,
            class F,
            class... Args2,
            std::enable_if_t<std::is_base_of_v<control_scheme, std::decay_t<S>>,
                             int> = 0>
  friend constexpr void dpsg_traverse(S&& s, F&& f, Args2&&... rest) noexcept {
    dpsg::traverse(
        std::forward<S>(s).controls,
        [&f](auto&& ctrl, auto&&... rest) {
          dpsg::traverse(std::forward<decltype(ctrl)>(ctrl),
                         std::forward<F>(f),
                         std::forward<decltype(rest)>(rest)...);
        },
        std::forward<Args2>(rest)...);
  }
};

template <class... Args>
control_scheme(Args&&...) -> control_scheme<std::decay_t<Args>...>;

template <class Cs,
          class... Args,
          std::enable_if_t<std::conjunction_v<is_input<Args>...>, int> = 0>
constexpr auto combine(Cs&& cs, Args&&... args) noexcept {
  return std::apply(
      [&args...](auto&&... ctls) {
        return control_scheme{std::forward<decltype(ctls)>(ctls)...,
                              std::forward<Args>(args)...};
      },
      std::forward<Cs>(cs).controls);
}

namespace detail {
template <class T, class = void>
struct has_controls : std::false_type {};

template <class T>
struct has_controls<T, std::void_t<decltype(std::declval<T>().controls)>>
    : std::true_type {};

template <class T>
constexpr static inline bool has_controls_v = has_controls<T>::value;
}  // namespace detail

template <class C1,
          class C2,
          std::enable_if_t<std::conjunction_v<detail::has_controls<C1>,
                                              detail::has_controls<C2>>,
                           int> = 0>
constexpr auto combine(C1&& c1, C2&& c2) noexcept {
  return std::apply(
      [](auto&&... cs) {
        return control_scheme{std::forward<decltype(cs)>(cs)...};
      },
      std::tuple_cat(std::forward<C1>(c1).controls,
                     std::forward<C2>(c2).controls));
}

template <class T1,
          class T2,
          class... Args,
          std::enable_if_t<std::disjunction_v<std::negation<is_input<T2>>,
                                              std::negation<is_input<Args>>...>,
                           int> = 0>
constexpr auto combine(T1&& t1, T2&& t2, Args&&... args) noexcept {
  return combine(combine(std::forward<T1>(t1), std::forward<T2>(t2)),
                 std::forward<Args>(args)...);
}
}  // namespace control

#endif  // GUARD_DPSG_CONTROL_SCHEME_HPP