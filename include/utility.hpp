#ifndef GUARD_DPSG_UTILITY_HEADER
#define GUARD_DPSG_UTILITY_HEADER

#include <utility>

namespace dpsg {

template <class F> struct on_scope_exit_t {
  F f;
  constexpr explicit on_scope_exit_t(F &&f) : f(std::forward<F>(f)) {}
  ~on_scope_exit_t() noexcept { f(); }
};

template <class F> on_scope_exit_t(F &&f) -> on_scope_exit_t<F>;

enum class ExecutionStatus { Success = EXIT_SUCCESS, Failure = EXIT_FAILURE };

} // namespace dpsg

#endif // GUARD_DPSG_UTILITY_HEADER