#include "input_timer.hpp"
#include "key_mapper.hpp"
#include "layout.hpp"
#include "load_shaders.hpp"
#include "make_window.hpp"
#include "shaders.hpp"
#include "structured_buffers.hpp"

#include "opengl.hpp"

void cube(dpsg::window& window, key_mapper& kmap) {
  using namespace dpsg;
  using namespace dpsg::input;

  // clang-format off
  // NOLINTNEXTLINE
  constexpr float vertex_data[] = {
      0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT
      0.25f, -0.25f, -1.25f, 1.0f, // NOLINT
      -0.25f, 0.25f, -1.25f, 1.0f, // NOLINT

      0.25f, -0.25f, -1.25f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -1.25f, 1.0f, // NOLINT
      -0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT

      0.25f, 0.25f, -2.75f, 1.0f,  // NOLINT
      -0.25f, 0.25f, -2.75f, 1.0f, // NOLINT
      0.25f, -0.25f, -2.75f, 1.0f, // NOLINT

      0.25f, -0.25f, -2.75f, 1.0f,  // NOLINT
      -0.25f, 0.25f, -2.75f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -2.75f, 1.0f, // NOLINT

      -0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -1.25f, 1.0f, // NOLINT
      -0.25f, -0.25f, -2.75f, 1.0f, // NOLINT

      -0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -2.75f, 1.0f, // NOLINT
      -0.25f, 0.25f, -2.75f, 1.0f,  // NOLINT

      0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT
      0.25f, -0.25f, -2.75f, 1.0f, // NOLINT
      0.25f, -0.25f, -1.25f, 1.0f, // NOLINT

      0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT
      0.25f, 0.25f, -2.75f, 1.0f,  // NOLINT
      0.25f, -0.25f, -2.75f, 1.0f, // NOLINT

      0.25f, 0.25f, -2.75f, 1.0f,  // NOLINT
      0.25f, 0.25f, -1.25f, 1.0f,  // NOLINT
      -0.25f, 0.25f, -1.25f, 1.0f, // NOLINT

      0.25f, 0.25f, -2.75f, 1.0f,  // NOLINT
      -0.25f, 0.25f, -1.25f, 1.0f, // NOLINT
      -0.25f, 0.25f, -2.75f, 1.0f, // NOLINT

      0.25f, -0.25f, -2.75f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -1.25f, 1.0f, // NOLINT
      0.25f, -0.25f, -1.25f, 1.0f,  // NOLINT

      0.25f, -0.25f, -2.75f, 1.0f,  // NOLINT
      -0.25f, -0.25f, -2.75f, 1.0f, // NOLINT
      -0.25f, -0.25f, -1.25f, 1.0f, // NOLINT

      // Colors
      0.0f, 0.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 0.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 0.0f, 1.0f, 1.0f, // NOLINT

      0.0f, 0.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 0.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 0.0f, 1.0f, 1.0f, // NOLINT

      0.8f, 0.8f, 0.8f, 1.0f, // NOLINT
      0.8f, 0.8f, 0.8f, 1.0f, // NOLINT
      0.8f, 0.8f, 0.8f, 1.0f, // NOLINT

      0.8f, 0.8f, 0.8f, 1.0f, // NOLINT
      0.8f, 0.8f, 0.8f, 1.0f, // NOLINT
      0.8f, 0.8f, 0.8f, 1.0f, // NOLINT

      0.0f, 1.0f, 0.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 0.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 0.0f, 1.0f, // NOLINT

      0.0f, 1.0f, 0.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 0.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 0.0f, 1.0f, // NOLINT

      0.5f, 0.5f, 0.0f, 1.0f, // NOLINT
      0.5f, 0.5f, 0.0f, 1.0f, // NOLINT
      0.5f, 0.5f, 0.0f, 1.0f, // NOLINT

      0.5f, 0.5f, 0.0f, 1.0f, // NOLINT
      0.5f, 0.5f, 0.0f, 1.0f, // NOLINT
      0.5f, 0.5f, 0.0f, 1.0f, // NOLINT

      1.0f, 0.0f, 0.0f, 1.0f, // NOLINT
      1.0f, 0.0f, 0.0f, 1.0f, // NOLINT
      1.0f, 0.0f, 0.0f, 1.0f, // NOLINT

      1.0f, 0.0f, 0.0f, 1.0f, // NOLINT
      1.0f, 0.0f, 0.0f, 1.0f, // NOLINT
      1.0f, 0.0f, 0.0f, 1.0f, // NOLINT

      0.0f, 1.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 1.0f, 1.0f, // NOLINT

      0.0f, 1.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 1.0f, 1.0f, // NOLINT
      0.0f, 1.0f, 1.0f, 1.0f, // NOLINT
  };
  // clang-format on

  auto prog = load(vs_filename{"shaders/ortho_with_offset.vs"},
                   fs_filename("shaders/basic.fs"))
                  .value();

  gl::enable(gl::capability::cull_face);
  gl::cull_face(gl::cull_mode::back);
  gl::front_face(gl::face_mode::clockwise);

  gl::enable(gl::capability::depth_test);

  using seq_layout = sequenced<group<4>, group<4>>;
  fixed_size_structured_buffer buffer(seq_layout{}, vertex_data);
  buffer.enable();

  prog.use();
  auto offset_u = prog.uniform_location<gl::vec_t<2, float>>("offset").value();
  auto z_near_u = prog.uniform_location<float>("z_near").value();
  auto z_far_u = prog.uniform_location<float>("z_far").value();
  auto frustum_scale_u = prog.uniform_location<float>("frustum_scale").value();

  constexpr float default_x{0};
  constexpr float default_y{0};
  constexpr float default_z_near{1};
  constexpr float default_z_far{3};
  constexpr float default_fscale{1};
  float x_offset{default_x};
  float y_offset{default_y};
  float z_near{default_z_near};
  float z_far{default_z_far};
  float frustum_scale{default_fscale};

  const auto update = [](float& u, auto&& f) {
    return [&u, f = std::forward<decltype(f)>(f)](auto p) {
      return ignore([&u, f = std::move(f), p] {
        u += p;
        f();
      });
    };
  };

  const auto update_zn = update(z_near, [&] { z_near_u.bind(z_near); });

  const auto update_zf = update(z_far, [&] { z_far_u.bind(z_far); });

  const auto update_fs =
      update(frustum_scale, [&] { frustum_scale_u.bind(frustum_scale); });

  const auto update_x = update(x_offset, [] {});
  const auto update_y = update(y_offset, [] {});
  const auto reset = ignore([&] {
    frustum_scale = default_fscale;
    z_near = default_z_near;
    z_far = default_z_far;
    x_offset = default_x;
    y_offset = default_y;

    frustum_scale_u.bind(frustum_scale);
    z_near_u.bind(z_near);
    z_far_u.bind(z_far);
  });

  z_near_u.bind(z_near);
  z_far_u.bind(z_far);
  frustum_scale_u.bind(frustum_scale);

  using namespace std::literals::chrono_literals;
  constexpr float off = 0.01;
  constexpr auto interval = 10ms;
  kmap.while_(key::down, update_y(-off));
  kmap.while_(key::up, update_y(+off));
  kmap.while_(key::right, update_x(+off));
  kmap.while_(key::left, update_x(-off));

  kmap.on(key::R, reset);
  kmap.while_(key::Q, update_zn(+off));
  kmap.while_(key::A, update_zn(-off));
  kmap.while_(key::W, update_zf(+off));
  kmap.while_(key::S, update_zf(-off));
  kmap.while_(key::E, update_fs(+off));
  kmap.while_(key::D, update_fs(-off));

  input_timer timer{[&] { kmap.trigger_pressed_callbacks(window); }, interval};

  const auto bind_offset = [&](float x, float y) {
    offset_u.bind(x + x_offset, y + y_offset);
  };

  constexpr float pos = 0.5;
  window.render_loop([&] {
    gl::clear(gl::buffer_bit::color | gl::buffer_bit::depth);

    bind_offset(pos, pos);
    buffer.draw();
    bind_offset(-pos, pos);
    buffer.draw();
    bind_offset(pos, -pos);
    buffer.draw();
    bind_offset(-pos, -pos);
    buffer.draw();

    timer.trigger();
  });
}

int main() {
  return windowed(cube);
}