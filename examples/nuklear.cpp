#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear/nuklear.h"

#include "buffers.hpp"
#include "layout.hpp"
#include "opengl.hpp"
#include "program.hpp"
#include "shaders.hpp"
#include "window.hpp"

#include <cstddef>
#include <iterator>

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

class context {
 public:
  using type = nk_context;
  using reference = nk_context&;
  using const_reference = const nk_context&;

  inline explicit context(const nk_user_font* font = nullptr) noexcept {
    nk_init_default(&_ctx, font);
  }

  inline ~context() noexcept { nk_free(&_ctx); }

  [[nodiscard]] inline constexpr const_reference ctx() const noexcept {
    return _ctx;
  }

  [[nodiscard]] inline constexpr reference ctx() noexcept { return _ctx; }

  inline void clear() noexcept { nk_clear(&_ctx); }

  inline void input_begin() noexcept { nk_input_begin(&_ctx); }

  inline void input_motion(int x, int y) noexcept {
    nk_input_motion(&_ctx, x, y);
  }

  inline void input_key(nk_keys key, bool down) noexcept {
    nk_input_key(&_ctx, key, down ? nk_true : nk_false);
  }

  inline void input_button(nk_buttons button,
                           int x,
                           int y,
                           bool down) noexcept {
    nk_input_button(&_ctx, button, x, y, down ? nk_true : nk_false);
  }

  inline void input_scroll(struct nk_vec2 val) noexcept {
    nk_input_scroll(&_ctx, val);
  }

  inline void input_char(char c) noexcept { nk_input_char(&_ctx, c); }

  inline void input_glyph(const nk_glyph glyph) noexcept {
    nk_input_glyph(&_ctx, glyph);
  }

  inline void input_unicode(nk_rune rune) noexcept {
    nk_input_unicode(&_ctx, rune);
  }

  inline void input_end() noexcept { nk_input_end(&_ctx); }

  class const_iterator {
    using difference_type = std::ptrdiff_t;
    using value_type = const nk_command;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    const_iterator& operator++() noexcept {
      _cmd = nk__next(_ctx, _cmd);
      return *this;
    }

   private:
    inline constexpr const_iterator(nk_context* ctx, pointer cmd) noexcept
        : _ctx(ctx), _cmd{cmd} {}

    nk_context* _ctx;
    pointer _cmd;
    friend class context;
  };

  inline const_iterator begin() noexcept {
    const nk_command* cmd = nk__begin(&_ctx);
    return const_iterator{&_ctx, cmd};
  }

 private:
  type _ctx{};
};

}  // namespace nk

namespace dpsg {
class nk_gl3_backend {
 public:
  nk_gl3_backend()
      : _prog{_generate().value()},
        _texture_unif{gl::get_uniform_location(_prog.id(), "Texture")},
        _projection_unif(gl::get_uniform_location(_prog.id(), "ProjMtx")) {
    _init_nk();
  }
  nk_gl3_backend(const nk_gl3_backend&) = delete;
  nk_gl3_backend(nk_gl3_backend&&) noexcept = default;
  nk_gl3_backend& operator=(const nk_gl3_backend&) = delete;
  nk_gl3_backend& operator=(nk_gl3_backend&&) noexcept = default;

  ~nk_gl3_backend() noexcept {
    nk_buffer_free(&_cmds);
    gl::delete_texture(_texture);
  }

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

  void render(nk_context* ctx,
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

    nk_buffer vbuf;
    nk_buffer ebuf;
    nk_buffer_init_fixed(&vbuf, vertices, max_vertex_buffer.value);
    nk_buffer_init_fixed(&ebuf, elements, max_element_buffer.value);
    nk_convert(ctx, &_cmds, &vbuf, &ebuf, &config);

    gl::unmap_buffer(gl::buffer_type::array);
    gl::unmap_buffer(gl::buffer_type::element_array);

    const nk_draw_command* cmd{nullptr};
    gl::offset offset{0};
    nk_draw_foreach(cmd, ctx, &_cmds) {
      if (cmd->elem_count == 0) {
        continue;
      }
      gl::bind_texture(
          gl::texture_target::_2d,
          gl::texture_id{static_cast<gl::uint_t>(cmd->texture.id)});
      gl::scissor(
          gl::x{static_cast<gl::int_t>(cmd->clip_rect.x * fb_scale.x)},
          gl::y{static_cast<gl::int_t>(
              (static_cast<gl::int_t>(window_height.value) -
               static_cast<gl::int_t>(cmd->clip_rect.y + cmd->clip_rect.h)) *
              fb_scale.y)},
          gl::width{static_cast<gl::uint_t>(cmd->clip_rect.w * fb_scale.x)},
          gl::height{static_cast<gl::uint_t>(cmd->clip_rect.h * fb_scale.y)});
      gl::draw_elements<gl::ushort_t>(
          gl::drawing_mode::triangles,
          gl::element_count{static_cast<gl::int_t>(cmd->elem_count)},
          offset);
      offset.value += cmd->elem_count;
    }
    nk_clear(ctx);
    nk_buffer_clear(&_cmds);
  }

 private:
  explicit nk_gl3_backend(program prog) noexcept : _prog{std::move(prog)} {
    _init_nk();
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
  nk_buffer _cmds;
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

  void _init_nk() noexcept { nk_buffer_init_default(&_cmds); }

 public:
  // NOLINTNEXTLINE(bugprone-exception-escape) spurious
  inline static result<nk_gl3_backend, gl_error> create() noexcept {
    return _generate().map([](program&& prog) noexcept {
      return nk_gl3_backend{std::move(prog)};
    });
  }
};  // namespace dpsg

}  // namespace dpsg

namespace nk {
template <class Backend>
class nuklear_context : Backend {
 public:
 private:
};

}  // namespace nk

int main() {
  return 0;
}