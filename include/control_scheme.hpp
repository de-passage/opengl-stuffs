#ifndef GUARD_DPSG_CONTROL_SCHEME_HPP
#define GUARD_DPSG_CONTROL_SCHEME_HPP

#include <type_traits>
#include <utility>
#include "traversecpp/is_template_instance.hpp"
#include "traversecpp/traverse.hpp"

namespace control {
struct action {};
template <class T>
using is_action = std::is_base_of<action, std::decay_t<T>>;
template <class T>
constexpr static inline bool is_action_v = is_action<T>::value;

struct base_input {};
template <class T>
using is_input = std::is_base_of<base_input, std::decay_t<T>>;
template <class T>
constexpr static inline bool is_input_v = is_input<T>::value;

struct repeated {};
struct one_time {};

template <class CRTP, class Action, class... Bindings>
struct input : base_input {
  static_assert(sizeof...(Bindings) > 0, "Actions need at least 1 binding");
  template <class A, class... Bs>
  constexpr explicit input(A&& action, Bs&&... bindings) noexcept
      : action{std::forward<A>(action)},
        bindings{std::forward<Bs>(bindings)...} {}
  Action action;
  std::tuple<Bindings...> bindings;

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
                   std::decay_t<I>::marker,
                   std::forward<I>(input).action,
                   std::forward<Args>(args)...);
  }
};
template <class A, class... Bs>
input(A&&, Bs&&...) -> input<std::decay_t<A>, std::decay_t<Bs>...>;

template <class Action, class... Args>
struct repeat : input<repeat<Action, Args...>, Action, Args...> {
  using base = input<repeat<Action, Args...>, Action, Args...>;
  template <class A, class... As, std::enable_if_t<is_action_v<A>, int> = 0>
  constexpr explicit repeat(A&& a, As&&... as)
      : base{std::forward<A>(a), std::forward<As>(as)...} {}

  constexpr static inline repeated marker{};
};
template <class A, class... As>
repeat(A&&, As&&...) -> repeat<std::decay_t<A>, std::decay_t<As>...>;

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
            std::enable_if_t<std::is_same_v<control_scheme, std::decay_t<S>>,
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
}  // namespace control

#endif  // GUARD_DPSG_CONTROL_SCHEME_HPP