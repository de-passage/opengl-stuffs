#ifndef GUARD_DPSG_META_MIXIN_HEADER
#define GUARD_DPSG_META_MIXIN_HEADER

#include <type_traits>
namespace dpsg {

// template<...> struct mixin;
//
// The mixin<...> struct is the whole framework.
// It expects as template parameter a 'base' type, from a CRTP standpoint,
// followed by a list of types containing a nested template named 'type' that
// we'll call mixins. The intended usage is as the base of a user-defined type.
// e. g.
//
//    class my_mixin_enabled_class :
//        public mixin<my_mixin_enabled_class, mixin_a, mixin_b> {
//    };
//
// where mixin_a and mixin_b are of the form
//
//    struct mixin_a {
//      template<typename Base>
//      struct type : Base {
//        <mixin implementation here>
//      };
//    };
//
// The effect is to create a type that inherits from both mixin_a::type and
// mixin_b::type. Compared to regular use of inheritance, this has several
// advantages:
// + linear hierarchy, so no risk of a diamond inheritance
// + flexible
// + safe downcast from mixin classes thanks to CRTP (see real_type below)
// + consequently a lot of patterns normally implemented with virtual functions
//   can be made overhead-free
// + and one can access template functions of derived classes (ever thought
// about
//   all that you could do with virtual template methods? Now you can)
// Of course since the whole thing relies on a whole bunch of template
// instanciations and type deductions, expect an increase of compilation times,
// and polymorphic functions are a bit more complicated than with a regular
// inheritance hierarchy (see contains_mixin below)
//
//
// template<typename> using real_type_t;
//
// real_type_t<T> is a utility meta-function intended to extract the base type
// from within a mixin. The idea is that since all the types are known at
// compile time, we can safely downcast from any mixin to the class implementing
// said mixin. One could also use that type information to access compile time
// properties of the implementer, etc. Example:
//
//    struct my_mixin {
//      template<typename Base>
//      struct type : Base {
//        decltype(auto) pointless() const {
//          return static_cast<real_type_t<Base>*>>(this)->some_member();
//        }
//      };
//    };
//
//
// template<typename T> using extract_mixin_parameters_t;
//
// This is mostly for internal use, it returns typename T::mixin_t. This type is
// defined by the mixin framework, but the implementor is required to expose it
// (by inheriting publicly from mixin<...> or the appropriate using declaration)
// for this function to work. contains_mixin depends on this function, so leave
// mixin_t exposed!
//
//
// template<class Implementor, class Mixin> bool contains_mixin_v;
//
// Is true if the given Implementor type contains a mixin list including Mixin.
// This is meant as a utility to select implementations at compile time.
//
//    template<class T>
//    void my_constexpr_func(T my_val) {
//      if constexpr (contains_mixin_v<T, my_mixin>) {
//        <implementation specific to my_mixin>
//      }
//      else {
//        <general case>
//      }
//    }
//
//    template<class T,
//             std::enable_if_t<contains_mixin_v<T, my_mixin>, int> = 0>
//    void my_sfinae_function(T my_val) {
//      <this overload is only selected if T implements my_mixin>
//    }
template <class... Ts>
struct mixin;

template <class... Args>
struct tuple {
  template <template <class...> class T>
  using apply = T<Args...>;
};

template <class T>
struct mixin<T> {
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
template <class T, class... Ts>
struct real_type<mixin<T, Ts...>> {
  using type = T;
};

template <class T>
using real_type_t = typename real_type<T>::type;

template <class T>
struct extract_mixin_parameters {
  using type = typename T::mixin_t;

  template <template <class...> class U>
  using apply = typename type::template apply<U>;
};

template <class T, class... Args>
struct extract_mixin_parameters<mixin<T, Args...>> {
  using type = tuple<Args...>;

  template <template <class...> class U>
  using apply = typename type::template apply<U>;
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

template <class Mixin, class... Args>
struct append;
template <template <class...> class Mixin, class... Args1, class... Args2>
struct append<Mixin<Args1...>, Args2...> {
  using type = Mixin<Args1..., Args2...>;
};
template <class Mixin, class... Args>
using append_t = typename append<Mixin, Args...>::type;

template <class Mixin, class... Args>
struct prepend;
template <template <class...> class Mixin, class... Args1, class... Args2>
struct prepend<Mixin<Args1...>, Args2...> {
  using type = Mixin<Args2..., Args1...>;
};
template <class Mixin, class... Args>
using prepend_t = typename prepend<Mixin, Args...>::type;

}  // namespace dpsg

#endif  // GUARD_DPSG_META_MIXIN_HEADER