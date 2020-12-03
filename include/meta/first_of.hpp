#ifndef GUARD_DPSG_FIRST_OF_HEADER
#define GUARD_DPSG_FIRST_OF_HEADER

namespace dpsg {
template <class...> struct first_of;
template <class T, class... Ts> struct first_of<T, Ts...> { using type = T; };
template <class... Ts> using first_of_t = typename first_of<Ts...>::type;
} // namespace dpsg

#endif // GUARD_DPSG_FIRST_OF_HEADER