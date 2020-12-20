#ifndef GUARD_NK_FONT_ATLAS_HEADER
#define GUARD_NK_FONT_ATLAS_HEADER

#include "./config.hpp"

#include <type_traits>
namespace nk {

struct default_font_atlas_allocator {
  static inline void alloc(nk_font_atlas* atlas) noexcept {
    nk_font_atlas_init_default(atlas);
  }

  static inline void dealloc(nk_font_atlas* atlas) noexcept {
    nk_font_atlas_clear(atlas);
  }
};  // class default_atlas_allocator

class atlas_interface {
 public:
  inline nk_font* add(const struct nk_font_config& config) {
    return nk_font_atlas_add(_atlas, &config);
  }

  inline nk_font* add_default(
      float height,
      const struct nk_font_config* config = nullptr) noexcept {
    return nk_font_atlas_add_default(_atlas, height, config);
  }

  inline nk_font* add_from_memory(
      void* memory,
      nk_size size,
      float height,
      const struct nk_font_config* config = nullptr) noexcept {
    return nk_font_atlas_add_from_memory(_atlas, memory, size, height, config);
  }

  inline nk_font* add_from_file(
      const char* file_path,
      float height,
      const struct nk_font_config* config = nullptr) noexcept {
    return nk_font_atlas_add_from_file(_atlas, file_path, height, config);
  }

  nk_font* add_compressed(
      void* memory,
      nk_size size,
      float height,
      const struct nk_font_config* config = nullptr) noexcept {
    return nk_font_atlas_add_compressed(_atlas, memory, size, height, config);
  }

  nk_font* add_compressed_base85(
      const char* data,
      float height,
      const struct nk_font_config* config = nullptr) noexcept {
    return nk_font_atlas_add_compressed_base85(_atlas, data, height, config);
  }

  const void* bake(int& width,
                   int& height,
                   enum nk_font_atlas_format format) noexcept {
    return nk_font_atlas_bake(_atlas, &width, &height, format);
  }

 private:
  explicit atlas_interface(nk_font_atlas* atlas) noexcept : _atlas{atlas} {}
  template <class Alloc>
  friend class basic_font_atlas;
  nk_font_atlas* _atlas;
};  // class atlas_interface

template <class Alloc>
class basic_font_atlas : Alloc {
  using Alloc::alloc;
  using Alloc::dealloc;

 public:
  template <class... Args,
            std::enable_if_t<std::is_default_constructible_v<Alloc>, int> = 0>
  explicit basic_font_atlas(Args&&... args) noexcept(
      std::is_nothrow_default_constructible_v<Alloc>&& noexcept(
          alloc(&_atlas, std::forward<Args>(args)...))) {
    alloc(&_atlas, std::forward<Args>(args)...);
  }

  template <class A = Alloc,
            class... Args,
            std::enable_if_t<std::is_constructible_v<Alloc, A>, int> = 0>
  explicit basic_font_atlas(A&& alloc, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<Alloc, A>&& noexcept(
          alloc(&_atlas, std::forward<Args>(args)...)))
      : Alloc(std::forward<A>(alloc)) {
    alloc(&atlas, std::forward<Args>(args)...);
  }

  ~basic_font_atlas() noexcept { dealloc(&_atlas); }

  basic_font_atlas(const basic_font_atlas& other) = delete;
  basic_font_atlas(basic_font_atlas&& other) noexcept(
      std::is_nothrow_move_constructible_v<Alloc>)
      : Alloc(static_cast<Alloc&&>(other)),
        _atlas(std::exchange(other._atlas, nk_font_atlas{})) {}

  basic_font_atlas& operator=(const basic_font_atlas& other) = delete;
  basic_font_atlas& operator=(basic_font_atlas&& other) noexcept(
      std::is_nothrow_move_constructible_v<Alloc>) {
    basic_font_atlas{other}.swap(this);
    return *this;
  }

  void swap(basic_font_atlas& other) noexcept {
    using std::swap;
    swap(_atlas, other._atlas);
  }

  template <class F>
  void with_atlas(nk_draw_null_texture& texture, F&& f) noexcept(
      noexcept(std::forward<F>(f)(atlas_interface{&_atlas}))) {
    static_assert(
        std::is_same_v<std::invoke_result_t<F, atlas_interface&&>, int>,
        "callback must return an int representing the id of the font texture.");
    nk_font_atlas_begin(&_atlas);
    int id{};
    try {
      id = std::forward<F>(f)(atlas_interface{&_atlas});
    }
    catch (...) {
      nk_font_atlas_end(&_atlas, nk_handle_id(0), nullptr);
      throw;
    }
    nk_font_atlas_end(&_atlas, nk_handle_id(id), &texture);
  }

  [[nodiscard]] const nk_font* default_font() const noexcept {
    return _atlas.default_font;
  }

  [[nodiscard]] nk_font* default_font() noexcept { return _atlas.default_font; }

  [[nodiscard]] nk_cursor* cursors() noexcept { return _atlas.cursors; }

  [[nodiscard]] const nk_cursor* cursors() const noexcept {
    return _atlas.cursors;
  }

 private:
  nk_font_atlas _atlas;
};  // class font_atlas

using font_atlas = basic_font_atlas<default_font_atlas_allocator>;

}  // namespace nk

#endif  // GUARD_NK_FONT_ATLAS_HEADER