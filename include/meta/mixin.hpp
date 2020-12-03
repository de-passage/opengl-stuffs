#ifndef GUARD_DPSG_META_MIXIN_HEADER
#define GUARD_DPSG_META_MIXIN_HEADER

#include <type_traits>

namespace dpsg {
template <class... Ts> struct mixin;
template <class T> struct mixin<T> { using crtp_type = T; };
template <class T, class F, class... Ts>
struct mixin<T, F, Ts...> : F::template type<mixin<T, Ts...>> {
protected:
  using base = typename F::template type<mixin<T, Ts...>>;
  template <class... Us>
  constexpr explicit mixin(Us &&... us) noexcept(
      std::is_nothrow_constructible_v<base, Us &&...>)
      : base(std::forward<Us>(us)...) {}
};

template <class T> struct real_type;
template <class... Ts> struct real_type<mixin<Ts...>> {
  using type = typename mixin<Ts...>::crtp_type;
};
template <class T> using real_type_t = typename real_type<T>::type;

} // namespace dpsg

#endif // GUARD_DPSG_META_MIXIN_HEADER