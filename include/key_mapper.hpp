#ifndef GUARD_KEY_MAPPER_HEADER
#define GUARD_KEY_MAPPER_HEADER

#include "with_window.hpp"

#include <functional>
#include <tuple>
#include <type_traits>
#include <unordered_map>

template <class W>
class basic_key_mapper {
 public:
  using window_type = W;
  using key_pressed_callback = std::function<void(window_type&)>;

 private:
  using tuple_type =
      std::tuple<bool, key_pressed_callback, key_pressed_callback>;
  constexpr static inline std::size_t _is_pressed_idx{0};
  constexpr static inline std::size_t _keypress_cb_idx{1};
  constexpr static inline std::size_t _maintain_cb_idx{2};
  using map_type = std::unordered_map<dpsg::input::key, tuple_type>;
  map_type _map;

  static inline bool _is_pressed(const tuple_type& c) {
    return std::get<_is_pressed_idx>(c);
  }
  static inline bool& _is_pressed(tuple_type& c) {
    return std::get<_is_pressed_idx>(c);
  }
  static inline key_pressed_callback& _pressed_callback(tuple_type& c) {
    return std::get<_keypress_cb_idx>(c);
  }
  static inline key_pressed_callback& _maintained_callback(tuple_type& c) {
    return std::get<_maintain_cb_idx>(c);
  }
  template <class... Args>
  static inline void _trigger_maintained(const tuple_type& c, Args&&... args) {
    const auto& f = std::get<_maintain_cb_idx>(c);
    if (f) {
      f(std::forward<Args>(args)...);
    }
  }
  template <class... Args>
  static inline void _trigger_pressed(const tuple_type& c, Args&&... args) {
    const auto& f = std::get<_keypress_cb_idx>(c);
    if (f) {
      f(std::forward<Args>(args)...);
    }
  }

 public:
  void operator()(window_type& w,
                  dpsg::input::key k,
                  [[maybe_unused]] int scancode,
                  dpsg::input::status st,
                  [[maybe_unused]] int mode) {
    if (auto it = _map.find(k); it != _map.end()) {
      if (st == dpsg::input::status::pressed) {
        _is_pressed(it->second) = true;
      }
      else if (st == dpsg::input::status::released && _is_pressed(it->second)) {
        _is_pressed(it->second) = false;
        _trigger_pressed(it->second, w);
      }
    }
  }

  inline void on(dpsg::input::key k, key_pressed_callback cb) {
    _pressed_callback(_map[k]) = std::move(cb);
  }

  inline void while_(dpsg::input::key k, key_pressed_callback cb) {
    _maintained_callback(_map[k]) = std::move(cb);
  }

  inline void trigger_pressed_callbacks(window_type& window) const {
    for (auto tuple : _map) {
      if (_is_pressed(tuple.second)) {
        _trigger_maintained(tuple.second, window);
      }
    }
  }
};

using key_mapper = basic_key_mapper<dpsg::window>;

#endif  // GUARD_KEY_MAPPER_HEADER