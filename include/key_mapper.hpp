#ifndef GUARD_KEY_MAPPER_HEADER
#define GUARD_KEY_MAPPER_HEADER

#include "with_window.hpp"

#include <functional>
#include <tuple>
#include <unordered_map>

class key_mapper {
 public:
  using key_pressed_callback = std::function<void(dpsg::window&)>;

 private:
  using tuple_type =
      std::tuple<bool, key_pressed_callback, key_pressed_callback>;
  using map_type = std::unordered_map<dpsg::input::key, tuple_type>;
  map_type _map;

  static inline bool _is_pressed(const tuple_type& c) { return std::get<0>(c); }
  static inline bool& _is_pressed(tuple_type& c) { return std::get<0>(c); }
  static inline key_pressed_callback& _pressed_callback(tuple_type& c) {
    return std::get<1>(c);
  }
  static inline key_pressed_callback& _maintained_callback(tuple_type& c) {
    return std::get<2>(c);
  }
  template <class... Args>
  static inline void _trigger_maintained(const tuple_type& c, Args&&... args) {
    const auto& f = std::get<2>(c);
    if (f) {
      f(std::forward<Args>(args)...);
    }
  }
  template <class... Args>
  static inline void _trigger_pressed(const tuple_type& c, Args&&... args) {
    const auto& f = std::get<1>(c);
    if (f) {
      f(std::forward<Args>(args)...);
    }
  }

 public:
  void operator()(dpsg::window& w,
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

  inline void trigger_pressed_callbacks(dpsg::window& window) const {
    for (auto tuple : _map) {
      if (_is_pressed(tuple.second)) {
        _trigger_maintained(tuple.second, window);
      }
    }
  }
};

#endif  // GUARD_KEY_MAPPER_HEADER