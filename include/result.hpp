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
 private:
  using variant_type = std::variant<Success, Error>;
  variant_type _value;
  constexpr static inline std::size_t success_index = 0;
  constexpr static inline std::size_t error_index = 1;

 public:
  using success_type = Success;
  using error_type = Error;

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

  template <class V,
            class S = typename V::success_type,
            class E = typename V::error_type,
            std::enable_if_t<
                std::conjunction_v<
                    std::is_convertible<S, Success>,
                    std::is_convertible<E, Error>,
                    std::disjunction<std::negation<std::is_same<S, Success>>,
                                     std::negation<std::is_same<E, Error>>>>,
                int> = 0>
  constexpr result(V&& val)
      : _value(std::forward<V>(val).either(
            [](auto&& success) -> variant_type {
              return variant_type{std::in_place_type<success_type>,
                                  std::forward<decltype(success)>(success)};
            },
            [](auto&& error) -> variant_type {
              return variant_type{std::in_place_type<error_type>,
                                  std::forward<decltype(error)>(error)};
            })) {}

  [[nodiscard]] inline constexpr bool has_value() const {
    return _value.index() == success_index;
  }
  [[nodiscard]] inline constexpr bool is_error() const {
    return _value.index() == error_index;
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
            class R = std::common_type_t<std::invoke_result_t<F, Success&>,
                                         std::invoke_result_t<G, Error&>>>
  constexpr inline R either(F&& on_success, G&& on_error) & noexcept(
      noexcept_call<F, Success&>&& noexcept_call<G, Error&>) {
    if (Success* ptr = std::get_if<success_index>(&_value)) {
      return std::forward<F>(on_success)(*ptr);
    }
    return std::forward<G>(on_error)(error());
  }

  template <
      class F,
      class G,
      class R = std::common_type_t<std::invoke_result_t<F, const Success&>,
                                   std::invoke_result_t<G, const Error&>>>
  constexpr inline R either(F&& on_success, G&& on_error) const& noexcept(
      noexcept_call<F, const Success&>&& noexcept_call<G, const Error&>) {
    if (const Success* ptr = std::get_if<success_index>(&_value)) {
      return std::forward<F>(on_success)(*ptr);
    }
    return std::forward<G>(on_error)(error());
  }

  template <class F,
            class G,
            class R = std::common_type_t<std::invoke_result_t<F, Success&&>,
                                         std::invoke_result_t<G, Error&&>>>
  constexpr inline R either(F&& on_success, G&& on_error) && noexcept(
      noexcept_call<F, Success&&>&& noexcept_call<G, Error&&>) {
    if (Success* ptr = std::get_if<success_index>(&_value)) {
      return std::forward<F>(on_success)(std::move(*ptr));
    }
    return std::forward<G>(on_error)(error());
  }

  template <
      class F,
      class G,
      class R = std::common_type_t<std::invoke_result_t<F, const Success&&>,
                                   std::invoke_result_t<G, const Error&&>>>
  constexpr inline R either(F&& on_success, G&& on_error) const&& noexcept(
      noexcept_call<F, const Success&&>&& noexcept_call<G, const Error&&>) {
    if (const Success* ptr = std::get_if<success_index>(&_value)) {
      return std::forward<F>(on_success)(std::move(*ptr));
    }
    return std::forward<G>(on_error)(error());
  }

  [[nodiscard]] constexpr inline const Success& value() const& {
    if (const Success* ptr = std::get_if<success_index>(&_value)) {
      return *ptr;
    }
    throw error();
  }

  [[nodiscard]] constexpr inline Success& value() & {
    if (Success* ptr = std::get_if<success_index>(&_value)) {
      return *ptr;
    }
    throw error();
  }

  [[nodiscard]] constexpr inline const Success&& value() const&& {
    if (const Success* ptr = std::get_if<success_index>(&_value)) {
      return std::move(*ptr);
    }
    throw error();
  }

  [[nodiscard]] constexpr inline Success&& value() && {
    if (Success* ptr = std::get_if<success_index>(&_value)) {
      return std::move(*ptr);
    }
    throw std::get<error_index>(std::move(_value));
  }

  template <class T = Success,
            std::enable_if_t<std::is_convertible_v<T, Success>>>
  [[nodiscard]] constexpr inline Success value_or(T default_) const& noexcept {
    if (const Success* ptr = std::get_if<success_index>(&_value)) {
      return *ptr;
    }
    return static_cast<Success>(default_);
  }

  [[nodiscard]] constexpr Error& error() & noexcept {
    return std::get<error_index>(_value);
  }

  [[nodiscard]] constexpr const Error& error() const& noexcept {
    return std::get<error_index>(_value);
  }

  [[nodiscard]] constexpr Error&& error() && noexcept {
    return std::get<error_index>(std::move(_value));
  }

  [[nodiscard]] constexpr const Error&& error() const&& noexcept {
    return std::get<error_index>(std::move(_value));
  }

  [[nodiscard]] constexpr variant_type& to_variant() & noexcept {
    return _value;
  }

  [[nodiscard]] constexpr const variant_type& to_variant() const& noexcept {
    return _value;
  }

  [[nodiscard]] constexpr variant_type&& to_variant() && noexcept {
    return std::move(_value);
  }

  [[nodiscard]] constexpr const variant_type&& to_variant() const&& noexcept {
    return std::move(_value);
  }
};
}  // namespace dpsg

#endif  // GUARD_DPSG_RESULT_HEADER