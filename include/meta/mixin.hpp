#ifndef GUARD_DPSG_META_MIXIN_HEADER
#define GUARD_DPSG_META_MIXIN_HEADER

#include <type_traits>

namespace dpsg {
template <class... Ts>
struct mixin;
template <class...>
struct tuple {};
template <class T>
struct mixin<T> {
  using crtp_type = T;
  using mixin_t = tuple<>;
};
template <class T, class F, class... Ts>
struct mixin<T, F, Ts...> : F::template type<mixin<T, Ts...>> {
  using mixin_t = tuple<F, Ts...>;

 protected:
  using base = typename F::template type<mixin<T, Ts...>>;
  template <class... Us>
  constexpr explicit mixin(Us&&... us) noexcept(
      std::is_nothrow_constructible_v<base, Us&&...>)
      : base(std::forward<Us>(us)...) {}
};

template <class T>
struct real_type;
template <class... Ts>
struct real_type<mixin<Ts...>> {
  using type = typename mixin<Ts...>::crtp_type;
};

template <class T>
using real_type_t = typename real_type<T>::type;

template <class T>
struct extract_mixin_parameters {
  using type = typename T::mixin_t;
};

template <class T>
using extract_mixin_parameters_t = typename extract_mixin_parameters<T>::type;

namespace detail {
template <class T, class Mixin>
struct contains_mixin : std::false_type {};

template <template <class...> class C, class M, class... Args>
struct contains_mixin<C<M, Args...>, M> : std::true_type {};

template <template <class...> class C, class M1, class M2, class... Args>
struct contains_mixin<C<M1, Args...>, M2> : contains_mixin<C<Args...>, M2> {};
}  // namespace detail

template <class T, class Mixin, class = void>
struct contains_mixin : std::false_type {};

template <class T, class Mixin>
struct contains_mixin<T, Mixin, std::void_t<decltype(typename T::mixin_t{})>>
    : detail::contains_mixin<typename T::mixin_t, Mixin> {};

template <class T, class Mixin>
constexpr static inline bool contains_mixin_v = contains_mixin<T, Mixin>::value;
}  // namespace dpsg

#endif  // GUARD_DPSG_META_MIXIN_HEADER