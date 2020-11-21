#ifndef GUARD_KEY_MAPPER_HEADER
#define GUARD_KEY_MAPPER_HEADER

#include "window.hpp"

#include <functional>
#include <unordered_map>

struct key_mapper {
  using callback = std::function<void(dpsg::window &)>;

  void operator()(dpsg::window &w, dpsg::input::key k,
                  [[maybe_unused]] int scancode, dpsg::input::status st,
                  [[maybe_unused]] int mode) {
    if (auto it = _map.find(k); it != _map.end()) {
      if (st == dpsg::input::status::pressed) {
        it->second.first = true;
      } else if (st == dpsg::input::status::released && it->second.first) {
        it->second.first = false;
        it->second.second(w);
      }
    }
  }

  void on(dpsg::input::key k, callback cb) { _map[k].second = std::move(cb); }

private:
  std::unordered_map<dpsg::input::key, std::pair<bool, callback>> _map;
};

#endif // GUARD_KEY_MAPPER_HEADER