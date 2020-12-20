#ifndef GUARD_NK_CONTEXT_HEADER
#define GUARD_NK_CONTEXT_HEADER

#include "./config.hpp"

#include "./buffer.hpp"
#include "./enums.hpp"
#include "./interfaces.hpp"
#include "meta/mixin.hpp"

#include <iterator>
#include <utility>

namespace nk {

namespace detail {
template <class T>
using context_mixin =
    dpsg::mixin<T, input_interface, window_query_interface, self_interface>;
}  // namespace detail

class context : public detail::context_mixin<context> {
 public:
  using rect_t = struct nk_rect;
  using type = nk_context;
  using reference = nk_context&;
  using const_reference = const nk_context&;

  inline explicit context(const nk_user_font* font = nullptr) noexcept {
    nk_init_default(&_ctx, font);
  }

  context(const context&) = delete;
  context& operator=(const context&) = delete;

  inline context(context&& ctx) noexcept : _ctx(std::exchange(ctx._ctx, {})) {}
  inline context& operator=(context&& ctx) noexcept {
    context(std::move(ctx)).swap(*this);
    return *this;
  }

  inline void swap(context& ctx) {
    using std::swap;
    swap(_ctx, ctx._ctx);
  }

  inline ~context() noexcept { nk_free(&_ctx); }

  [[nodiscard]] inline constexpr const_reference ctx() const noexcept {
    return _ctx;
  }

  [[nodiscard]] inline constexpr reference ctx() noexcept { return _ctx; }

  inline void clear() noexcept { nk_clear(&_ctx); }

  inline void input_begin() noexcept { nk_input_begin(&_ctx); }

  inline void input_end() noexcept { nk_input_end(&_ctx); }

  inline void convert(nk_buffer* cmds,
                      nk_buffer* vertices,
                      nk_buffer* elements,
                      nk_convert_config* convert_config) noexcept {
    nk_convert(&_ctx, cmds, vertices, elements, convert_config);
  }

  template <class A1, class A2, class A3>
  inline void convert(buffer<A1>& cmds,
                      buffer<A2>& vertices,
                      buffer<A3>& elements,
                      nk_convert_config* convert_config) noexcept {
    nk_convert(
        &_ctx, cmds.buf(), vertices.buf(), elements.buf(), convert_config);
  }

  template <class F>
  inline void handle_input(F&& f) noexcept(
      noexcept(std::forward<F>(f)(std::declval<input_handler>()))) {
    input_begin();
    try {
      std::forward<F>(f)(input_handler{&_ctx});
    }
    catch (...) {
      input_end();
      throw;
    }
    input_end();
  }

  class const_iterator {
    using difference_type = std::ptrdiff_t;
    using value_type = const nk_command;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

    const_iterator& operator++() noexcept {
      _cmd = nk__next(_ctx, _cmd);
      return *this;
    }

    [[nodiscard]] constexpr inline bool operator==(
        const const_iterator& iter) const noexcept {
      return _cmd == iter._cmd;
    }

    [[nodiscard]] constexpr inline bool operator!=(
        const const_iterator& iter) const noexcept {
      return (*this == iter);
    }

    reference operator*() const noexcept { return *_cmd; }

    pointer operator->() const noexcept { return _cmd; }

   private:
    inline constexpr const_iterator(nk_context* ctx, pointer cmd) noexcept
        : _ctx(ctx), _cmd{cmd} {}

    nk_context* _ctx;
    pointer _cmd;
    friend class context;
  };

  inline const_iterator begin() noexcept {
    const nk_command* cmd = nk__begin(&_ctx);
    return const_iterator{&_ctx, cmd};
  }

  static inline const_iterator end() noexcept {
    return const_iterator{nullptr, nullptr};
  }

  inline bool begin_window(const char* title,
                           rect_t bounds,
                           nk_flags flags) noexcept {
    return nk_begin(&_ctx, title, bounds, flags) == nk_true;
  }

  inline bool begin_window(const char* identifier,
                           const char* title,
                           rect_t bounds,
                           nk_flags flags) noexcept {
    return nk_begin_titled(&_ctx, identifier, title, bounds, flags) == nk_true;
  }

  inline void end_window() noexcept { nk_end(&_ctx); }

  template <class F>
  inline bool with_window(
      const char* title,
      rect_t bounds,
      panel_flags flags,
      F&& f) noexcept(noexcept(std::forward<F>(f)(std::declval<window>()))) {
    if (begin_window(title, bounds, static_cast<nk_flags>(flags))) {
      try {
        std::forward<F>(f)(window{&_ctx});
      }
      catch (...) {
        end_window();
        throw;
      }
      end_window();
      return true;
    }
    return false;
  }

  template <class F>
  inline bool with_window(
      const char* identifier,
      const char* title,
      rect_t bounds,
      panel_flags flags,
      F&& f) noexcept(noexcept(std::forward<F>(f)(std::declval<window>()))) {
    if (begin_window(identifier, title, bounds, static_cast<nk_flags>(flags))) {
      try {
        std::forward<F>(f)(window{&_ctx});
      }
      catch (...) {
        end_window();
        throw;
      }
      end_window();
      return true;
    }
    return false;
  }

  inline nk_window* find_window(const char* identifier) {
    return nk_window_find(&_ctx, identifier);
  }

 private:
  type _ctx{};
};

}  // namespace nk

#endif  // GUARD_NK_CONTEXT_HEADER