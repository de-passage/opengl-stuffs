#include "make_window.hpp"
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
      0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
      0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
      -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
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

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  prog.use();
  prog.uniform_location<int>("ourTexture").value().bind(0);
  wdw.render_loop([&] {
    // render
    // ------
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);

    wallText.bind();
    vao.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  });
}

int main() { return windowed(texture_example); }