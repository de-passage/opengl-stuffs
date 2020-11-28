#ifndef GUARD_DPSG_VERTEX_ARRAY_RENDERER_HEADER
#define GUARD_DPSG_VERTEX_ARRAY_RENDERER_HEADER

#include "common.hpp"

#include "buffers.hpp"
#include "shaders.hpp"

#include <cstdlib>

namespace dpsg {

template <std::size_t S, std::size_t N, class T = float>
struct vertex_array_renderer {
  // NOLINTNEXTLINE
  explicit vertex_array_renderer(T (&arr)[S * N]) {
    vao.bind();
    vbo.bind(dpsg::buffer_type::array);
    glBufferData(GL_ARRAY_BUFFER, N * S * sizeof(T), static_cast<void *>(arr),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, S, GL_FLOAT, GL_FALSE, S * sizeof(T),
                          static_cast<void *>(0));
    glEnableVertexAttribArray(0); // Needs to be in its own function?
  }

  explicit vertex_array_renderer(const std::array<T, N * S> &arr) {
    vao.bind();
    vbo.bind(dpsg::buffer_type::array);
    glBufferData(GL_ARRAY_BUFFER, N * S * sizeof(T), arr.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, S, GL_FLOAT, GL_FALSE, S * sizeof(T),
                          static_cast<void *>(0));
    glEnableVertexAttribArray(0); // Needs to be in its own function?
  }

  void render(dpsg::program &prog, drawing_mode dm) const {
    prog();
    render(dm);
  }
  void render(drawing_mode dm = drawing_mode::triangles) const {
    vao.bind();
    glDrawArrays(static_cast<int>(dm), 0, N);
  }

private:
  dpsg::buffer vbo;
  dpsg::vertex_array vao;
};

} // namespace dpsg

#endif // GUARD_DPSG_VERTEX_ARRAY_RENDERER_HEADER