
#include "nuklear/buffer.hpp"
#include "nuklear/nuklear++.hpp"

#include "buffers.hpp"
#include "glfw_context.hpp"
#include "layout.hpp"
#include "opengl.hpp"
#include "program.hpp"
#include "shaders.hpp"
#include "window.hpp"

#include <cstddef>
#include <iterator>
#include <utility>

namespace nk {
class user_font {
 public:
  using type = nk_user_font;
  using reference = type&;
  using const_reference = const type&;

  [[nodiscard]] constexpr const_reference font() const noexcept {
    return _font;
  }

  [[nodiscard]] constexpr reference font() noexcept { return _font; }

 private:
  struct nk_user_font _font;
};

}  // namespace nk

namespace dpsg {
class nk_gl3_backend {
 public:
  nk_gl3_backend()
      : _prog{_generate().value()},
        _texture_unif{gl::get_uniform_location(_prog.id(), "Texture")},
        _projection_unif(gl::get_uniform_location(_prog.id(), "ProjMtx")) {}
  nk_gl3_backend(const nk_gl3_backend&) = delete;
  nk_gl3_backend(nk_gl3_backend&&) noexcept = default;
  nk_gl3_backend& operator=(const nk_gl3_backend&) = delete;
  nk_gl3_backend& operator=(nk_gl3_backend&&) noexcept = default;

  ~nk_gl3_backend() noexcept { gl::delete_texture(_texture); }

  template <class T>
  inline void upload_atlas(const T* image, gl::width w, gl::height h) {
    gl::gen_texture(_texture);
    gl::bind_texture(gl::texture_target::_2d, _texture);
    gl::tex_parameter(gl::texture_target::_2d, gl::min_filter::linear);
    gl::tex_parameter(gl::texture_target::_2d, gl::mag_filter::linear);
    gl::tex_image_2D(
        gl::texture_image_target::_2d, w, h, gl::image_format::rgba, image);
  }

  void load_font(nk_context* ctx,
                 const char* filepath,
                 float height,
                 const struct nk_font_config* cfg = nullptr) {
    nk_font_atlas atlas{};
    nk_font_atlas_init_default(&atlas);
    nk_font_atlas_begin(&atlas);
    auto* val = nk_font_atlas_add_from_file(&atlas, filepath, height, cfg);

    const void* image{};
    int w{};
    int h{};
    image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    upload_atlas(static_cast<const gl::ubyte_t*>(image),
                 gl::width{static_cast<gl::uint_t>(w)},
                 gl::height{static_cast<gl::uint_t>(h)});
    nk_font_atlas_end(
        &atlas, nk_handle_id(static_cast<int>(_texture.value)), &_null);
    if (atlas.default_font != nullptr)
      nk_style_set_font(ctx, &atlas.default_font->handle);

    nk_style_load_all_cursors(ctx, static_cast<nk_cursor*>(atlas.cursors));
    nk_style_set_font(ctx, &val->handle);
  }

  void render(nk::context& ctx,
              gl::width window_width,
              gl::height window_height,
              gl::byte_size max_vertex_buffer,
              gl::byte_size max_element_buffer,
              nk_anti_aliasing anti_aliasing,
              struct nk_vec2 fb_scale) {
    gl::enable(gl::capability::blend);

    // clang-format off
    gl::mat_t<4, 4> ortho = {
        2.F, 0.F, 0.F, 0.F,
        0.F, -2.F, 0.F, 0.F,
        0.F, 0.F, -1.F, 0.F,
        -1.F, 1.F, 0.F, 1.F,
    };
    // clang-format on

    ortho[{0, 0}] /= static_cast<gl::float_t>(window_width.value);
    ortho[{1, 1}] /= static_cast<gl::float_t>(window_height.value);

    gl::blend_equation(gl::blend_mode::add);
    gl::blend_func(gl::blend_factor::src_alpha,
                   gl::blend_factor::one_minus_src_alpha);
    gl::disable(gl::capability::cull_face);
    gl::disable(gl::capability::depth_test);
    gl::enable(gl::capability::scissor_test);
    gl::active_texture(gl::texture_name::_0);
    _prog.use();
    gl::uniform(_texture_unif, 0U);
    gl::uniform(_projection_unif, ortho);

    _vao.bind();
    _vbo.bind();
    _ebo.bind();
    gl::buffer_data(
        gl::buffer_type::array, max_vertex_buffer, gl::data_hint::stream_draw);
    gl::buffer_data(gl::buffer_type::element_array,
                    max_element_buffer,
                    gl::data_hint::stream_draw);
    void* vertices =
        gl::map_buffer(gl::buffer_type::array, gl::access::write_only);
    void* elements =
        gl::map_buffer(gl::buffer_type::element_array, gl::access::write_only);
    nk_convert_config config{};
    config.vertex_layout =
        static_cast<const nk_draw_vertex_layout_element*>(vertex_layout);
    config.vertex_size = sizeof(vertex);
    config.vertex_alignment = alignof(vertex);
    config.null = _null;
    config.circle_segment_count = 22;
    config.curve_segment_count = 22;
    config.arc_segment_count = 22;
    config.global_alpha = 1.0f;
    config.shape_AA = anti_aliasing;
    config.line_AA = anti_aliasing;

    nk::buffer vbuf(vertices, max_vertex_buffer.value);
    nk::buffer ebuf(elements, max_vertex_buffer.value);
    ctx.convert(_cmds, vbuf, ebuf, &config);

    gl::unmap_buffer(gl::buffer_type::array);
    gl::unmap_buffer(gl::buffer_type::element_array);

    const nk_draw_command* cmd{nullptr};
    gl::offset offset{0};
    nk_draw_foreach(cmd, &ctx.ctx(), _cmds.buf()) {
      if (cmd->elem_count == 0) {
        continue;
      }
      gl::bind_texture(
          gl::texture_target::_2d,
          gl::texture_id{static_cast<gl::uint_t>(cmd->texture.id)});
      gl::scissor(
          gl::x{static_cast<gl::int_t>(cmd->clip_rect.x * fb_scale.x)},
          gl::y{(static_cast<gl::int_t>(window_height.value) -
                 static_cast<gl::int_t>(cmd->clip_rect.y + cmd->clip_rect.h)) *
                static_cast<gl::int_t>(fb_scale.y)},
          gl::width{static_cast<gl::uint_t>(cmd->clip_rect.w * fb_scale.x)},
          gl::height{static_cast<gl::uint_t>(cmd->clip_rect.h * fb_scale.y)});
      gl::draw_elements<gl::ushort_t>(
          gl::drawing_mode::triangles,
          gl::element_count{static_cast<gl::int_t>(cmd->elem_count)},
          offset);
      offset.value += cmd->elem_count;
    }
    ctx.clear();
    _cmds.clear();
  }

 private:
  explicit nk_gl3_backend(program prog) noexcept : _prog{std::move(prog)} {
    _vao.bind();
    _vbo.bind();
    _ebo.bind();
    gl::vertex_attrib_pointer<float>(
        gl::index{0},
        gl::element_count{2},
        gl::byte_stride{sizeof(vertex)},
        gl::byte_offset{offsetof(vertex, position)});
    gl::vertex_attrib_pointer<float>(gl::index{1},
                                     gl::element_count{2},
                                     gl::byte_stride{sizeof(vertex)},
                                     gl::byte_offset{offsetof(vertex, uv)});
    gl::vertex_attrib_pointer<nk_byte>(
        gl::index{2},
        gl::element_count{4},
        gl::normalized::yes,
        gl::byte_stride{sizeof(vertex)},
        gl::byte_offset{offsetof(vertex, color)});
  }

  program _prog;
  nk::dynamic_buffer _cmds;
  nk_draw_null_texture _null;
  vertex_array _vao;
  element_buffer _ebo;
  vertex_buffer _vbo;
  gl::texture_id _texture{0};
  gl::uniform_location _texture_unif{-1};
  gl::uniform_location _projection_unif{-1};

  static constexpr inline vs_source vertex_shader_source{
      "#version 330 core"
      "uniform mat4 ProjMtx;\n"
      "layout(location = 0) in vec2 Position;\n"
      "layout(location = 1) in vec2 TexCoord;\n"
      "layout(location = 2) in vec4 Color;\n"
      "out vec2 Frag_UV;\n"
      "out vec4 Frag_Color;\n"
      "void main() {\n"
      "   Frag_UV = TexCoord;\n"
      "   Frag_Color = Color;\n"
      "   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
      "}\n"};
  static constexpr inline fs_source fragment_shader_source{
      "#version 330 core"
      "precision mediump float;\n"
      "uniform sampler2D Texture;\n"
      "in vec2 Frag_UV;\n"
      "in vec4 Frag_Color;\n"
      "out vec4 Out_Color;\n"
      "void main(){\n"
      "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
      "}\n"};
  struct vertex {
    float position[2];  // NOLINT
    float uv[2];        // NOLINT
    nk_byte color[4];   // NOLINT
  };

  // NOLINTNEXTLINE
  constexpr static nk_draw_vertex_layout_element vertex_layout[] = {
      {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, offsetof(vertex, position)},
      {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, offsetof(vertex, uv)},
      {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, offsetof(vertex, color)},
      {NK_VERTEX_LAYOUT_END}};
  using vs_layout =
      packed<gl::vec_t<2, float>, gl::vec_t<2, float>, gl::vec_t<4, nk_byte>>;

  // NOLINTNEXTLINE(bugprone-exception-escape) spurious
  static inline result<program, gl_error> _generate() noexcept {
    return dpsg::vertex_shader::create(vertex_shader_source)
        // NOLINTNEXTLINE(bugprone-exception-escape) spurious
        .then([](vertex_shader&& vs) noexcept {
          return dpsg::fragment_shader::create(fragment_shader_source)
              .then([&vs](fragment_shader&& fs) noexcept {
                return result<program, gl_error>{
                    program::create(std::move(vs), std::move(fs))};
              });
        });
  }

 public:
  // NOLINTNEXTLINE(bugprone-exception-escape) spurious
  inline static result<nk_gl3_backend, gl_error> create() noexcept {
    return _generate().map([](program&& prog) noexcept {
      return nk_gl3_backend{std::move(prog)};
    });
  }
};  // class nk_gl3_backend

}  // namespace dpsg

namespace nk {
template <class Backend>
class nuklear_context : Backend {
 public:
 private:
};

}  // namespace nk

int main() {
  dpsg::within_glfw_context([] {
    nk::context ctx;
    ctx.with_window(
        "title",
        nk_rect(0, 0, 10, 10),
        nk::panel_flags::border,
        [](nk::window window) {
          auto p = window.panel();
          auto c1 = window.content_region_min();
          auto c2 = window.content_region();
          auto c3 = window.content_region_max();
          auto c4 = window.canvas();
          auto s = window.scroll();
          auto sx = window.x_scroll();
          auto sy = window.y_scroll();
          auto b = window.has_focus() || window.is_any_hovered();
          auto b2 = window.is_collapsed("const char *id") ||
                    window.is_closed("const char *id") ||
                    window.is_hidden("const char *id") ||
                    window.is_active("") || window.is_any_active();
          window.set_position("", nk_vec2(1, 2));
          window.set_size("", nk_vec2(1, 2));
          window.set_bounds("", nk_rect(1, 2, 1, 2));
          window.set_focus("const char *id");
          window.set_scroll(1, 2);
          window.close("");
          window.collapse("", NK_MAXIMIZED);
          window.collapse_if("", NK_MAXIMIZED, 0);
          window.show("", nk_show_states::NK_SHOWN);
          window.show_if("", nk_show_states::NK_SHOWN, 0);
          window.set_min_row_height(0.F);
          window.reset_min_row_height();
          auto bounds = window.widget_bounds();
          float pf = window.ratio_from_pixel(1);
          window.row_dynamic(pf, 0);
          window.row_static(0.F, 0, 0);
          window.with_row(
              nk_layout_format::NK_DYNAMIC, 0.F, 0, [](nk::row row) {
                row.push(0.F);
              });
          window.row_template_begin(0.F);
          window.row_template_end();
          window.row_template_push_static(bounds.x);
          window.row_template_push_variable(c1.x);
          window.row_template_push_static(0.F);
          window.with_space(
              nk_layout_format::NK_DYNAMIC,
              c1.y,
              *p->offset_x,
              [&](nk::space sp) {
                sp.push(c4->clip);
                struct nk_rect r =
                    sp.space_to_screen(sp.space_to_local(nk_rect(0, 0, 0, 0)));
                struct nk_vec2 v =
                    sp.space_to_screen(sp.space_to_local(nk_vec2(0, 0)));
              });
          window.with_group("", "", NK_MAXIMIZED, [](nk::group) {});
          window.with_group("", NK_MAXIMIZED, [](nk::group) {});
          unsigned int i{};
          nk_scroll scr{};
          window.with_group(&i, &i, "", NK_MAXIMIZED, [](nk::group) {});
          window.with_group(&scr, "", NK_MAXIMIZED, [](nk::group) {});
        });
    ctx.handle_input([](nk::input_handler input) { input.motion(1, 1); });
  });

  return 0;
}