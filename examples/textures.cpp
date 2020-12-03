#include "make_window.hpp"
#include "opengl.hpp"
#include "utils.hpp"

#include "buffers.hpp"
#include "input/keys.hpp"
#include "key_mapper.hpp"
#include "load_shaders.hpp"
#include "structured_buffers.hpp"
#include "texture.hpp"
#include "utility.hpp"
#include "window.hpp"

void texture_example(dpsg::window &wdw) {
  using namespace dpsg;

  auto prog = load(vs_filename{"shaders/textured.vs"},
                   fs_filename{"shaders/textured.fs"});
  auto wallText = load(texture_filename{"assets/wall.jpg"}).value();

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

  using layout = packed<group<3>, group<3>, group<2>>;
  structured_buffer buffer(layout{}, vertices);
  element_buffer ebo;
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
    gl::active_texture(gl::texture_name::_0);

    wallText.bind();
    buffer.get_vertex_buffer().bind();
    gl::draw_elements<unsigned int>(gl::drawing_mode::triangles,
                                    gl::element_count{6});
  });
}

int main() { return windowed(texture_example); }