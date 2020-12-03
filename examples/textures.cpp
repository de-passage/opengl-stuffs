#include "make_window.hpp"
#include "opengl.hpp"

#include "fixed_size_element_buffer.hpp"
#include "load_shaders.hpp"
#include "stbi_wrapper.hpp"
#include "structured_buffers.hpp"
#include "window.hpp"


void texture_example(dpsg::window &wdw) {
  using namespace dpsg;

  auto prog = load(vs_filename{"shaders/textured.vs"},
                   fs_filename{"shaders/textured.fs"});
  auto wallText = load(texture_filename{"assets/wall.jpg"}).value();
  auto smiley = load(texture_filename{"assets/awesomeface.png"}).value();

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
  fixed_size_element_buffer ebo{indices};

  gl::clear_color(gl::r{0.2F}, gl::g{0.3F}, gl::b{0.3F});
  prog.use();
  prog.uniform_location<int>("ourTexture").value().bind(0);
  wallText.bind();

  wdw.render_loop([&] {
    gl::clear(gl::buffer_bit::color);

    ebo.draw();
  });
}

int main() { return windowed(texture_example); }