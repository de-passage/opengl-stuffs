#ifndef GUARD_DPSG_C_STR_HEADER
#define GUARD_DPSG_C_STR_HEADER

#include <type_traits>

namespace dpsg {
namespace detail {

template <class Char> using is_char_ptr = std::is_pointer<std::decay_t<Char>>;
template <class Char>
constexpr static inline bool is_char_ptr_v = is_char_ptr<Char>::value;

template <class T, class = void> struct has_c_str_mem_fun : std::false_type {};
template <class T>
struct has_c_str_mem_fun<T, std::void_t<decltype(std::declval<T>().c_str())>>
    : std::true_type {};
template <class T>
constexpr static inline bool has_c_str_mem_fun_v = has_c_str_mem_fun<T>::value;
} // namespace detail

template <typename Char, std::enable_if_t<detail::is_char_ptr_v<Char>, int> = 0>
constexpr std::decay_t<Char> c_str(Char str) noexcept {
  return str;
}

template <class T, std::enable_if_t<detail::has_c_str_mem_fun_v<T &&>, int> = 0>
constexpr decltype(auto)
c_str(T &&str) noexcept(noexcept(std::declval<T &&>().c_str())) {
  return std::forward<T>(str).c_str();
}

} // namespace dpsg

#endif // GUARD_DPSG_C_STR_HEADER