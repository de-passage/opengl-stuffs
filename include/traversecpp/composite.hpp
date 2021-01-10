#ifndef GUARD_DPSG_COMPOSITE_HPP
#define GUARD_DPSG_COMPOSITE_HPP

#include <type_traits>
#include "./fold.hpp"
#include "./traverse.hpp"

namespace dpsg {

template <class... Args>
struct composite {
  constexpr composite() noexcept {}
  template <
      class... Args2,
      std::enable_if_t<
          std::conjunction_v<std::is_convertible<std::decay_t<Args2>, Args>...>,
          int> = 0>

  constexpr explicit composite(Args2&&... args) noexcept
      : components{std::forward<Args2>(args)...} {}

  std::tuple<Args...> components;

  template <class C,
            class F,
            class... Args2,
            std::enable_if_t<std::is_base_of_v<composite, C>, int> = 0>
  constexpr friend void dpsg_traverse(const C& c, F&& f, Args2&&... args) {
    f(c,
      next(c, f, dpsg::feed_t<composite, std::index_sequence_for>{}),
      std::forward<Args2>(args)...);
  }

  template <class C,
            class F,
            class A,
            class... Args2,
            std::enable_if_t<std::is_base_of_v<composite, C>, int> = 0>
  constexpr friend auto dpsg_fold(const C& c,
                                  A&& a,
                                  F&& f,
                                  Args2&... args) noexcept {
    return f(std::forward<A>(a),
             c,
             next_fold<0, feed_t<composite, detail::parameter_count>>(c, f),
             args...);
  }

 private:
  template <std::size_t N, class Count, class C, class F, std::size_t... Is>
  constexpr static auto next_fold(C&& c, F&& f) noexcept {
    return [&c, &f](auto&& acc, [[maybe_unused]] auto&&... user_input) {
      if constexpr (N >= Count::value) {
        (void)(c);
        (void)(f);
        return acc;
      }
      else {
        return dpsg::fold(std::get<N>(c.components),
                          next_fold<N + 1, Count>(c, f)(
                              std::forward<decltype(acc)>(acc), user_input...),
                          f,
                          user_input...);
      }
    };
  }

  template <class C,
            class F,
            std::size_t... Is,
            std::enable_if_t<std::is_base_of_v<composite, C>, int> = 0>
  constexpr static auto next(
      [[maybe_unused]] const C& c1,
      [[maybe_unused]] F&& f,
      [[maybe_unused]] std::index_sequence<Is...> seq) noexcept {
    return [&c1, &f](auto&&... user_input) {
      if constexpr (sizeof...(Is) == 0) {
        // suppresses warnings in the case of a composite with no elements
        // there doesn't seem to be a way to apply [[maybe_unused]] to a
        // lambda capture list
        (void)(c1);
        (void)(f);
      }
      else {
        (dpsg::traverse(std::get<Is>(c1.components),
                        f,
                        std::forward<decltype(user_input)>(user_input)...),
         ...);
      }
    };
  }
};

template <class... Args>
composite(Args&&...) -> composite<std::decay_t<Args>...>;

}  // namespace dpsg

#endif  // GUARD_DPSG_COMPOSITE_HPP
