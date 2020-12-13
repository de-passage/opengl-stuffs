#ifndef GUARD_DPSG_RESULT_HEADER
#define GUARD_DPSG_RESULT_HEADER
#include <type_traits>
#include <utility>
#include <variant>

namespace dpsg {
struct in_place_error_t {
} constexpr static inline in_place_error;
struct in_place_success_t {
} constexpr static inline in_place_success;

template <class Success, class Error>
class result {
 public:
  template <class T>
  constexpr explicit result(T&& value) noexcept
      : _value(std::forward<T>(value)) {}

  template <class... Args>
  constexpr explicit result(
      [[maybe_unused]] in_place_success_t success,
      Args&&... args) noexcept(std::is_nothrow_constructible_v<Success,
                                                               Args...>)
      : _value(std::in_place_type<Success>, std::forward<Args>(args)...) {}

  template <class... Args>
  constexpr explicit result(
      [[maybe_unused]] in_place_error_t error,
      Args&&... args) noexcept(std::is_nothrow_constructible_v<Error, Args...>)
      : _value(std::in_place_type<Error>, std::forward<Args>(args)...) {}

  [[nodiscard]] inline constexpr bool has_value() const {
    return _value.index() == 0;
  }
  [[nodiscard]] inline constexpr bool is_error() const {
    return _value.index() == 1;
  }
  [[nodiscard]] inline constexpr bool is_success() const { return has_value(); }
  [[nodiscard]] inline constexpr operator bool() const {  // NOLINT
    return has_value();
  }

 private:
  template <class F, class P>
  constexpr static inline bool noexcept_call =
      noexcept(std::forward<F>(std::declval<F>())(std::declval<P>()));

 public:
  template <class F,
            class G,
            class R = std::common_type_t<std::invoke_result<F, Success&>,
                                         std::invoke_result<G, Error&>>>
  constexpr inline R either(F&& on_success, G&& on_error) & noexcept(
      noexcept_call<F, Success&>&& noexcept_call<G, Error&>) {
    if (Success* ptr = std::get_if<0>(&_value)) {
      return std::forward<F>(on_success)(*ptr);
    }
    return std::forward<G>(on_error)(std::get<1>(_value));
  }

  template <class F,
            class G,
            class R = std::common_type_t<std::invoke_result<F, const Success&>,
                                         std::invoke_result<G, const Error&>>>
  constexpr inline R either(F&& on_success, G&& on_error) const& noexcept(
      noexcept_call<F, const Success&>&& noexcept_call<G, const Error&>) {
    if (const Success* ptr = std::get_if<0>(&_value)) {
      return std::forward<F>(on_success)(*ptr);
    }
    return std::forward<G>(on_error)(std::get<1>(_value));
  }

  template <class F,
            class G,
            class R = std::common_type_t<std::invoke_result<F, Success&&>,
                                         std::invoke_result<G, Error&&>>>
  constexpr inline R either(F&& on_success, G&& on_error) && noexcept(
      noexcept_call<F, Success&&>&& noexcept_call<G, Error&&>) {
    if (Success* ptr = std::get_if<0>(&_value)) {
      return std::forward<F>(on_success)(std::move(*ptr));
    }
    return std::forward<G>(on_error)(std::get<1>(std::move(_value)));
  }

  template <class F,
            class G,
            class R = std::common_type_t<std::invoke_result<F, const Success&&>,
                                         std::invoke_result<G, const Error&&>>>
  constexpr inline R either(F&& on_success, G&& on_error) const&& noexcept(
      noexcept_call<F, const Success&&>&& noexcept_call<G, const Error&&>) {
    if (const Success* ptr = std::get_if<0>(&_value)) {
      return std::forward<F>(on_success)(std::move(*ptr));
    }
    return std::forward<G>(on_error)(std::get<1>(std::move(_value)));
  }

  [[nodiscard]] constexpr inline const Success& value() const& {
    if (Success* ptr = std::get_if<0>(&_value)) {
      return *ptr;
    }
    throw std::get<1>(_value);
  }

  [[nodiscard]] constexpr inline Success& value() & {
    if (const Success* ptr = std::get_if<0>(&_value)) {
      return *ptr;
    }
    throw std::get<1>(_value);
  }

  [[nodiscard]] constexpr inline const Success&& value() const&& {
    if (Success* ptr = std::get_if<0>(&_value)) {
      return std::move(*ptr);
    }
    throw std::get<1>(std::move(_value));
  }

  [[nodiscard]] constexpr inline Success& value() && {
    if (const Success* ptr = std::get_if<0>(&_value)) {
      return std::move(*ptr);
    }
    throw std::get<1>(std::move(_value));
  }

  template <class T = Success,
            std::enable_if_t<std::is_convertible_v<T, Success>>>
  [[nodiscard]] constexpr inline Success value_or(T default_) const& noexcept {
    if (const Success* ptr = std::get_if<0>(&_value)) {
      return *ptr;
    }
    return static_cast<Success>(default_);
  }

 private:
  std::variant<Success, Error> _value;
};
}  // namespace dpsg

#endif  // GUARD_DPSG_RESULT_HEADER