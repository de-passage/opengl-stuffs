#include "glm_traits.hpp"
#include "make_window.hpp"
#include "opengl.hpp"

#include <type_traits>
#include <vector>

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
  explicit matrix_stack(M &&mat) : _stack{1, std::forward<M>(mat)} {}

  [[nodiscard]] const mat_type &top() const { return _stack.back(); }

  [[nodiscard]] mat_type &top() { return _stack.back(); }

  [[nodiscard]] std::size_t size() const { return _stack.size() - 1; }

  mat_type &push() {
    _stack.push_back(top());
    return top();
  }

  void pop() {
    assert(size() > 1);
    _stack.pop_back();
  }

private:
  stack_type _stack{1, traits::identity_matrix};
};

struct scale {
  constexpr explicit scale(float x, float y, float z) : value{x, y, z} {}
  glm::vec3 value;
};

struct rotation {
  constexpr explicit rotation(float x, float y, float z, float a)
      : value{x, y, z}, angle{a} {}
  glm::vec3 value;
  dpsg::radians angle;
};

struct position {
  constexpr explicit position(float x, float y, float z) : value{x, y, z} {}
  glm::vec3 value;
};

template <class... Components> struct hierarchy_t : Components... {
  template <class... Args>
  constexpr explicit hierarchy_t(Args &&... args)
      : Components(std::forward<Args>(args))... {}
};

template <class T, class H> struct contains;
template <class T, class... Components>
struct contains<T, hierarchy_t<Components...>>
    : std::disjunction<std::is_same<T, Components>...> {};
template <class T, class H>
constexpr static inline bool contains_v = contains<T, H>::value;

void hierarchy(dpsg::window &wdw) {
  using namespace dpsg;

  matrix_stack<glm_traits> stack;

  gl::enable(gl::capability::depth_test);

  wdw.render_loop(
      [&] { gl::clear(gl::buffer_bit::color | gl::buffer_bit::depth); });
}

int main() { return windowed(hierarchy); }