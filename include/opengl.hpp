#ifndef GUARD_DPSG_OPENGL_HEADER
#define GUARD_DPSG_OPENGL_HEADER

#include "glad/glad.h"

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

inline bool is_enabled(capability cp, unsigned int index) noexcept {
  return glIsEnabledi(static_cast<int>(cp), index) == GL_TRUE;
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

inline void draw_arrays(drawing_mode mode, unsigned int first,
                        unsigned int element_count) noexcept {
  glDrawArrays(static_cast<int>(mode), first, element_count);
}

} // namespace dpsg::gl

#endif