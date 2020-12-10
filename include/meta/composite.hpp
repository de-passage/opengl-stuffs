#ifndef GUARD_DPSG_META_COMPOSITE_HEADER
#define GUARD_DPSG_META_COMPOSITE_HEADER

#include <tuple>
#include <utility>

namespace dpsg {

template <class T>
struct tag_t {};

template <class T>
constexpr static inline tag_t<T> tag{};

template <class Tag, class... Components>
struct composite {
  using tag = Tag;

  template <class... Args>
  constexpr explicit composite([[maybe_unused]] tag_t<tag> tag_, Args&&... args)
      : components(std::forward<Args>(args)...) {}

  std::tuple<Components...> components;
};

template <class Tag, class... Components>
composite(tag_t<Tag>, Components...) -> composite<Tag, Components...>;

template <class H>
struct is_composite : std::false_type {};
template <class T, class... Cs>
struct is_composite<composite<T, Cs...>> : std::true_type {};

template <class H>
constexpr static inline bool is_composite_v = is_composite<H>::value;

struct traverse_t {
  template <class H,
            class F,
            std::enable_if_t<is_composite_v<std::decay_t<H>>, int> = 0,
            class... Args>
  constexpr void operator()(H&& h, F f, Args&&... args) const {
    f(h, next(h, f), std::forward<Args>(args)...);
  }

  template <class T,
            class F,
            std::enable_if_t<!is_composite_v<std::decay_t<T>>, int> = 0,
            class... Args>
  constexpr void operator()(T&& t, F&& f, Args&&... args) const {
    std::forward<F>(f)(
        std::forward<T>(t), [] {}, std::forward<Args>(args)...);
  }

 private:
  template <class Tag, class F, class... Cs>
  constexpr static auto next(const composite<Tag, Cs...>& h, F f) {
    return [&h, f](auto&&... args) {
      (traverse_t{}(std::get<Cs>(h.components),
                    f,
                    std::forward<decltype(args)>(args)...),
       ...);
    };
  }

  template <class Tag, class F, class... Cs>
  constexpr static auto next(composite<Tag, Cs...>& h, F f) {
    return [&h, f](auto&&... args) mutable {
      (traverse_t{}(std::get<Cs>(h.components),
                    f,
                    std::forward<decltype(args)>(args)...),
       ...);
    };
  }

} constexpr traverse;

namespace detail {

template <class R, class... Args>
struct find_similar;

template <class R, class... Args>
struct find_similar<R, R, Args...> {
  using type = R;
};

template <class R, class S, class... Args>
struct find_similar<R, S, Args...> {
  using type = typename find_similar<R, Args...>::type;
};

template <class R, class... Cs, class... Args>
struct find_similar<R, composite<R, Cs...>, Args...> {
  using type = composite<R, Cs...>;
};

template <class R, class... Args>
using find_similar_t = typename find_similar<R, Args...>::type;

template <class... Args>
struct tuple {};

template <class H>
struct components;

template <class T, class... Args>
struct components<composite<T, Args...>> {
  using type = tuple<Args...>;
};

template <class H>
using components_t = typename components<H>::type;

template <class R, class T, class... Args>
constexpr decltype(auto) extract([[maybe_unused]] tuple<Args...> tag,
                                 T&& tupl) noexcept {
  return std::get<find_similar_t<R, Args...>>(std::forward<T>(tupl));
}
}  // namespace detail

template <class... Args>
struct path_t {
  template <class... Ts>
  constexpr static inline path_t<Args..., Ts...> then{};
};

template <class T, class... Args>
struct path_t<T, Args...> {
  using head = T;
  using tail = path_t<Args...>;
  template <class... Ts>
  constexpr static inline path_t<T, Args..., Ts...> then{};
};

template <class... Args>
constexpr static inline path_t<Args...> path{};

template <class H, class... P>
constexpr decltype(auto) extract([[maybe_unused]] path_t<P...> unused,
                                 H&& composite) noexcept {
  using head = typename path_t<P...>::head;
  using tail = typename path_t<P...>::tail;
  constexpr auto S = sizeof...(P);
  static_assert(S >= 1, "Invalid empty path");
  if constexpr (sizeof...(P) > 1) {
    return extract(
        tail{},
        detail::extract<head>(detail::components_t<std::decay_t<H>>{},
                              std::forward<H>(composite).components));
  }
  else {
    return detail::extract<head>(detail::components_t<std::decay_t<H>>{},
                                 std::forward<H>(composite).components);
  }
}

template <class R, class H>
constexpr decltype(auto) extract(H&& hierarchy) noexcept {
  return extract(path<R>, std::forward<H>(hierarchy));
}
}  // namespace dpsg

#endif  // GUARD_DPSG_META_COMPOSITE_HEADER