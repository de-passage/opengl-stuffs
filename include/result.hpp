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
  template <class S, class E>
  using variant_t = std::variant<S, E>;
  using variant_type = variant_t<Success, Error>;
  variant_type _value;
  constexpr static inline std::size_t success_index = 0;
  constexpr static inline std::size_t error_index = 1;

 public:
  using success_type = Success;
  using error_type = Error;

  template <class T,
            std::enable_if_t<std::is_convertible_v<T, variant_type>, int> = 0>
  constexpr explicit result(T&& value) noexcept(
      std::is_nothrow_constructible_v<variant_type, T>)
      : _value(std::forward<T>(value)) {}

  template <class... Args>
  constexpr explicit result(
      [[maybe_unused]] in_place_success_t success,
      Args&&... args) noexcept(std::is_nothrow_constructible_v<success_type,
                                                               Args...>)
      : _value(std::in_place_type<Success>, std::forward<Args>(args)...) {}

  template <class... Args>
  constexpr explicit result(
      [[maybe_unused]] in_place_error_t error,
      Args&&... args) noexcept(std::is_nothrow_constructible_v<error_type,
                                                               Args...>)
      : _value(std::in_place_type<Error>, std::forward<Args>(args)...) {}

  template <
      class V,
      class S = typename V::success_type,
      class E = typename V::error_type,
      std::enable_if_t<
          std::conjunction_v<
              std::is_convertible<S, success_type>,
              std::is_convertible<E, error_type>,
              std::disjunction<std::negation<std::is_same<S, success_type>>,
                               std::negation<std::is_same<E, error_type>>>>,
          int> = 0>
  constexpr explicit result(V&& val)
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
            class R = std::common_type_t<std::invoke_result_t<F, success_type&>,
                                         std::invoke_result_t<G, error_type&>>>
  constexpr inline R either(F&& on_success, G&& on_error) & noexcept(
      noexcept_call<F, success_type&>&& noexcept_call<G, error_type&>) {
    if (success_type* ptr = std::get_if<success_index>(&_value)) {
      return std::forward<F>(on_success)(*ptr);
    }
    return std::forward<G>(on_error)(error());
  }

  template <
      class F,
      class G,
      class R = std::common_type_t<std::invoke_result_t<F, const success_type&>,
                                   std::invoke_result_t<G, const error_type&>>>
  constexpr inline R either(F&& on_success, G&& on_error) const& noexcept(
      noexcept_call<F, const success_type&>&&
          noexcept_call<G, const error_type&>) {
    if (const success_type* ptr = std::get_if<success_index>(&_value)) {
      return std::forward<F>(on_success)(*ptr);
    }
    return std::forward<G>(on_error)(error());
  }

  template <
      class F,
      class G,
      class R = std::common_type_t<std::invoke_result_t<F, success_type&&>,
                                   std::invoke_result_t<G, error_type&&>>>
  constexpr inline R either(F&& on_success, G&& on_error) && noexcept(
      noexcept_call<F, success_type&&>&& noexcept_call<G, error_type&&>) {
    if (success_type* ptr = std::get_if<success_index>(&_value)) {
      return std::forward<F>(on_success)(std::move(*ptr));
    }
    return std::forward<G>(on_error)(error());
  }

  template <class F,
            class G,
            class R = std::common_type_t<
                std::invoke_result_t<F, const success_type&&>,
                std::invoke_result_t<G, const error_type&&>>>
  constexpr inline R either(F&& on_success, G&& on_error) const&& noexcept(
      noexcept_call<F, const success_type&&>&&
          noexcept_call<G, const error_type&&>) {
    if (const success_type* ptr = std::get_if<success_index>(&_value)) {
      return std::forward<F>(on_success)(std::move(*ptr));
    }
    return std::forward<G>(on_error)(error());
  }

  [[nodiscard]] constexpr inline const success_type& value() const& {
    if (const success_type* ptr = std::get_if<success_index>(&_value)) {
      return *ptr;
    }
    throw error();
  }

  [[nodiscard]] constexpr inline success_type& value() & {
    if (success_type* ptr = std::get_if<success_index>(&_value)) {
      return *ptr;
    }
    throw error();
  }

  [[nodiscard]] constexpr inline const success_type&& value() const&& {
    if (const success_type* ptr = std::get_if<success_index>(&_value)) {
      return std::move(*ptr);
    }
    throw error();
  }

  [[nodiscard]] constexpr inline success_type&& value() && {
    if (success_type* ptr = std::get_if<success_index>(&_value)) {
      return std::move(*ptr);
    }
    throw std::get<error_index>(std::move(_value));
  }

  template <class T = success_type,
            std::enable_if_t<std::is_convertible_v<T, success_type>>>
  [[nodiscard]] constexpr inline success_type value_or(
      T default_) const& noexcept {
    if (const success_type* ptr = std::get_if<success_index>(&_value)) {
      return *ptr;
    }
    return static_cast<success_type>(default_);
  }

  [[nodiscard]] constexpr error_type& error() & noexcept {
    return std::get<error_index>(_value);
  }

  [[nodiscard]] constexpr const error_type& error() const& noexcept {
    return std::get<error_index>(_value);
  }

  [[nodiscard]] constexpr error_type&& error() && noexcept {
    return std::get<error_index>(std::move(_value));
  }

  [[nodiscard]] constexpr const error_type&& error() const&& noexcept {
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