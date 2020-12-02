#ifndef GUARD_DPSG_OPENGL_HEADER
#define GUARD_DPSG_OPENGL_HEADER

#include "glad/glad.h"

#include "meta/is_one_of.hpp"
#include <type_traits>

namespace dpsg::gl {

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

template <class T, class U, std::enable_if_t<!std::is_same_v<T, U>, int> = 0,
          class... Args>
inline float get_color([[maybe_unused]] U ignored, Args... args) noexcept {
  return get_color<T>(args...);
}

template <class U, class T, std::enable_if_t<std::is_same_v<T, U>, int> = 0,
          class... Args>
inline float get_color(T val, [[maybe_unused]] Args... ignored) noexcept {
  return val.value;
}

template <class T> float get_color(float val) noexcept { return val; }

} // namespace detail

template <class... Args> inline void clear_color(Args &&... colors) noexcept {
  static_assert(sizeof...(Args) <= 4, "Too many components");
  static_assert((detail::valid_colors<Args>::value && ...),
                "clear_color(...) only accept r, g, b and a inputs");
  static_assert(detail::no_duplication<Args...>::value,
                "A color is duplicated");
  glClearColor(detail::get_color<r>(colors..., 0.F),
               detail::get_color<g>(colors..., 0.F),
               detail::get_color<b>(colors..., 0.F),
               detail::get_color<a>(colors..., 1.F));
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

} // namespace dpsg::gl

#endif