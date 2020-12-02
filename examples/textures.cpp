#include "make_window.hpp"
#include "opengl.hpp"
#include "utils.hpp"

#include "buffers.hpp"
#include "input/keys.hpp"
#include "key_mapper.hpp"
#include "load_shaders.hpp"
#include "texture.hpp"
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

  constexpr float vertices[] = {
      // positions        // colors         // texture coords
      0.5F,  0.5F,  0.0F, 1.0F, 0.0F, 0.0F, 1.0F, 1.0F, // top right NOLINT
      0.5F,  -0.5F, 0.0F, 0.0F, 1.0F, 0.0F, 1.0F, 0.0F, // bottom right NOLINT
      -0.5F, -0.5F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, // bottom left NOLINT
      -0.5F, 0.5F,  0.0F, 1.0F, 1.0F, 0.0F, 0.0F, 1.0F  // top left NOLINT
  };

  constexpr unsigned int indices[] = {
      0, 1, 3, // first triangle
      1, 2, 3  // second triangle
  };

  vertex_buffer vbo;
  vertex_array vao;
  element_buffer ebo;
  vao.bind();
  vbo.bind();
  gl::buffer_data(gl::buffer_type::array, vertices, gl::data_hint::static_draw);

  gl::vertex_attrib_pointer<float>(gl::index{0}, gl::vec<3>, gl::stride{8});
  gl::vertex_attrib_pointer<float>(gl::index{1}, gl::element_count{3},
                                   gl::stride{8}, gl::offset{3});
  gl::vertex_attrib_pointer<float>(gl::index{2}, gl::vec<2>, gl::stride{8},
                                   gl::offset{6});
  gl::enable_vertex_attrib_array(0, gl::position{1}, 2);

  ebo.bind();
  gl::buffer_data(gl::buffer_type::element_array, indices,
                  gl::data_hint::static_draw);

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
    gl::draw_elements<unsigned int>(gl::drawing_mode::triangles, gl::element_count{6});
  });
}

int main() { return windowed(texture_example); }