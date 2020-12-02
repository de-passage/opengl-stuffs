#ifndef GUARD_DPSG_META_IS_ONE_OF_HEADER
#define GUARD_DPSG_META_IS_ONE_OF_HEADER

#include <type_traits>

namespace dpsg {
template <class T, class... Args>
using is_one_of = std::disjunction<std::is_same<T, Args>...>;
template <class T, class... Args>
constexpr static inline bool is_one_of_v = is_one_of<T, Args...>::value;

} // namespace dpsg

#endif // GUARD_DPSG_META_IS_ONE_OF_HEADER