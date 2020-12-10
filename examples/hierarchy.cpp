#define GLM_FORCE_SILENT_WARNINGS

#include "buffers.hpp"
#include "camera.hpp"
#include "common.hpp"
#include "fixed_size_element_buffer.hpp"
#include "glm_traits.hpp"
#include "input_timer.hpp"
#include "load_shaders.hpp"
#include "make_window.hpp"
#include "matrix_stack.hpp"
#include "meta/composite.hpp"
#include "opengl.hpp"
#include "opengl/glm.hpp"
#include "stbi_wrapper.hpp"
#include "structured_buffers.hpp"

#include <chrono>
#include <type_traits>

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

using dpsg::composite;
using dpsg::tag;

constexpr float finger_length = 2;
constexpr float finger_width = 0.25;
constexpr float angle_lower_finger = 45;
constexpr float angle_upper_finger = 180;
constexpr composite finger_segment{
    tag<struct finger_segment>,
    position{0, 0, finger_length / 2},
    scale{finger_width, finger_width, finger_length / 2},
    draw};

constexpr composite left_lower_finger{tag<struct left_lower_finger>,
                                      position{0, 0, finger_length},
                                      y_rotation{-angle_lower_finger},
                                      finger_segment};

constexpr composite right_lower_finger{tag<struct right_lower_finger>,
                                       position{0, 0, finger_length},
                                       y_rotation{angle_lower_finger},
                                       finger_segment};

constexpr composite left_finger{tag<struct left_finger>,
                                position{1, 0, 1},
                                y_rotation{angle_upper_finger},
                                finger_segment,
                                left_lower_finger};

constexpr composite right_finger{tag<struct right_finger>,
                                 position{-1, 0, 1},
                                 y_rotation{-angle_upper_finger},
                                 finger_segment,
                                 right_lower_finger};

constexpr float wrist_length = 1;
constexpr float wrist_width = 1;
constexpr float wrist_roll_angle = 0;
constexpr float wrist_pitch_angle = 67.5;
constexpr composite wrist_segment{tag<struct wrist_segment>,
                                  scale{wrist_width, wrist_width, wrist_length},
                                  draw};

constexpr composite wrist{tag<struct wrist>,
                          position{0, 0, 5},
                          z_rotation{wrist_roll_angle},
                          x_rotation{wrist_pitch_angle},
                          wrist_segment,
                          left_finger,
                          right_finger};

constexpr float lower_arm_angle = 146.25;
constexpr float lower_arm_length = 2.5;
constexpr float lower_arm_width = 0.75;
constexpr composite lower_arm_segment{
    tag<struct lower_arm_segment>,
    position{0, 0, lower_arm_length},
    scale{lower_arm_width, lower_arm_width, lower_arm_length},
    draw};

constexpr composite lower_arm{tag<struct lower_arm>,
                              position{0, 0, 8},
                              x_rotation{lower_arm_angle},
                              lower_arm_segment,
                              wrist};

constexpr float upper_arm_size = 4.5;
constexpr float upper_arm_angle = -33.75;
constexpr composite upper_arm_segment{tag<struct upper_arm_segment>,
                                      position{0, 0, upper_arm_size - 1},
                                      scale{1, 1, upper_arm_size},
                                      draw};

constexpr composite upper_arm{tag<struct upper_arm>,
                              x_rotation{upper_arm_angle},
                              upper_arm_segment,
                              lower_arm};

constexpr float base_scale = 3;
constexpr float base_angle = -45;
constexpr composite left_base{tag<struct left_base>,
                              position{2, 0, 0},
                              scale{1, 1, base_scale},
                              draw};
constexpr composite right_base{tag<struct right_base>,
                               position{-2, 0, 0},
                               scale{1, 1, base_scale},
                               draw};
constexpr composite crane{tag<struct crane>,
                          position{3, -5, -40},
                          y_rotation{base_angle},
                          left_base,
                          right_base,
                          upper_arm};

template <class T>
struct name;
template <class Tag, class... Cs>
struct name<composite<Tag, Cs...>> {
  constexpr static inline const char* value = name<Tag>::value;
};
#define GEN_SPECIALIZATION(name_)                        \
  template <>                                            \
  struct name<struct name_> {                            \
    constexpr static inline const char value[] = #name_; \
  };
#define TAGS(f)                                                         \
  f(finger_segment) f(crane) f(left_lower_finger) f(right_lower_finger) \
      f(left_finger) f(right_finger) f(upper_arm) f(upper_arm_segment)  \
          f(lower_arm_segment) f(lower_arm) f(left_base) f(wrist)       \
              f(wrist_segment) f(right_base)

TAGS(GEN_SPECIALIZATION)

constexpr void indent(int level) noexcept {
  for (int i = 0; i < level; ++i) {
    std::cout.put(' ');
  }
}

template <class H>
constexpr auto print(H& model) noexcept {
  return ignore([&model] {
    constexpr auto print_ = [](const auto& v, auto next, int indent_level = 0) {
      using value_type = std::decay_t<decltype(v)>;

      indent(indent_level);

      if constexpr (dpsg::is_composite_v<value_type>) {
        std::cout << name<value_type>::value << "{\n";
        next(indent_level + 2);
        indent(indent_level);
        std::cout << "},\n";
      }
      else if constexpr (std::is_same_v<value_type, position>) {
        std::cout << "position: (" << v.value[0] << "," << v.value[1] << ","
                  << v.value[2] << "),\n";
      }
      else if constexpr (std::is_base_of_v<rotation, value_type>) {
        std::cout << "rotation: (" << v.value[0] << "," << v.value[1] << ","
                  << v.value[2] << ", rad(" << v.angle.value << ")),\n";
      }
      else if constexpr (std::is_same_v<scale, value_type>) {
        std::cout << "scale: (" << v.value[0] << "," << v.value[1] << ","
                  << v.value[2] << "),\n";
      }
      else if constexpr (std::is_same_v<draw_t, value_type>) {
        std::cout << "draw required\n";
      }
      else {
        static_assert(std::is_same_v<value_type, void>, "Non exhaustive");
      }
    };

    dpsg::traverse(model, print_);
    std::cout.flush();
  });
}

constexpr auto gl_draw =
    [](auto& mstack, const auto& loc, const auto& elements) {
      return [&mstack, &loc, &elements](
                 const auto& v, auto next, glm::mat4& matrix) {
        using value_type = std::decay_t<decltype(v)>;

        if constexpr (dpsg::is_composite_v<value_type>) {
          mstack.push(next);
        }
        else if constexpr (std::is_base_of_v<rotation, value_type>) {
          matrix = glm::rotate(matrix, v.angle.value, v.value);
        }
        else if constexpr (std::is_same_v<position, value_type>) {
          matrix = glm::translate(matrix, v.value);
        }
        else if constexpr (std::is_same_v<scale, value_type>) {
          matrix = glm::scale(matrix, v.value);
        }
        else if constexpr (std::is_same_v<draw_t, value_type>) {
          loc.bind(matrix);
          elements.draw();
        }
        else {
          static_assert(std::is_same_v<value_type, void>, "Non exhaustive");
        }
      };
    };

constexpr float full_circle{glm::radians(360.F)};
constexpr float quarter_circle{glm::radians(90.F)};

template <class H>
constexpr auto rotate_base(H& model, float angle) noexcept {
  return ignore([&model, angle] {
    auto& v = dpsg::extract<y_rotation>(model).angle.value;
    v = std::fmodf(v + glm::radians(angle), full_circle);
  });
}

template <class H>
constexpr auto rotate_upper_arm(H& model, float angle) noexcept {
  return ignore([&model, angle] {
    auto& v = dpsg::extract(dpsg::path<struct upper_arm, x_rotation>, model)
                  .angle.value;
    v = std::clamp(v + glm::radians(angle), -quarter_circle, 0.F);
  });
}

template <class H>
constexpr auto rotate_lower_arm(H& model, float angle) noexcept {
  return ignore([&model, angle] {
    auto& v =
        dpsg::extract(
            dpsg::path<struct upper_arm, struct lower_arm, x_rotation>, model)
            .angle.value;
    v = std::clamp(v + glm::radians(angle), 0.F, glm::radians(lower_arm_angle));
  });
}

// clang-format off
const dpsg::gl::ushort_t index_data[] = {
    0,  1,  2,  // NOLINT
    2,  3,  0,  // NOLINT

    4,  5,  6,  // NOLINT
    6,  7,  4,  // NOLINT

    8,  9,  10,  // NOLINT
    10, 11, 8,   // NOLINT

    12, 13, 14,  // NOLINT
    14, 15, 12,  // NOLINT

    16, 17, 18,  // NOLINT
    18, 19, 16,  // NOLINT

    20, 21, 22,  // NOLINT
    22, 23, 20,  // NOLINT
};


#define RED_COLOR 1.0F, 0.0F, 0.0F, 1.0F
#define GREEN_COLOR 0.0F, 1.0F, 0.0F, 1.0F
#define BLUE_COLOR 	0.0F, 0.0F, 1.0F, 1.0F

#define YELLOW_COLOR 1.0F, 1.0F, 0.0F, 1.0F
#define CYAN_COLOR 0.0F, 1.0F, 1.0F, 1.0F
#define MAGENTA_COLOR 	1.0F, 0.0F, 1.0F, 1.0F

const dpsg::gl::float_t vertex_data[] =
{
	//Front
	+1.0F, +1.0F, +1.0F, // NOLINT
	+1.0F, -1.0F, +1.0F, // NOLINT
	-1.0F, -1.0F, +1.0F, // NOLINT
	-1.0F, +1.0F, +1.0F, // NOLINT

	//Top
	+1.0F, +1.0F, +1.0F, // NOLINT
	-1.0F, +1.0F, +1.0F, // NOLINT
	-1.0F, +1.0F, -1.0F, // NOLINT
	+1.0F, +1.0F, -1.0F, // NOLINT

	//LeFt
	+1.0F, +1.0F, +1.0F, // NOLINT
	+1.0F, +1.0F, -1.0F, // NOLINT
	+1.0F, -1.0F, -1.0F, // NOLINT
	+1.0F, -1.0F, +1.0F, // NOLINT

	//Back
	+1.0F, +1.0F, -1.0F, // NOLINT
	-1.0F, +1.0F, -1.0F, // NOLINT
	-1.0F, -1.0F, -1.0F, // NOLINT
	+1.0F, -1.0F, -1.0F, // NOLINT

	//Bottom
	+1.0F, -1.0F, +1.0F, // NOLINT
	+1.0F, -1.0F, -1.0F, // NOLINT
	-1.0F, -1.0F, -1.0F, // NOLINT
	-1.0F, -1.0F, +1.0F, // NOLINT

	//Right
	-1.0F, +1.0F, +1.0F, // NOLINT
	-1.0F, -1.0F, +1.0F, // NOLINT
	-1.0F, -1.0F, -1.0F, // NOLINT
	-1.0F, +1.0F, -1.0F, // NOLINT


	GREEN_COLOR,
	GREEN_COLOR,
	GREEN_COLOR,
	GREEN_COLOR,

	BLUE_COLOR,
	BLUE_COLOR,
	BLUE_COLOR,
	BLUE_COLOR,

	RED_COLOR,
	RED_COLOR,
	RED_COLOR,
	RED_COLOR,

	YELLOW_COLOR,
	YELLOW_COLOR,
	YELLOW_COLOR,
	YELLOW_COLOR,

	CYAN_COLOR,
	CYAN_COLOR,
	CYAN_COLOR,
	CYAN_COLOR,

	MAGENTA_COLOR,
	MAGENTA_COLOR,
	MAGENTA_COLOR,
	MAGENTA_COLOR,
};
// clang-format on

void hierarchy(dpsg::window& wdw, key_mapper& kmap) {
  using namespace dpsg;
  using namespace dpsg::input;

  gl::enable(gl::capability::cull_face);
  gl::cull_face(gl::cull_mode::back);
  gl::front_face(gl::face_mode::clockwise);

  gl::enable(gl::capability::depth_test);
  gl::depth_mask(true);
  gl::depth_func(gl::compare_function::lequal);
  gl::depth_range(gl::near{0}, gl::far{1});

  matrix_stack<traits::glm> stack;
  camera<traits::glm> camera(SCR_WIDTH / SCR_HEIGHT);
  auto prog = load(vs_filename{"shaders/projected_with_colors.vs"},
                   fs_filename{"shaders/basic.fs"});
  prog.use();
  auto model_u = prog.uniform_location<glm::mat4>("model").value();
  auto projection_u =
      prog.uniform_location<glm::mat4>("projected_view").value();
  projection_u.bind(camera.projected_view());

  using layout = sequenced<group<3>, group<4>>;
  fixed_size_structured_buffer vertex_array{layout{}, vertex_data};
  vertex_array.enable();
  fixed_size_element_buffer element_buffer{index_data};

  auto model{crane};

  kmap.on(key::enter, print(model));
  constexpr float standard_angle_increment = 11.25;
  constexpr float small_angle_increment = 9;
  kmap.while_(key::M, rotate_base(model, standard_angle_increment));
  kmap.while_(key::N, rotate_base(model, -standard_angle_increment));
  kmap.while_(key::J, rotate_upper_arm(model, -standard_angle_increment));
  kmap.while_(key::H, rotate_upper_arm(model, +standard_angle_increment));
  kmap.while_(key::Y, rotate_lower_arm(model, standard_angle_increment));
  kmap.while_(key::U, rotate_lower_arm(model, -standard_angle_increment));

  using namespace std::literals::chrono_literals;
  constexpr auto interval = 30ms;
  input_timer timer{[&kmap, &wdw] { kmap.trigger_pressed_callbacks(wdw); },
                    interval};

  wdw.render_loop([&] {
    gl::clear(gl::buffer_bit::color | gl::buffer_bit::depth);
    traverse(model, gl_draw(stack, model_u, element_buffer), stack.top());
    timer.trigger();
  });
}

int main() {
  return windowed(hierarchy);
}