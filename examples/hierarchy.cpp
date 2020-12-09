#include "common.hpp"
#include "glm/ext/vector_float3.hpp"
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

  template <class F> void push(F &&f) {
    auto &top = push();
    std::forward<F>(f)(top);
    pop();
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
      : value{x, y, z}, angle{dpsg::to_radians(dpsg::degrees{a})} {}
  glm::vec3 value;
  dpsg::radians angle;
};

struct x_rotation : rotation {
  constexpr explicit x_rotation(float angle) : rotation(1, 0, 0, angle) {}
};

struct y_rotation : rotation {
  constexpr explicit y_rotation(float angle) : rotation(0, 1, 0, angle) {}
};

struct z_rotation : rotation {
  constexpr explicit z_rotation(float angle) : rotation(0, 0, 1, angle) {}
};

struct position {
  constexpr explicit position(float x, float y, float z) : value{x, y, z} {}
  glm::vec3 value;
};

struct draw_t {
} constexpr draw;

template <class T> struct tag_t {};
template <class T> constexpr tag_t<T> tag{};
template <class Tag, class... Components> struct hierarchy_t {
  template <class... Args>
  constexpr explicit hierarchy_t([[maybe_unused]] tag_t<Tag> tag,
                                 Args &&... args)
      : components(std::forward<Args>(args)...) {}

  std::tuple<Components...> components;
};

template <class Tag, class... Components>
hierarchy_t(tag_t<Tag>, Components...) -> hierarchy_t<Tag, Components...>;

constexpr float finger_length = 2;
constexpr float finger_width = 0.5;
constexpr float angle_lower_finger = 45;
constexpr float angle_upper_finger = 180;
constexpr hierarchy_t finger_segment{
    tag<struct finger_segment>, position{0, 0, finger_length / 2},
    scale{finger_width / 2, finger_width / 2, finger_length / 2}, draw};

constexpr hierarchy_t left_lower_finger{
    tag<struct left_lower_finger>, position{0, 0, finger_length},
    y_rotation{-angle_lower_finger}, finger_segment};

constexpr hierarchy_t right_lower_finger{
    tag<struct right_lower_finger>, position{0, 0, finger_length},
    y_rotation{angle_lower_finger}, finger_segment};

constexpr hierarchy_t left_finger{tag<struct left_finger>, position{1, 0, 1},
                                  y_rotation{angle_upper_finger},
                                  finger_segment, left_lower_finger};

constexpr hierarchy_t right_finger{tag<struct right_finger>, position{-1, 0, 1},
                                   y_rotation{-angle_upper_finger},
                                   finger_segment, right_lower_finger};

constexpr float wrist_length = 1;
constexpr float wrist_width = 1;
constexpr float wrist_roll_angle = 0;
constexpr float wrist_pitch_angle = 67.5;
constexpr hierarchy_t wrist_segment{
    tag<struct wrist_segment>, scale{wrist_width, wrist_width, wrist_length},
    draw};

constexpr hierarchy_t wrist{tag<struct wrist>,
                            position{0, 0, 0.5},
                            z_rotation{wrist_roll_angle},
                            x_rotation{wrist_pitch_angle},
                            wrist_segment,
                            left_finger,
                            right_finger};

constexpr float lower_arm_angle(146.25);
constexpr float lower_arm_length(2.5);
constexpr float lower_arm_width(1.5);
constexpr hierarchy_t lower_arm_segment{
    tag<struct lower_arm_segment>, position{0, 0, lower_arm_length},
    scale{lower_arm_width, lower_arm_width, lower_arm_length}, draw};

constexpr hierarchy_t lower_arm{tag<struct lower_arm>, position{0, 0, 8},
                                x_rotation{lower_arm_angle}, lower_arm_segment,
                                wrist};

constexpr float upper_arm_size = 4.5;
constexpr float upper_arm_angle = -33.75;
constexpr hierarchy_t upper_arm_segment{tag<struct upper_arm_segment>,
                                        position{0, 0, upper_arm_size - 1},
                                        scale{1, 1, upper_arm_size}, draw};

constexpr hierarchy_t upper_arm{tag<struct upper_arm>,
                                x_rotation{upper_arm_angle}, upper_arm_segment,
                                lower_arm};

constexpr float base_scale = 3;
constexpr float base_angle = -45;
constexpr hierarchy_t left_base{tag<struct left_base>, position{2, 0, 0},
                                scale{1, 1, base_scale}, draw};
constexpr hierarchy_t right_base{tag<struct right_base>, position{-2, 0, 0},
                                 scale{1, 1, base_scale}, draw};
constexpr hierarchy_t crane{
    tag<struct crane>, position{3, -5, -40}, y_rotation{base_angle},
    left_base,         right_base,           upper_arm};

template <class T, class H> struct contains;
template <class T, class... Components>
struct contains<T, hierarchy_t<Components...>>
    : std::disjunction<std::is_same<T, Components>...> {};
template <class T, class H>
constexpr static inline bool contains_v = contains<T, H>::value;

void hierarchy(dpsg::window &wdw) {
  using namespace dpsg;

  matrix_stack<traits::glm> stack;

  gl::enable(gl::capability::depth_test);

  wdw.render_loop(
      [&] { gl::clear(gl::buffer_bit::color | gl::buffer_bit::depth); });
}

int main() { return windowed(hierarchy); }