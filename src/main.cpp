#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdlib.h>
#include <type_traits>
#include <utility>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

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

enum class profile {
  core = GLFW_OPENGL_CORE_PROFILE,
  any = GLFW_OPENGL_ANY_PROFILE,
  compat = GLFW_OPENGL_COMPAT_PROFILE
};

struct window_hint {
private:
  template <int Hint, class T> struct trait_impl {
    template <class U,
              std::enable_if_t<std::is_convertible_v<U &&, T>, int> = 0>
    constexpr static void set_hint(U &&u) noexcept {
      glfwWindowHint(Hint, static_cast<int>(std::forward<T>(u)));
    }
  };

  template <int Hint> struct trait_impl<Hint, const char *> {
    constexpr static void set_hint(const char *c) noexcept {
      glfwWindowHintString(Hint, c);
    }
  };

  template <int Hint> struct trait_impl<Hint, bool> {
    constexpr static void set_hint(bool b) noexcept {
      glfwWindowHint(Hint, b ? GLFW_TRUE : GLFW_FALSE);
    }
  };

  template <int Hint, class T> struct trait : trait_impl<Hint, T> {
    constexpr static inline int hint = Hint;
    using type = T;
  };

  template <int Hint, class T> struct hint_type;
  template <class... Args> struct hint_list;

public:
  template <class T> struct value;

  template <int Hint, class T> struct value<hint_type<Hint, T>> {
    value(const value &) noexcept = default;
    value(value &&) noexcept = default;
    value &operator=(const value &) noexcept = default;
    value &operator=(value &&) noexcept = default;
    ~value() noexcept = default;

  private:
    T v = {};
    value() noexcept = default;
    constexpr explicit value(T t) noexcept : v(t) {}
    friend window_hint;

  public:
    void operator()() noexcept { trait<Hint, T>::set_hint(v); }
  };

  template <int I, class T> using hint = value<hint_type<I, T>>;

  template <int I> using bool_hint = hint<I, bool>;

  template <int I> using string_hint = hint<I, const char *>;

  template <int I> using int_hint = hint<I, int>;

  static constexpr int_hint<GLFW_CONTEXT_VERSION_MAJOR>
  context_version_major(int i) noexcept {
    return int_hint<GLFW_CONTEXT_VERSION_MAJOR>{i};
  }

  static constexpr int_hint<GLFW_CONTEXT_VERSION_MINOR>
  context_version_minor(int i) noexcept {
    return int_hint<GLFW_CONTEXT_VERSION_MINOR>{i};
  }

  static constexpr hint<GLFW_OPENGL_PROFILE, profile>
  opengl_profile(profile p) noexcept {
    return hint<GLFW_OPENGL_PROFILE, profile>{p};
  }

  static constexpr bool_hint<GLFW_OPENGL_FORWARD_COMPAT>
  opengl_forward_compat(bool b) noexcept {
    return bool_hint<GLFW_OPENGL_FORWARD_COMPAT>{b};
  }
};

template <class T, class... Args>
decltype(auto) with_window(window_hint::value<T> hint, Args &&... args) {
  hint();
  return with_window(std::forward<Args>(args)...);
}

class window {
public:
  explicit window(GLFWwindow *w) : _window(w) {}
  window(window &&w) noexcept : _window(std::exchange(w._window, nullptr)) {}
  window(const window &) = delete;
  window &operator=(window &&w) noexcept {
    std::swap(w._window, _window);
    w._clean();
    return *this;
  }
  window &operator=(const window &) = delete;
  ~window() { _clean(); }

  constexpr const GLFWwindow *data() const noexcept { return _window; }
  constexpr GLFWwindow *data() noexcept { return _window; }
  GLFWwindow *release() &&noexcept { return std::exchange(_window, nullptr); }

private:
  void _clean() {
    if (_window) {
      glfwDestroyWindow(_window);
    }
  }
  GLFWwindow *_window;
};

struct width {
  int value;
};
struct height {
  int value;
};

template <class F,
          decltype((std::declval<F &&>()(std::declval<window &>()), int{})) = 0>
ExecutionStatus with_window(width w, height h, const char *title, F &&f) {
  GLFWwindow *wptr = glfwCreateWindow(w.value, h.value, title, NULL, NULL);
  if (!wptr) {
    return ExecutionStatus::Failure;
  }
  window win(wptr);
  std::forward<F>(f)(win);
  return ExecutionStatus::Success;
}

} // namespace dpsg

// settings
constexpr dpsg::width SCR_WIDTH{800};
constexpr dpsg::height SCR_HEIGHT{600};

int main() {
  using namespace dpsg;

  return static_cast<int>(within_glfw_context([]() -> dpsg::ExecutionStatus {
    using wh = window_hint;
    return with_window(
        wh::context_version_major(3), wh::context_version_minor(3),
        wh::opengl_profile(profile::core),

#ifdef __APPLE__
        wh::opengl_forward_compat(true),
#endif
        SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", [](window &window) -> void {
          // glfw window creation
          // --------------------
          glfwMakeContextCurrent(window.data());
          glfwSetFramebufferSizeCallback(window.data(),
                                         framebuffer_size_callback);

          // glad: load all OpenGL function pointers
          // ---------------------------------------
          if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return; // dpsg::ExecutionStatus::Failure;
          }

          // render loop
          // -----------
          while (!glfwWindowShouldClose(window.data())) {
            // input
            // -----
            processInput(window.data());

            // render
            // ------
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // glfw: swap buffers and poll IO events (keys
            // pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window.data());
            glfwPollEvents();
          }
        });
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