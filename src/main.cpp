#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdlib.h>
#include <string_view>
#include <type_traits>
#include <utility>

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

namespace input {
enum class status {
  released = GLFW_RELEASE,
  pressed = GLFW_PRESS,
  repeat = GLFW_REPEAT
};

enum class key {
  unknown = GLFW_KEY_UNKNOWN,
  space = GLFW_KEY_SPACE,
  apostrophe = GLFW_KEY_APOSTROPHE,
  comma = GLFW_KEY_COMMA,
  minus = GLFW_KEY_MINUS,
  period = GLFW_KEY_PERIOD,
  slash = GLFW_KEY_SLASH,
  num0 = GLFW_KEY_0,
  num1 = GLFW_KEY_1,
  num2 = GLFW_KEY_2,
  num3 = GLFW_KEY_3,
  num4 = GLFW_KEY_4,
  num5 = GLFW_KEY_5,
  num6 = GLFW_KEY_6,
  num7 = GLFW_KEY_7,
  num8 = GLFW_KEY_8,
  num9 = GLFW_KEY_9,
  semicolon = GLFW_KEY_SEMICOLON,
  equal = GLFW_KEY_EQUAL,
  A = GLFW_KEY_A,
  B = GLFW_KEY_B,
  C = GLFW_KEY_C,
  D = GLFW_KEY_D,
  E = GLFW_KEY_E,
  F = GLFW_KEY_F,
  G = GLFW_KEY_G,
  H = GLFW_KEY_H,
  I = GLFW_KEY_I,
  J = GLFW_KEY_J,
  K = GLFW_KEY_K,
  L = GLFW_KEY_L,
  M = GLFW_KEY_M,
  N = GLFW_KEY_N,
  O = GLFW_KEY_O,
  P = GLFW_KEY_P,
  Q = GLFW_KEY_Q,
  R = GLFW_KEY_R,
  S = GLFW_KEY_S,
  T = GLFW_KEY_T,
  U = GLFW_KEY_U,
  V = GLFW_KEY_V,
  W = GLFW_KEY_W,
  X = GLFW_KEY_X,
  Y = GLFW_KEY_Y,
  Z = GLFW_KEY_Z,
  left_bracket = GLFW_KEY_LEFT_BRACKET,
  backslash = GLFW_KEY_BACKSLASH,
  right_bracket = GLFW_KEY_RIGHT_BRACKET,
  grave_accent = GLFW_KEY_GRAVE_ACCENT,
  world_1 = GLFW_KEY_WORLD_1,
  world_2 = GLFW_KEY_WORLD_2,
  escape = GLFW_KEY_ESCAPE,
  enter = GLFW_KEY_ENTER,
  tab = GLFW_KEY_TAB,
  backspace = GLFW_KEY_BACKSPACE,
  insert = GLFW_KEY_INSERT,
  del = GLFW_KEY_DELETE,
  right = GLFW_KEY_RIGHT,
  left = GLFW_KEY_LEFT,
  down = GLFW_KEY_DOWN,
  up = GLFW_KEY_UP,
  page_up = GLFW_KEY_PAGE_UP,
  page_down = GLFW_KEY_PAGE_DOWN,
  home = GLFW_KEY_HOME,
  end = GLFW_KEY_END,
  caps_lock = GLFW_KEY_CAPS_LOCK,
  scroll_lock = GLFW_KEY_SCROLL_LOCK,
  num_lock = GLFW_KEY_NUM_LOCK,
  print_screen = GLFW_KEY_PRINT_SCREEN,
  pause = GLFW_KEY_PAUSE,
  F1 = GLFW_KEY_F1,
  F2 = GLFW_KEY_F2,
  F3 = GLFW_KEY_F3,
  F4 = GLFW_KEY_F4,
  F5 = GLFW_KEY_F5,
  F6 = GLFW_KEY_F6,
  F7 = GLFW_KEY_F7,
  F8 = GLFW_KEY_F8,
  F9 = GLFW_KEY_F9,
  F10 = GLFW_KEY_F10,
  F11 = GLFW_KEY_F11,
  F12 = GLFW_KEY_F12,
  F13 = GLFW_KEY_F13,
  F14 = GLFW_KEY_F14,
  F15 = GLFW_KEY_F15,
  F16 = GLFW_KEY_F16,
  F17 = GLFW_KEY_F17,
  F18 = GLFW_KEY_F18,
  F19 = GLFW_KEY_F19,
  F20 = GLFW_KEY_F20,
  F21 = GLFW_KEY_F21,
  F22 = GLFW_KEY_F22,
  F23 = GLFW_KEY_F23,
  F24 = GLFW_KEY_F24,
  F25 = GLFW_KEY_F25,
  kp_0 = GLFW_KEY_KP_0,
  kp_1 = GLFW_KEY_KP_1,
  kp_2 = GLFW_KEY_KP_2,
  kp_3 = GLFW_KEY_KP_3,
  kp_4 = GLFW_KEY_KP_4,
  kp_5 = GLFW_KEY_KP_5,
  kp_6 = GLFW_KEY_KP_6,
  kp_7 = GLFW_KEY_KP_7,
  kp_8 = GLFW_KEY_KP_8,
  kp_9 = GLFW_KEY_KP_9,
  kp_decimal = GLFW_KEY_KP_DECIMAL,
  kp_divide = GLFW_KEY_KP_DIVIDE,
  kp_multiply = GLFW_KEY_KP_MULTIPLY,
  kp_subtract = GLFW_KEY_KP_SUBTRACT,
  kp_add = GLFW_KEY_KP_ADD,
  kp_enter = GLFW_KEY_KP_ENTER,
  kp_equal = GLFW_KEY_KP_EQUAL,
  left_shift = GLFW_KEY_LEFT_SHIFT,
  left_control = GLFW_KEY_LEFT_CONTROL,
  left_alt = GLFW_KEY_LEFT_ALT,
  left_super = GLFW_KEY_LEFT_SUPER,
  right_shift = GLFW_KEY_RIGHT_SHIFT,
  right_control = GLFW_KEY_RIGHT_CONTROL,
  right_alt = GLFW_KEY_RIGHT_ALT,
  right_super = GLFW_KEY_RIGHT_SUPER,
  menu = GLFW_KEY_MENU,
  last = GLFW_KEY_LAST
};
} // namespace input

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

  template <int Hint, class T> struct trait : trait_impl<Hint, T> {};

public:
  template <int Hint, class T> struct hint_type {
    constexpr static inline int hint = Hint;
    using type = T;
  };
  template <class... Args> struct hint_list;

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

  template <class T, class... Ts>
  struct value<hint_list<T, Ts...>> : value<hint_list<Ts...>> {
    value(const value &) noexcept = default;
    value(value &&) noexcept = default;
    value &operator=(const value &) noexcept = default;
    value &operator=(value &&) noexcept = default;
    ~value() noexcept = default;

  private:
    using base = value<hint_list<Ts...>>;
    typename T::type _value;
    value() noexcept = default;
    template <
        class U, class... Args,
        std::enable_if_t<std::is_convertible_v<U, typename T::type>, int> = 0>
    constexpr explicit value(U &&u, Args &&... ts) noexcept
        : base(std::forward<Args>(ts)...), _value(std::forward<U>(u)) {}

    friend window_hint;

  public:
    void operator()() noexcept {
      trait<T::hint, typename T::type>::set_hint(_value);
      base::operator()();
    }
  };

  template <> struct value<hint_list<>> {
    void operator()() noexcept {}
  };

  template <int I, class T> using hint = value<hint_type<I, T>>;

  template <int I> using bool_hint = hint_type<I, bool>;

  template <int I> using string_hint = hint_type<I, const char *>;

  template <int I> using int_hint = hint_type<I, int>;

  using context_version_major_t = int_hint<GLFW_CONTEXT_VERSION_MAJOR>;
  static constexpr value<context_version_major_t>
  context_version_major(int i) noexcept {
    return value<context_version_major_t>{i};
  }

  using context_version_minor_t = int_hint<GLFW_CONTEXT_VERSION_MINOR>;
  static constexpr value<context_version_minor_t>
  context_version_minor(int i) noexcept {
    return value<context_version_minor_t>{i};
  }

  template <class... Args> using composite_hint = value<hint_list<Args...>>;
  static constexpr composite_hint<context_version_major_t,
                                  context_version_minor_t>
  context_version(int major, int minor) noexcept {
    return composite_hint<context_version_major_t, context_version_minor_t>{
        major, minor};
  }

  static constexpr hint<GLFW_OPENGL_PROFILE, profile>
  opengl_profile(profile p) noexcept {
    return hint<GLFW_OPENGL_PROFILE, profile>{p};
  }

  static constexpr value<bool_hint<GLFW_OPENGL_FORWARD_COMPAT>>
  opengl_forward_compat(bool b) noexcept {
    return value<bool_hint<GLFW_OPENGL_FORWARD_COMPAT>>{b};
  }
};

template <class T, class... Args>
decltype(auto) with_window(window_hint::value<T> hint, Args &&... args) {
  hint();
  return with_window(std::forward<Args>(args)...);
}

class window {
public:
  struct width {
    int value;
  };
  struct height {
    int value;
  };
  struct title {
    const char *value;
  };

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

  void make_context_current() { glfwMakeContextCurrent(_window); }
  GLFWframebuffersizefun
  set_framebuffer_size_callback(GLFWframebuffersizefun f) {
    return glfwSetFramebufferSizeCallback(_window, f);
  }

  bool should_close() const {
    return glfwWindowShouldClose(_window) == GLFW_TRUE;
  }

  void should_close(bool b) const {
    glfwSetWindowShouldClose(_window, b ? GLFW_TRUE : GLFW_FALSE);
  }

  input::status get_key(input::key k) const {
    return static_cast<input::status>(glfwGetKey(_window, static_cast<int>(k)));
  }

  void swap_buffers() { glfwSwapBuffers(_window); }

private:
  void _clean() {
    if (_window) {
      glfwDestroyWindow(_window);
    }
  }
  GLFWwindow *_window;
};

template <class F,
          std::enable_if_t<!std::is_same_v<std::invoke_result_t<F &&, window &>,
                                           ExecutionStatus>,
                           int> = 0>
ExecutionStatus with_window(window::width w, window::height h,
                            window::title title, F &&f) {
  GLFWwindow *wptr =
      glfwCreateWindow(w.value, h.value, title.value, NULL, NULL);
  if (!wptr) {
    return ExecutionStatus::Failure;
  }
  window win(wptr);
  std::forward<F>(f)(win);
  return ExecutionStatus::Success;
}

template <class F,
          std::enable_if_t<std::is_same_v<std::invoke_result_t<F &&, window &>,
                                          ExecutionStatus>,
                           int> = 0>
ExecutionStatus with_window(window::width w, window::height h,
                            window::title title, F &&f) {
  GLFWwindow *wptr =
      glfwCreateWindow(w.value, h.value, title.value, NULL, NULL);
  if (!wptr) {
    return ExecutionStatus::Failure;
  }
  window win(wptr);
  return std::forward<F>(f)(win);
}

} // namespace dpsg

// settings
constexpr dpsg::window::width SCR_WIDTH{800};
constexpr dpsg::window::height SCR_HEIGHT{600};

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(dpsg::window &window);

int main() {
  using namespace dpsg;

  auto r = within_glfw_context([]() -> dpsg::ExecutionStatus {
    using wh = window_hint;

    return with_window(
        wh::context_version(3, 3), wh::opengl_profile(profile::core),

#ifdef __APPLE__
        wh::opengl_forward_compat(true),
#endif
        SCR_WIDTH, SCR_HEIGHT, window::title{"LearnOpenGL"},
        [](window &wdw) -> ExecutionStatus {
          // glad: load all OpenGL function pointers
          // ---------------------------------------
          if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return dpsg::ExecutionStatus::Failure;
          }
          // glfw window creation
          // --------------------
          wdw.make_context_current();
          wdw.set_framebuffer_size_callback(framebuffer_size_callback);

          // render loop
          // -----------
          while (!wdw.should_close()) {
            // input
            // -----
            processInput(wdw);

            // render
            // ------
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // glfw: swap buffers and poll IO events (keys
            // pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            wdw.swap_buffers();
            glfwPollEvents();
          }
          return ExecutionStatus::Success;
        });
  });

  return static_cast<int>(r);
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(dpsg::window &window) {
  if (window.get_key(dpsg::input::key::escape) == dpsg::input::status::pressed)
    window.should_close(true);
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