#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdlib.h>
#include <type_traits>
#include <utility>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

namespace dpsg {
template <class F> struct on_scope_exit_t {
  F f;
  constexpr explicit on_scope_exit_t(F &&f) : f(std::forward<F>(f)) {}
  ~on_scope_exit_t() noexcept { f(); }
};

template <class F> on_scope_exit_t(F &&f) -> on_scope_exit_t<F>;

enum class ExecutionStatus { Success = EXIT_SUCCESS, Failure = EXIT_FAILURE };

template <class F,
          std::enable_if_t<
              !std::is_same_v<std::invoke_result<F>, ExecutionStatus>, int> = 0>
ExecutionStatus
within_glfw_context(F &&f) noexcept(noexcept(std::forward<F>(f)())) {
  if (glfwInit() == GLFW_FALSE) {
    return ExecutionStatus::Failure;
  }

  auto on_exit = on_scope_exit_t{glfwTerminate};

  std::forward<F>(f)();

  return ExecutionStatus::Success;
}

template <class F,
          std::enable_if_t<
              std::is_same_v<std::invoke_result<F>, ExecutionStatus>, int> = 0>
ExecutionStatus
within_glfw_context(F &&f) noexcept(noexcept(std::forward<F>(f)())) {
  if (glfwInit() == GLFW_FALSE) {
    return ExecutionStatus::Failure;
  }

  auto on_exit = on_scope_exit_t{glfwTerminate};

  auto result = std::forward<F>(f)();

  return result;
}

struct init_hint {

  template <int Hint> struct value {
    value(const value &) noexcept = default;
    value(value &&) noexcept = default;
    value &operator=(const value &) noexcept = default;
    value &operator=(value &&) noexcept = default;
    ~value() noexcept = default;

  private:
    bool v = true;
    value() noexcept = default;
    constexpr explicit value(bool b) noexcept : v(b) {}
    friend init_hint;

  public:
    void operator()() noexcept { glfwInitHint(Hint, v); }
  };

  static constexpr value<GLFW_JOYSTICK_HAT_BUTTONS>
  joystick_hat_buttons(bool b) noexcept {
    return value<GLFW_JOYSTICK_HAT_BUTTONS>{b};
  }
  static constexpr value<GLFW_COCOA_CHDIR_RESOURCES>
  cocoa_chdir_resources(bool b) noexcept {
    return value<GLFW_COCOA_CHDIR_RESOURCES>{b};
  }
  static constexpr value<GLFW_COCOA_MENUBAR>
  cocoa_chdir_menubar(bool b) noexcept {
    return value<GLFW_COCOA_MENUBAR>{b};
  }
};

template <int I, class... Args>
decltype(auto) within_glfw_context(init_hint::value<I> hint,
                                   Args &&... args) noexcept {
  hint();
  return within_glfw_context(std::forward<Args>(args)...);
}

struct window_hint {

  template <int Hint, class T> struct value {
    value(const value &) noexcept = default;
    value(value &&) noexcept = default;
    value &operator=(const value &) noexcept = default;
    value &operator=(value &&) noexcept = default;
    ~value() noexcept = default;

  private:
    T v = true;
    value() noexcept = default;
    constexpr explicit value(T t) noexcept : v(t) {}
    friend window_hint;

  public:
    void operator()() noexcept { glfwInitHint(Hint, v); }
  };

  template <int I> using bool_hint = value<I, bool>;

  template <int I> using string_hint = value<I, const char *>;

  template <int I> using int_hint = value<I, int>;

  static constexpr int_hint<GLFW_CONTEXT_VERSION_MAJOR>
  context_version_major(int i) noexcept {
    return int_hint<GLFW_CONTEXT_VERSION_MAJOR>{i};
  }

  static constexpr int_hint<GLFW_CONTEXT_VERSION_MINOR>
  context_version_minor(int i) noexcept {
    return int_hint<GLFW_CONTEXT_VERSION_MINOR>{i};
  }
};

template <class T, int I, class... Args>
decltype(auto) with_window(window_hint::value<I, T> hint, Args &&... args) {
  hint();
  return with_window(std::forward<Args>(args)...);
}

struct window {};
struct width {
  int value;
};
struct height {
  int value;
};

template <class F,
          decltype((std::declval<F &&>(std::declval<window &>()), int{})) = 0>
ExecutionStatus with_window(width w, height h, const char *title, F &&f) {
  GLFWwindow *window = glfwCreateWindow(w.value, h.value, title, NULL, NULL);
  if (!window) {
    return ExecutionStatus::Failure;
  }
  auto on_exit = on_scope_exit([window] { glfwDestroyWindow(window); });
  std::forward<F>(*w);
  return ExecutionStatus::Success;
}

} // namespace dpsg

int main() {
  using namespace dpsg;

  return static_cast<int>(within_glfw_context([]() -> dpsg::ExecutionStatus {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
      std::cout << "Failed to create GLFW window" << std::endl;
      return dpsg::ExecutionStatus::Failure;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return dpsg::ExecutionStatus::Failure;
    }

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
      // input
      // -----
      processInput(window);

      // render
      // ------
      glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      // glfw: swap buffers and poll IO events (keys pressed/released, mouse
      // moved etc.)
      // -------------------------------------------------------------------------------
      glfwSwapBuffers(window);
      glfwPollEvents();
    }
    return dpsg::ExecutionStatus::Success;
  }));
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}