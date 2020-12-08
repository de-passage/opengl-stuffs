#ifndef GUARD_DPSG_INPUT_TIMER_HEADER
#define GUARD_DPSG_INPUT_TIMER_HEADER

#include <chrono>

template <class T> class input_timer {
public:
  using clock_type = std::chrono::steady_clock;
  using time_point = std::chrono::time_point<clock_type>;
  using duration = typename time_point::duration;

  template <class U = T, class Rep, class Dur,
            std::enable_if_t<std::is_convertible_v<U, T>, int> = 0>
  constexpr explicit input_timer(U &&callback,
                                 std::chrono::duration<Rep, Dur> duration)
      : _callback{std::forward<U>(callback)}, _duration{duration} {}

  void trigger() {
    auto now = clock_type::now();
    if (now - _last_trigger > _duration) {
      _last_trigger = now;
      _callback();
    }
  }

private:
  T _callback;
  duration _duration;
  time_point _last_trigger;
};
template <class T, class R, class D>
input_timer(T &&, std::chrono::duration<R, D>) -> input_timer<T>;

#endif // GUARD_DPSG_INPUT_TIMER_HEADER