#ifndef GUARD_DPSG_OPENGL_HEADER
#define GUARD_DPSG_OPENGL_HEADER

#include "glad/glad.h"

#include "meta/is_one_of.hpp"
#include <type_traits>

namespace dpsg::gl {

namespace detail {
template <class T, class = void> struct has_value_member : std::false_type {};
template <class T>
struct has_value_member<T, std::void_t<decltype(std::declval<T>().value)>>
    : std::true_type {};

template <class T>
constexpr static inline bool has_value_member_v = has_value_member<T>::value;

template <class T, std::enable_if_t<!has_value_member_v<T>, int> = 0>
auto value(T t) noexcept {
  return t;
}
template <class T, std::enable_if_t<has_value_member_v<T>, int> = 0>
auto value(const T &t) noexcept {
  return t.value;
}

} // namespace detail

enum class buffer_bit {
  color = GL_COLOR_BUFFER_BIT,
  depth = GL_DEPTH_BUFFER_BIT,
  stencil = GL_STENCIL_BUFFER_BIT,
};

constexpr inline buffer_bit operator&(buffer_bit left,
                                      buffer_bit right) noexcept {
  return static_cast<buffer_bit>(static_cast<unsigned int>(left) &
                                 static_cast<unsigned int>(right));
}

constexpr inline buffer_bit operator|(buffer_bit left,
                                      buffer_bit right) noexcept {
  return static_cast<buffer_bit>(static_cast<unsigned int>(left) |
                                 static_cast<unsigned int>(right));
}

constexpr inline buffer_bit operator^(buffer_bit left,
                                      buffer_bit right) noexcept {
  return static_cast<buffer_bit>(static_cast<unsigned int>(left) ^
                                 static_cast<unsigned int>(right));
}

inline void clear(buffer_bit bb) noexcept { glClear(static_cast<int>(bb)); }

enum class drawing_mode {
  points = GL_POINTS,
  line_strip = GL_LINE_STRIP,
  line_loop = GL_LINE_LOOP,
  lines = GL_LINES,
  line_strip_adjacency = GL_LINE_STRIP_ADJACENCY,
  lines_adjacency = GL_LINES_ADJACENCY,
  triangle_strip = GL_TRIANGLE_STRIP,
  triangle_fan = GL_TRIANGLE_FAN,
  triangles = GL_TRIANGLES,
  triangle_strip_adjacency = GL_TRIANGLE_STRIP_ADJACENCY,
  triangles_adjacency = GL_TRIANGLES_ADJACENCY,
};

enum class capability {
  blend = GL_BLEND,
  clip_distance_0 = GL_CLIP_DISTANCE0,
  clip_distance_1 = GL_CLIP_DISTANCE1,
  clip_distance_2 = GL_CLIP_DISTANCE2,
  clip_distance_3 = GL_CLIP_DISTANCE3,
  clip_distance_4 = GL_CLIP_DISTANCE4,
  clip_distance_5 = GL_CLIP_DISTANCE5,
  clip_distance_6 = GL_CLIP_DISTANCE6,
  clip_distance_7 = GL_CLIP_DISTANCE7,
  color_logic_op = GL_COLOR_LOGIC_OP,
  cull_face = GL_CULL_FACE,
  // debug_output = GL_DEBUG_OUTPUT,
  // debug_output_synchronous = GL_DEBUG_OUTPUT_SYNCHRONOUS,
  depth_clamp = GL_DEPTH_CLAMP,
  depth_test = GL_DEPTH_TEST,
  dither = GL_DITHER,
  framebuffer_srgb = GL_FRAMEBUFFER_SRGB,
  line_smooth = GL_LINE_SMOOTH,
  multisample = GL_MULTISAMPLE,
  polygon_offset_fill = GL_POLYGON_OFFSET_FILL,
  polygon_offset_line = GL_POLYGON_OFFSET_LINE,
  polygon_offset_point = GL_POLYGON_OFFSET_POINT,
  polygon_smooth = GL_POLYGON_SMOOTH,
  primitive_restart = GL_PRIMITIVE_RESTART,
  // primitive_restart_fixed_index = GL_PRIMITIVE_RESTART_FIXED_INDEX,
  rasterizer_discard = GL_RASTERIZER_DISCARD,
  sample_alpha_to_coverage = GL_SAMPLE_ALPHA_TO_COVERAGE,
  sample_alpha_to_one = GL_SAMPLE_ALPHA_TO_ONE,
  sample_coverage = GL_SAMPLE_COVERAGE,
  // sample_shading = GL_SAMPLE_SHADING,
  sample_mask = GL_SAMPLE_MASK,
  scissor_test = GL_SCISSOR_TEST,
  stencil_test = GL_STENCIL_TEST,
  texture_cube_map_seamless = GL_TEXTURE_CUBE_MAP_SEAMLESS,
  program_point_size = GL_PROGRAM_POINT_SIZE,
};

inline void enable(capability cp) noexcept { glEnable(static_cast<int>(cp)); }
template <class... Args> inline void enable(Args &&... args) noexcept {
  (enable(std::forward<Args>(args)), ...);
}

inline void enable(capability cp, unsigned int index) noexcept {
  glEnablei(static_cast<int>(cp), index);
}

inline void disable(capability cp) noexcept { glDisable(static_cast<int>(cp)); }

inline void disable(capability cp, unsigned int i) noexcept {
  glDisablei(static_cast<int>(cp), i);
}

inline bool is_enabled(capability cp) noexcept {
  return glIsEnabled(static_cast<int>(cp)) == GL_TRUE;
}
struct index {
  unsigned int value;
};

inline bool is_enabled(capability cp, index index) noexcept {
  return glIsEnabledi(static_cast<int>(cp), index.value) == GL_TRUE;
}

enum class cull_mode {
  front = GL_FRONT,
  back = GL_BACK,
  front_and_back = GL_FRONT_AND_BACK,
};

inline void cull_face(cull_mode cm) noexcept {
  glCullFace(static_cast<int>(cm));
}

enum class buffer_type {
  array = GL_ARRAY_BUFFER,
  copy_read = GL_COPY_READ_BUFFER,
  copy_write = GL_COPY_WRITE_BUFFER,
  element_array = GL_ELEMENT_ARRAY_BUFFER,
  pixel_pack = GL_PIXEL_PACK_BUFFER,
  pixel_unpack = GL_PIXEL_UNPACK_BUFFER,
  texture = GL_TEXTURE_BUFFER,
  transform_feedback = GL_TRANSFORM_FEEDBACK_BUFFER,
  uniform = GL_UNIFORM_BUFFER
};

enum class data_hint {
  static_draw = GL_STATIC_DRAW,
  stream_draw = GL_STREAM_DRAW,
  dynamic_draw = GL_DYNAMIC_DRAW,
  static_copy = GL_STATIC_COPY,
  stream_copy = GL_STREAM_COPY,
  dynamic_copy = GL_DYNAMIC_COPY,
  static_read = GL_STATIC_READ,
  stream_read = GL_STREAM_READ,
  dynamic_read = GL_DYNAMIC_READ,
};

struct element_count {
  const std::size_t value;
};

inline void draw_arrays(drawing_mode mode, index first,
                        element_count count) noexcept {
  glDrawArrays(static_cast<int>(mode), first.value, count.value);
}

struct color {
  float r = 0.F;
  float g = 0.F;
  float b = 0.F;
  float a = 1.F;
};

struct r {
  float value;
};

struct g {
  float value;
};

struct b {
  float value;
};

struct a {
  float value;
};

inline void clear_color(color c) noexcept { glClearColor(c.r, c.g, c.b, c.a); }

namespace detail {
template <class T>
struct valid_colors : std::bool_constant<is_one_of_v<T, r, g, b, a>> {};

template <class... Args> struct no_duplication;
template <> struct no_duplication<> : std::true_type {};
template <class T, class... Args>
struct no_duplication<T, Args...>
    : std::bool_constant<!is_one_of_v<T, Args...> &&
                         no_duplication<Args...>::value> {};

template <class T, class U> inline auto get(U val) noexcept { return val; }

template <class U, class T, std::enable_if_t<std::is_same_v<T, U>, int> = 0,
          class... Args>
inline auto get(T val, [[maybe_unused]] Args... ignored) noexcept {
  return value(val);
}

template <class T, class U, class V,
          std::enable_if_t<!std::is_same_v<T, U>, int> = 0, class... Args>
inline auto get([[maybe_unused]] U ignored, V next, Args... args) noexcept {
  return get<T>(next, args...);
}

} // namespace detail

template <class... Args> inline void clear_color(Args &&... colors) noexcept {
  static_assert(sizeof...(Args) <= 4, "Too many components");
  static_assert((detail::valid_colors<Args>::value && ...),
                "clear_color(...) only accept r, g, b and a inputs");
  static_assert(detail::no_duplication<Args...>::value,
                "A color is duplicated");
  glClearColor(detail::get<r>(colors..., 0.F), detail::get<g>(colors..., 0.F),
               detail::get<b>(colors..., 0.F), detail::get<a>(colors..., 1.F));
}

struct memory_size {
  const std::size_t value;
};

namespace detail {
template <class T> struct deduce_gl_enum;
template <> struct deduce_gl_enum<float> {
  constexpr static inline int value = GL_FLOAT;
};
template <> struct deduce_gl_enum<int> {
  constexpr static inline int value = GL_INT;
};
template <> struct deduce_gl_enum<unsigned int> {
  constexpr static inline int value = GL_UNSIGNED_INT;
};
template <> struct deduce_gl_enum<char> {
  constexpr static inline int value = GL_BYTE;
};
template <> struct deduce_gl_enum<unsigned char> {
  constexpr static inline int value = GL_UNSIGNED_BYTE;
};
// NOLINTNEXTLINE
template <> struct deduce_gl_enum<unsigned short> {
  constexpr static inline int value = GL_UNSIGNED_SHORT;
};
// NOLINTNEXTLINE
template <> struct deduce_gl_enum<short> {
  constexpr static inline int value = GL_SHORT;
};
template <> struct deduce_gl_enum<double> {
  constexpr static inline int value = GL_DOUBLE;
};

template <class T>
constexpr static inline int deduce_gl_enum_v = deduce_gl_enum<T>::value;

template <class T, class = void> struct is_valid_gl_type : std::false_type {};

template <class T>
struct is_valid_gl_type<
    T, std::void_t<decltype(deduce_gl_enum<std::decay_t<T>>::value)>>
    : std::true_type {};

template <class T>
constexpr static inline bool is_valid_gl_type_v = is_valid_gl_type<T>::value;
} // namespace detail

template <class T>
inline void buffer_data(buffer_type type, memory_size size, T *ptr,
                        data_hint dmode) noexcept {
  static_assert(detail::is_valid_gl_type_v<T>,
                "Input pointer type is incompatible with the OpenGL API");
  glBufferData(static_cast<int>(type), size, ptr, static_cast<int>(dmode));
}

template <class T>
inline void buffer_data(buffer_type type, element_count count, T *ptr,
                        data_hint dmode) noexcept {
  static_assert(detail::is_valid_gl_type_v<T>,
                "Input pointer type is incompatible with the OpenGL API");
  glBufferData(static_cast<int>(type), count.value * sizeof(T), ptr,
               static_cast<int>(dmode));
}

template <class T, std::size_t N>
// NOLINTNEXTLINE
inline void buffer_data(buffer_type type, T (&ptr)[N],
                        data_hint dmode) noexcept {
  static_assert(detail::is_valid_gl_type_v<T>,
                "Input pointer type is incompatible with the OpenGL API");
  glBufferData(static_cast<int>(type), N * sizeof(T), ptr,
               static_cast<int>(dmode));
}

template <std::size_t N> struct vec_t {
  constexpr static inline std::size_t value = N;
};

namespace detail {
template <class T> struct is_vec_dimension_type : std::false_type {};
template <std::size_t N>
struct is_vec_dimension_type<vec_t<N>> : std::true_type {};
template <> struct is_vec_dimension_type<element_count> : std::true_type {};
template <class T>
constexpr static inline bool is_vec_dimension_type_v =
    is_vec_dimension_type<T>::value;
} // namespace detail

template <std::size_t N> constexpr static inline vec_t<N> vec;

struct stride {
  unsigned int value;
};
struct offset {
  unsigned int value;
};

template <typename T, class U,
          std::enable_if_t<detail::is_vec_dimension_type_v<U>, int> = 0,
          std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
inline void vertex_attrib_pointer(index idx, U element_count,
                                  stride s = stride{0},
                                  offset o = offset{0}) noexcept {
  static_assert(detail::is_valid_gl_type_v<T>,
                "The selected type is not supported by OpenGL");
  glVertexAttribPointer(
      idx.value, element_count.value, detail::deduce_gl_enum_v<T>, GL_FALSE,
      s.value * sizeof(T), reinterpret_cast<void *>(o.value * sizeof(T)));
}

template <typename T, class U,
          std::enable_if_t<detail::is_vec_dimension_type_v<U>, int> = 0,
          std::enable_if_t<std::is_integral_v<T>, int> = 0>
inline void vertex_attrib_pointer(index idx, U element_count,
                                  stride s = stride{0},
                                  offset o = offset{0}) noexcept {
  static_assert(detail::is_valid_gl_type_v<T>,
                "The selected type is not supported by OpenGL");
  glVertexAttribIPointer(idx.value, element_count.value,
                         detail::deduce_gl_enum_v<T>, s.value * sizeof(T),
                         reinterpret_cast<void *>(o.value * sizeof(T)));
}

enum class normalized {
  yes = GL_TRUE,
  no = GL_FALSE,
};

template <typename T, class U,
          std::enable_if_t<detail::is_vec_dimension_type_v<U>, int> = 0,
          std::enable_if_t<std::is_integral_v<T>, int> = 0>
inline void vertex_attrib_pointer(index idx, U element_count, normalized n,
                                  stride s = stride{0},
                                  offset o = offset{0}) noexcept {
  static_assert(detail::is_valid_gl_type_v<T>,
                "The selected type is not supported by OpenGL");
  glVertexAttribPointer(idx.value, element_count.value,
                        detail::deduce_gl_enum_v<T>, static_cast<int>(n),
                        s.value * sizeof(T),
                        reinterpret_cast<void *>(o.value * sizeof(T)));
}

namespace detail {
template <typename... Args>
using acceptable_index_types = std::conjunction<
    std::disjunction<std::is_same<std::decay_t<Args>, index>,
                     std::is_convertible<Args, unsigned int>>...>;
} // namespace detail

template <class... Args>
inline auto
enable_vertex_attrib_array(Args &&... is) noexcept -> std::void_t<decltype(
    std::enable_if_t<detail::acceptable_index_types<Args...>::value, int>{})> {
  (glEnableVertexAttribArray(detail::value(is)), ...);
}

template <class T>
inline void draw_elements(drawing_mode mode, element_count count,
                          offset o = offset{0}) noexcept {
  constexpr int gl_type = detail::deduce_gl_enum_v<T>;
  static_assert(
      gl_type == GL_UNSIGNED_BYTE || gl_type == GL_UNSIGNED_SHORT ||
          gl_type == GL_UNSIGNED_INT,
      "Input type to element rendering must be an unsigned integral type");
  glDrawElements(static_cast<int>(mode), count.value, gl_type,
                 reinterpret_cast<void *>(o.value * sizeof(T)));
}

struct generic_buffer_id {
  unsigned int value;
};

struct vertex_array_id {
  unsigned int value;
};

template <buffer_type Type> struct buffer_id : generic_buffer_id {
  constexpr static inline buffer_type buffer_type{Type};
};

inline void bind_buffer(buffer_type btype, generic_buffer_id id) noexcept {
  glBindBuffer(static_cast<int>(btype), id.value);
}

template <buffer_type Type>
inline void bind_buffer(buffer_id<Type> id) noexcept {
  glBindBuffer(static_cast<int>(buffer_id<Type>::buffer_type), id.value);
}

inline void unbind_buffer(buffer_type type) noexcept {
  glBindBuffer(static_cast<int>(type), 0);
}

template <std::size_t N>
// NOLINTNEXTLINE
inline void gen_buffers(generic_buffer_id (&buffer)[N]) noexcept {
  glGenBuffers(N, reinterpret_cast<unsigned int *>(buffer)); // NOLINT
}

inline void gen_buffers(std::size_t count, generic_buffer_id *buffer) noexcept {
  glGenBuffers(count, reinterpret_cast<unsigned int *>(buffer)); // NOLINT
}

inline void gen_buffer(generic_buffer_id &id) noexcept {
  glGenBuffers(1, reinterpret_cast<unsigned int *>(&id)); // NOLINT
}

template <buffer_type Type> inline buffer_id<Type> gen_buffer() noexcept {
  unsigned int i;
  glGenBuffers(1, &i);
  return buffer_id<Type>{i};
}

template <std::size_t N>
// NOLINTNEXTLINE
inline void delete_buffers(const generic_buffer_id (&buffer)[N]) noexcept {
  glDeleteBuffers(N, reinterpret_cast<const unsigned int *>(buffer)); // NOLINT
}

inline void delete_buffers(std::size_t count,
                           const generic_buffer_id *buffers) noexcept {
  glDeleteBuffers(count,
                  reinterpret_cast<const unsigned int *>(buffers)); // NOLINT
}

inline void delete_buffer(const generic_buffer_id &buffer) noexcept {
  glDeleteBuffers(1, reinterpret_cast<const unsigned int *>(&buffer)); // NOLINT
}

template <std::size_t N>
// NOLINTNEXTLINE
inline void gen_vertex_arrays(vertex_array_id (&buffer)[N]) noexcept {
  glGenVertexArrays(N, reinterpret_cast<unsigned int *>(buffer)); // NOLINT
}

inline void gen_vertex_arrays(std::size_t count,
                              vertex_array_id *buffer) noexcept {
  glGenVertexArrays(count, reinterpret_cast<unsigned int *>(buffer)); // NOLINT
}

inline void gen_vertex_array(vertex_array_id &buffer) noexcept {
  glGenVertexArrays(1, reinterpret_cast<unsigned int *>(&buffer)); // NOLINT
}

inline vertex_array_id gen_vertex_array() noexcept {
  unsigned int id; // NOLINT
  glGenVertexArrays(1, &id);
  return vertex_array_id{id};
}

template <std::size_t N>
// NOLINTNEXTLINE
inline void delete_vertex_arrays(const vertex_array_id (&buffer)[N]) noexcept {
  glDeleteVertexArrays(
      N, reinterpret_cast<const unsigned int *>(buffer)); // NOLINT
}

inline void delete_vertex_arrays(std::size_t count,
                                 const vertex_array_id *buffer) noexcept {
  glDeleteVertexArrays(
      count, reinterpret_cast<const unsigned int *>(buffer)); // NOLINT
}

inline void delete_vertex_array(const vertex_array_id &id) noexcept {
  glDeleteVertexArrays(1,
                       reinterpret_cast<const unsigned int *>(&id)); // NOLINT
}

inline void bind_vertex_array(vertex_array_id id) noexcept {
  glBindVertexArray(id.value);
}

inline void unbind_vertex_array() noexcept { glBindVertexArray(0); }

struct program_id {
  unsigned int value;
};

[[nodiscard]] inline program_id create_program() noexcept {
  return program_id{glCreateProgram()};
}

struct uniform_location {
  int value;
  [[nodiscard]] bool has_value() const noexcept { return value != -1; }
};

inline uniform_location get_uniform_location(program_id id,
                                             const char *name) noexcept {
  return uniform_location{glGetUniformLocation(id.value, name)};
}

enum class shader_type {
  // compute = GL_COMPUTE_SHADER,
  vertex = GL_VERTEX_SHADER,
  // tess_control = GL_TESS_CONTROL_SHADER,
  // tess_evaluation = GL_TESS_EVALUATION_SHADER,
  geometry = GL_GEOMETRY_SHADER,
  fragment = GL_FRAGMENT_SHADER
};

struct generic_shader_id {
  unsigned int value;
};

template <shader_type Type> struct shader_id : generic_shader_id {};

template <shader_type Type>
[[nodiscard]] inline shader_id<Type> create_shader() noexcept {
  return shader_id<Type>{{glCreateShader(static_cast<unsigned int>(Type))}};
}

[[nodiscard]] inline generic_shader_id
create_shader(shader_type type) noexcept {
  return generic_shader_id{glCreateShader(static_cast<unsigned int>(type))};
}

inline void shader_source(const generic_shader_id &id, std::size_t count,
                          const char **string, int *lengths) noexcept {
  glShaderSource(id.value, count, string, lengths);
}

template <std::size_t N>
// NOLINTNEXTLINE
inline void shader_source(const generic_shader_id &id, const char *(&string)[N],
                          int (&lengths)[N]) noexcept { // NOLINT
  shader_source(id, N, static_cast<const char **>(string),
                static_cast<int *>(lengths));
}

inline void shader_source(const generic_shader_id &id,
                          const char *string) noexcept {
  shader_source(id, 1, &string, nullptr);
}

inline void shader_source(const generic_shader_id &id, const char *string,
                          int length) noexcept {
  shader_source(id, 1, &string, &length);
}

template <std::size_t N>
inline void shader_source(const generic_shader_id &id,
                          const char (&string)[N] // NOLINT
                          ) noexcept {
  shader_source(id, static_cast<const char *>(string), N);
}

inline void compile_shader(const generic_shader_id &id) noexcept {
  glCompileShader(id.value);
}

static_assert(std::is_base_of<generic_shader_id, generic_shader_id>::value);

template <class... Args>
inline void attach_shader(program_id program, Args &&... args) noexcept {
  static_assert(
      std::conjunction_v<
          std::is_convertible<std::decay_t<Args>, generic_shader_id>...>,
      "All parameters to attach_shader after the program id must be shader "
      "ids");
  (glAttachShader(program.value, std::forward<Args>(args).value), ...);
}

inline void link_program(program_id id) noexcept { glLinkProgram(id.value); }

inline void use_program(program_id id) noexcept { glUseProgram(id.value); }

inline void delete_program(program_id id) noexcept {
  glDeleteProgram(id.value);
}

inline void delete_shader(const generic_shader_id &id) noexcept {
  glDeleteShader(id.value);
}

struct texture_id {
  unsigned int value;
};

template <std::size_t N>
// NOLINTNEXTLINE
inline void gen_textures(texture_id (&buffer)[N]) noexcept {
  glGenTextures(N, reinterpret_cast<unsigned int *>(buffer)); // NOLINT
}

inline void gen_textures(std::size_t count, texture_id *buffer) noexcept {
  glGenTextures(count, reinterpret_cast<unsigned int *>(buffer)); // NOLINT
}

inline void gen_texture(texture_id &buffer) noexcept {
  glGenTextures(1, reinterpret_cast<unsigned int *>(&buffer)); // NOLINT
}

[[nodiscard]] inline texture_id gen_texture() noexcept {
  unsigned int id; // NOLINT
  glGenTextures(1, &id);
  return texture_id{id};
}

template <std::size_t N>
// NOLINTNEXTLINE
inline void delete_textures(const texture_id (&buffer)[N]) noexcept {
  glDeleteTextures(N, reinterpret_cast<const unsigned int *>(buffer)); // NOLINT
}

inline void delete_textures(std::size_t count,
                            const texture_id *buffer) noexcept {
  glDeleteTextures(count,
                   reinterpret_cast<const unsigned int *>(buffer)); // NOLINT
}

inline void delete_texture(const texture_id &id) noexcept {
  glDeleteTextures(1,
                   reinterpret_cast<const unsigned int *>(&id)); // NOLINT
}

enum class texture_target {
  t1d = GL_TEXTURE_1D,
  t2d = GL_TEXTURE_2D,
  t3d = GL_TEXTURE_3D,
  t1d_array = GL_TEXTURE_1D_ARRAY,
  t2d_array = GL_TEXTURE_2D_ARRAY,
  rectangle = GL_TEXTURE_RECTANGLE,
  cube_map = GL_TEXTURE_CUBE_MAP,
  // cube_map_array = GL_TEXTURE_CUBE_MAP_ARRAY,
  buffer = GL_TEXTURE_BUFFER,
  t2d_multisample = GL_TEXTURE_2D_MULTISAMPLE,
  t2d_multisample_array = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
};

inline void bind_texture(texture_target t, texture_id id) noexcept {
  glBindTexture(static_cast<int>(t), id.value);
}

inline void unbind_texture(texture_target t) noexcept {
  glBindTexture(static_cast<int>(t), 0);
}

enum class texture_parameter_name {
  // depth_stencil = GL_DEPTH_STENCIL_TEXTURE_MODE,
  base_level = GL_TEXTURE_BASE_LEVEL,
  compare_func = GL_TEXTURE_COMPARE_FUNC,
  compare_mode = GL_TEXTURE_COMPARE_MODE,
  lod_bias = GL_TEXTURE_LOD_BIAS,
  min_filter = GL_TEXTURE_MIN_FILTER,
  mag_filter = GL_TEXTURE_MAG_FILTER,
  min_lod = GL_TEXTURE_MIN_LOD,
  max_lod = GL_TEXTURE_MAX_LOD,
  max_level = GL_TEXTURE_MAX_LEVEL,
  swizzle_r = GL_TEXTURE_SWIZZLE_R,
  swizzle_g = GL_TEXTURE_SWIZZLE_G,
  swizzle_b = GL_TEXTURE_SWIZZLE_B,
  swizzle_a = GL_TEXTURE_SWIZZLE_A,
  wrap_s = GL_TEXTURE_WRAP_S,
  wrap_t = GL_TEXTURE_WRAP_T,
  wrap_r = GL_TEXTURE_WRAP_R,
  border_color = GL_TEXTURE_BORDER_COLOR,
  swizzle_rgba = GL_TEXTURE_SWIZZLE_RGBA,
};

enum class compare_function {
  lequal = GL_LEQUAL,
  gequal = GL_GEQUAL,
  equal = GL_EQUAL,
  less = GL_LESS,
  greater = GL_GREATER,
  notequal = GL_NOTEQUAL,
  always = GL_ALWAYS,
  never = GL_NEVER,
};

enum class compare_mode {
  compare_ref_to_texture = GL_COMPARE_REF_TO_TEXTURE,
  none = GL_NONE
};

enum class wrap_target {
  s = GL_TEXTURE_WRAP_S,
  t = GL_TEXTURE_WRAP_T,
  r = GL_TEXTURE_WRAP_R,
};

enum class wrap_mode {
  clamp_to_edge = GL_CLAMP_TO_EDGE,
  clamp_to_border = GL_CLAMP_TO_BORDER,
  mirrored_repeat = GL_MIRRORED_REPEAT,
  repeat = GL_REPEAT,
  // GL_MIRROR_CLAMP_TO_EDGE,
};

enum class swizzle_target {
  r = GL_TEXTURE_SWIZZLE_R,
  g = GL_TEXTURE_SWIZZLE_G,
  b = GL_TEXTURE_SWIZZLE_B,
  a = GL_TEXTURE_SWIZZLE_A,
};

enum class swizzle_mode {
  red = GL_RED,
  green = GL_GREEN,
  blue = GL_BLUE,
  alpha = GL_ALPHA,
  zero = GL_ZERO,
  one = GL_ONE
};

enum class min_filter {
  nearest = GL_NEAREST,
  linear = GL_LINEAR,
  nearest_mipmap_nearest = GL_NEAREST_MIPMAP_NEAREST,
  linear_mipmap_nearest = GL_LINEAR_MIPMAP_NEAREST,
  nearest_mipmap_linear = GL_NEAREST_MIPMAP_LINEAR,
  linear_mipmap_linear = GL_LINEAR_MIPMAP_LINEAR,
};

enum class mag_filter {
  nearest = GL_NEAREST,
  linear = GL_LINEAR,
};

enum class lod_parameter {
  min = GL_TEXTURE_MAX_LOD,
  max = GL_TEXTURE_MIN_LOD,
  bias = GL_TEXTURE_LOD_BIAS,
};

enum class texture_level {
  base = GL_TEXTURE_BASE_LEVEL,
  max = GL_TEXTURE_MAX_LEVEL,
};

inline void tex_parameter(texture_target target, texture_parameter_name pname,
                          float f) noexcept {
  glTexParameterf(static_cast<int>(target), static_cast<int>(pname), f);
}

inline void tex_parameter(texture_target target, texture_parameter_name pname,
                          int i) noexcept {
  glTexParameteri(static_cast<int>(target), static_cast<int>(pname), i);
}

inline void tex_parameter(texture_target target, texture_parameter_name pname,
                          const int *i) noexcept {
  glTexParameteriv(static_cast<int>(target), static_cast<int>(pname), i);
}

inline void tex_parameter(texture_target target, texture_parameter_name pname,
                          const float *i) noexcept {
  glTexParameterfv(static_cast<int>(target), static_cast<int>(pname), i);
}

inline void tex_parameter(texture_target target, wrap_target wt,
                          wrap_mode mode) noexcept {
  glTexParameteri(static_cast<int>(target), static_cast<int>(wt),
                  static_cast<int>(mode));
}

inline void tex_parameter(texture_target target, swizzle_target wt,
                          swizzle_mode mode) noexcept {
  glTexParameteri(static_cast<int>(target), static_cast<int>(wt),
                  static_cast<int>(mode));
}

inline void tex_parameter(texture_target target, min_filter filter) noexcept {
  glTexParameteri(static_cast<int>(target), GL_TEXTURE_MIN_FILTER,
                  static_cast<int>(filter));
}

inline void tex_parameter(texture_target target, mag_filter filter) noexcept {
  glTexParameteri(static_cast<int>(target), GL_TEXTURE_MAG_FILTER,
                  static_cast<int>(filter));
}

inline void tex_parameter(texture_target target, compare_mode mode) noexcept {
  glTexParameteri(static_cast<int>(target), GL_TEXTURE_COMPARE_MODE,
                  static_cast<int>(mode));
}

inline void tex_parameter(texture_target target,
                          compare_function mode) noexcept {
  glTexParameteri(static_cast<int>(target), GL_TEXTURE_COMPARE_FUNC,
                  static_cast<int>(mode));
}

inline void tex_parameter(texture_target target, lod_parameter param,
                          float value) noexcept {
  glTexParameterf(static_cast<int>(target), static_cast<int>(param), value);
}

inline void tex_parameter(texture_target target, texture_level level,
                          int value) noexcept {
  glTexParameteri(static_cast<int>(target), static_cast<int>(level), value);
}

inline void tex_parameter(texture_target target,
                          const int (&colors)[4] // NOLINT
                          ) noexcept {
  glTexParameteriv(static_cast<int>(target), GL_TEXTURE_BORDER_COLOR,
                   static_cast<const int *>(colors));
}

inline void tex_parameter(texture_target target,
                          const float (&colors)[4] // NOLINT
                          ) noexcept {
  glTexParameterfv(static_cast<int>(target), GL_TEXTURE_BORDER_COLOR,
                   static_cast<const float *>(colors));
}

inline void tex_parameter_I(texture_target target,
                            const int (&colors)[4] // NOLINT
                            ) noexcept {
  glTexParameterIiv(static_cast<int>(target), GL_TEXTURE_BORDER_COLOR,
                    static_cast<const int *>(colors));
}

inline void tex_parameter_I(texture_target target,
                            const unsigned int (&colors)[4] // NOLINT
                            ) noexcept {
  glTexParameterIuiv(static_cast<int>(target), GL_TEXTURE_BORDER_COLOR,
                     static_cast<const unsigned int *>(colors));
}

inline void generate_mipmap(texture_target target) noexcept {
  glGenerateMipmap(static_cast<int>(target));
}

enum class texture_image_target {
  t2d = GL_TEXTURE_2D,
  proxy_2d = GL_PROXY_TEXTURE_2D,
  array_1d = GL_TEXTURE_1D_ARRAY,
  proxy_array_1d = GL_PROXY_TEXTURE_1D_ARRAY,
  rectangle = GL_TEXTURE_RECTANGLE,
  proxy_rectangle = GL_PROXY_TEXTURE_RECTANGLE,
  cube_map_positive_x = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
  cube_map_negative_x = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
  cube_map_positive_y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
  cube_map_negative_y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
  cube_map_positive_z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
  cube_map_negative_z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
  // proxy_cube_ma = GL_PROXY_TEXTURE_CUBE_MA,
};

struct mipmap_level {
  unsigned int value;
};

struct width {
  unsigned int value;
};

struct height {
  unsigned int value;
};

struct internal_format {
  int value;
};

enum class image_format {
  red = GL_RED,
  rg = GL_RG,
  rgb = GL_RGB,
  bgr = GL_BGR,
  rgba = GL_RGBA,
  bgra = GL_BGRA,
  red_integer = GL_RED_INTEGER,
  rg_integer = GL_RG_INTEGER,
  rgb_integer = GL_RGB_INTEGER,
  bgr_integer = GL_BGR_INTEGER,
  rgba_integer = GL_RGBA_INTEGER,
  bgra_integer = GL_BGRA_INTEGER,
  stencil_index = GL_STENCIL_INDEX,
  depth_component = GL_DEPTH_COMPONENT,
  depth_stencil = GL_DEPTH_STENCIL,
};

namespace detail {
struct pointer_placeholder {};
template <class... Args> struct contains;
template <class T, class U, class... Args>
struct contains<T, U, Args...> : contains<T, Args...> {};
template <class T, class... Args>
struct contains<T, T, Args...> : std::true_type {};
template <class T> struct contains<T> : std::false_type {};
template <class T, class... Args>
struct contains<pointer_placeholder, T *, Args...> : std::true_type {};

template <class T, class... Args>
constexpr static inline bool contains_v = contains<T, Args...>::value;

template <class T, class... Args>
inline T *get_data(T *data, Args... args) noexcept {
  return data;
}

template <class T, std::enable_if_t<!std::is_pointer_v<T>, int> = 0,
          class... Args>
inline auto *get_data([[maybe_unused]] T unused, Args... args) noexcept {
  return get_data(args...);
}

} // namespace detail

template <class... Args>
inline void tex_image_2D(texture_image_target target, Args... args) noexcept {
  static_assert(detail::contains_v<width, Args...>,
                "Parameter list must contain a width");
  static_assert(detail::contains_v<height, Args...>,
                "Parameter list must contain a height");
  static_assert(detail::contains_v<detail::pointer_placeholder, Args...>,
                "Parameter list must contain a data pointer");
  static_assert(detail::contains_v<internal_format, Args...>,
                "Parameter list must contain an internal format description");
  static_assert(detail::contains_v<image_format, Args...>,
                "Parameter list must contain a format description");

  auto *data = detail::get_data(args...);
  glTexImage2D(static_cast<int>(target), detail::get<mipmap_level>(args..., 0),
               detail::get<internal_format>(args...),
               detail::get<width>(args...), detail::get<height>(args...), 0,
               static_cast<int>(detail::get<image_format>(args...)),
               detail::deduce_gl_enum_v<
                   std::remove_pointer_t<std::decay_t<decltype(data)>>>,
               data);
}

} // namespace dpsg::gl

#endif