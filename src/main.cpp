#include <exception>
#include <glad/glad.h>

#include "glfw_context.hpp"
#include "input/keys.hpp"
#include "utility.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

// settings
constexpr dpsg::window::width SCR_WIDTH{800};
constexpr dpsg::window::height SCR_HEIGHT{600};

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(dpsg::window &window);

template <class F> dpsg::ExecutionStatus make_window(F f) {
  using namespace dpsg;

  return within_glfw_context([f]() -> dpsg::ExecutionStatus {
    using wh = window_hint;

    return with_window(
        wh::context_version(3, 3), wh::opengl_profile(profile::core),
#ifdef __APPLE__
        wh::opengl_forward_compat(true),
#endif
        SCR_WIDTH, SCR_HEIGHT, window::title{"LearnOpenGL"},
        [f](window &wdw) -> ExecutionStatus {
          // glfw window creation
          // --------------------
          wdw.make_context_current();
          wdw.set_framebuffer_size_callback(framebuffer_size_callback);

          // glad: load all OpenGL function pointers
          // ---------------------------------------
          if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return dpsg::ExecutionStatus::Failure;
          }

          // render loop
          // -----------
          f(wdw);
          return ExecutionStatus::Success;
        });
  });
}

float vertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};
const char *vertex_shader_source =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragment_shader_source = "#version 330 core\n"
                                     "out vec4 fragColor;\n"
                                     "void main() {"
                                     "fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);"
                                     "}";

namespace dpsg {
template <std::size_t N> class buffer_array_impl {
public:
  buffer_array_impl() noexcept {
    glGenBuffers(N, static_cast<unsigned int *>(values));
  }
  buffer_array_impl(const buffer_array_impl &) = delete;
  buffer_array_impl(buffer_array_impl &&a) {
    for (std::size_t i; i < N; ++i) {
      values[i] = std::exchange(a.values[i], 0);
    }
  }

  buffer_array_impl &operator=(buffer_array_impl &&) {

    for (std::size_t i; i < N; ++i) {
      values[i] = std::exchange(a.values[i], 0);
    }
  }
  buffer_array_impl &operator=(const buffer_array_impl &) = delete;
  ~buffer_array_impl() noexcept {
    glDeleteBuffers(N, static_cast<unsigned int *>(values));
  }

  unsigned int operator[](std::size_t s) const noexcept { return values[s]; }

protected:
  unsigned int values[N]; // NOLINT
};

template <std::size_t N> class buffer_array : public buffer_array_impl<N> {};

class buffer : buffer_array_impl<1> {
public:
  explicit operator unsigned int() const { return values[0]; }
};

class shader_error : public std::exception {
public:
  constexpr static inline std::size_t buffer_size = 512;
  shader_error(unsigned int i) : id(i) {}

  [[nodiscard]] const char *what() const override {
    glGetShaderInfoLog(id, buffer_size, nullptr, info);
    return info;
  }

private:
  unsigned int id;
  static inline char info[buffer_size]{};
};
template <int Type> class shader {
public:
  explicit shader(unsigned int i) noexcept : id{i} {}

  shader(const shader &) = delete;
  shader(shader &&s) noexcept : id(std::exchange(s.id, 0)) {}
  shader &operator=(const shader &) = delete;
  shader &operator=(shader &&s) noexcept {
    id = std::exchange(s.id, 0);
    return *this;
  }
  ~shader() noexcept { glDeleteShader(id); }

  static std::variant<shader, shader_error>
  create(const char *source) noexcept {
    unsigned int shader_id = glCreateShader(Type);
    glShaderSource(shader_id, 1, &source, nullptr);
    glCompileShader(shader_id);
    int success = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
      return std::variant<shader, shader_error>{
          std::in_place_type<shader_error>, shader_id};
    }
    return std::variant<shader, shader_error>{std::in_place_type<shader>,
                                              shader_id};
  }

  explicit operator unsigned int() const noexcept { return id; }

private:
  unsigned int id;
};

using vertex_shader = shader<GL_VERTEX_SHADER>;
using fragment_shader = shader<GL_FRAGMENT_SHADER>;

class program_error : public std::exception {
public:
  constexpr static inline std::size_t buffer_size = 512;
  explicit program_error(unsigned int i) : id{i} {}

  [[nodiscard]] const char *what() const override {
    glGetProgramInfoLog(id, 512, nullptr, info);
    return info;
  }

private:
  unsigned int id;
  static inline char info[buffer_size];
};

class program {
public:
  explicit program(unsigned int i) noexcept : id{i} {}

  program(const program &) = delete;
  program(program &&s) noexcept : id(std::exchange(s.id, 0)) {}
  program &operator=(const program &) = delete;
  program &operator=(program &&s) noexcept {
    id = std::exchange(s.id, 0);
    return *this;
  }
  ~program() noexcept { glDeleteProgram(id); }

  template <int I>
  static void attach_shader(unsigned int i, const shader<I> &shader) noexcept {
    glAttachShader(i, static_cast<unsigned int>(shader));
  }

  template <class... Args>
  static std::variant<program, program_error>
  create(Args &&... shaders) noexcept {
    unsigned int id = glCreateProgram();
    (attach_shader(id, std::forward<Args>(shaders)), ...);
    glLinkProgram(id);
    int success{};
    glGetProgramiv(id, GL_LINK_STATUS, &success);

    if (success != GL_TRUE) {
      return std::variant<program, program_error>{
          std::in_place_type<program_error>, id};
    }

    return std::variant<program, program_error>{std::in_place_type<program>,
                                                id};
  }

  void operator()() const noexcept { glUseProgram(id); }

private:
  unsigned int id;
};

} // namespace dpsg

int main() {

  dpsg::ExecutionStatus r = dpsg::ExecutionStatus::Failure;
  try {
    r = make_window([](dpsg::window &wdw) {
      auto vvar = dpsg::vertex_shader::create(vertex_shader_source);
      if (auto *verror = std::get_if<dpsg::shader_error>(&vvar)) {
        throw std::runtime_error{verror->what()};
      }
      auto vshader = std::get<dpsg::vertex_shader>(std::move(vvar));

      auto fvar = dpsg::fragment_shader::create(fragment_shader_source);
      if (auto *ferror = std::get_if<dpsg::shader_error>(&fvar)) {
        throw std::runtime_error{ferror->what()};
      }
      auto fshader = std::get<dpsg::fragment_shader>(std::move(fvar));

      auto pvar = dpsg::program::create(vshader, fshader);
      if (auto *perror = std::get_if<dpsg::program_error>(&pvar)) {
        throw std::runtime_error{perror->what()};
      }
      auto shader_program = std::get<dpsg::program>(std::move(pvar));

      dpsg::buffer vbo;
      unsigned int vao;
      glGenVertexArrays(1, &vao);

      glBindVertexArray(vao);

      glBindBuffer(GL_ARRAY_BUFFER, static_cast<unsigned int>(vbo));
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                            (void *)0);
      glEnableVertexAttribArray(0);

      wdw.render_loop([&] {
        processInput(wdw);
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader_program();
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
      });
    });

  } catch (std::exception &e) {
    std::cerr << "Exception in main: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unhandled error occured" << std::endl;
  }

  return static_cast<int>(r);
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(dpsg::window &window) {
  if (window.get_key(dpsg::input::key::escape) ==
      dpsg::input::status::pressed) {
    window.should_close(true);
  }
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback([[maybe_unused]] GLFWwindow *window, int width,
                               int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}