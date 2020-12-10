#ifndef GUARD_DPSG_MATRIX_STACK
#define GUARD_DPSG_MATRIX_STACK

#include <type_traits>
#include <vector>

namespace dpsg {

template <class Traits,
          class Container = std::vector<typename Traits::mat_type>>
class matrix_stack {
  using traits = Traits;

 public:
  using mat_type = typename traits::mat_type;
  using stack_type = Container;

  matrix_stack() = default;

  template <class M = mat_type,
            std::enable_if_t<std::is_convertible_v<M, mat_type>, int> = 0>
  explicit matrix_stack(M&& mat) : _stack{1, std::forward<M>(mat)} {}

  [[nodiscard]] const mat_type& top() const { return _stack.back(); }

  [[nodiscard]] mat_type& top() { return _stack.back(); }

  [[nodiscard]] std::size_t size() const { return _stack.size() - 1; }

  mat_type& push() {
    _stack.push_back(top());
    return top();
  }

  void pop() {
    assert(size() > 0);
    _stack.pop_back();
  }

  template <class F>
  void push(F&& f) {
    auto& top = push();
    std::forward<F>(f)(top);
    pop();
  }

 private:
  stack_type _stack{1, traits::identity_matrix};
};

}  // namespace dpsg

#endif