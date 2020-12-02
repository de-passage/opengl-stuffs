#include "make_window.hpp"
#include "opengl.hpp"
#include "utils.hpp"

#include "buffers.hpp"
#include "input/keys.hpp"
#include "key_mapper.hpp"
#include "load_shaders.hpp"
#include "utility.hpp"
#include "window.hpp"

void texture_example(dpsg::window &wdw) {
  using namespace dpsg;
  using namespace dpsg::input;

  auto prog = load(vs_filename{"shaders/textured.vs"},
                   fs_filename{"shaders/textured.fs"});
  auto wallText = load(texture_filename{"assets/wall.jpg"}).value();
  key_mapper kmap;
  wdw.set_key_callback(window::key_callback{std::ref(kmap)});

  kmap.on(key::escape, close);

  float vertices[] = {
      // positions        // colors         // texture coords
      0.5F,  0.5F,  0.0F, 1.0F, 0.0F, 0.0F, 1.0F, 1.0F, // top right NOLINT
      0.5F,  -0.5F, 0.0F, 0.0F, 1.0F, 0.0F, 1.0F, 0.0F, // bottom right NOLINT
      -0.5F, -0.5F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, // bottom left NOLINT
      -0.5F, 0.5F,  0.0F, 1.0F, 1.0F, 0.0F, 0.0F, 1.0F  // top left NOLINT
  };

  unsigned int indices[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  vertex_buffer vbo;
  vertex_array vao;
  element_buffer ebo;
  vao.bind();
  vbo.bind();
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  ebo.bind();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  gl::clear_color(gl::r{0.2F}, gl::g{0.3F}, gl::b{0.3F});
  prog.use();
  prog.uniform_location<int>("ourTexture").value().bind(0);
  wdw.render_loop([&] {
    // render
    // ------
    gl::clear(gl::buffer_bit::color);
    glActiveTexture(GL_TEXTURE0);

    wallText.bind();
    vao.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  });
}

int main() { return windowed(texture_example); }