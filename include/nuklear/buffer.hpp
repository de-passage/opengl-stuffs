#ifndef GUARD_NK_BUFFER_HEADER
#define GUARD_NK_BUFFER_HEADER

#include "./config.hpp"

#include <type_traits>

namespace nk {

struct default_buffer_alloc {
  static inline void alloc(nk_buffer& buffer) noexcept {
    nk_buffer_init_default(&buffer);
  };

  static inline void dealloc(nk_buffer& buffer) noexcept {
    nk_buffer_free(&buffer);
  }
};  // struct default_buffer_alloc

struct fixed_size_buffer_allocator {
  static inline void alloc(nk_buffer& buffer,
                           void* ptr,
                           std::size_t s) noexcept {
    nk_buffer_init_fixed(&buffer, ptr, s);
  }

  static inline void dealloc([[maybe_unused]] nk_buffer& unused) noexcept {}
};  // struct fized_size_buffer_allocator

template <class Alloc>
class buffer : Alloc {
 public:
  template <class... Args>
  explicit buffer(Args&&... args) noexcept(
      noexcept(Alloc::alloc(_buffer, std::forward<Args>(args)...))) {
    Alloc::alloc(_buffer, std::forward<Args>(args)...);
  }

  buffer(const buffer&) = delete;
  buffer(buffer&& buf) noexcept
      : _buffer{std::exchange(buf._buffer, nk_buffer{})} {}
  buffer& operator=(const buffer&) = delete;
  buffer& operator=(buffer&& buf) noexcept {
    buffer{std::move(buf)}.swap(*this);
    return *this;
  }

  inline void swap(buffer& buffer) noexcept {
    nk_buffer = buffer._buffer;
    buffer._buffer = _buffer;
    _buffer = nk_buffer;
  }

  ~buffer() noexcept { Alloc::dealloc(_buffer); }

  [[nodiscard]] const nk_buffer* buf() const noexcept { return _buffer; }
  [[nodiscard]] nk_buffer* buf() noexcept { return &_buffer; }

  void clear() noexcept { nk_buffer_clear(&_buffer); }

 private:
  nk_buffer _buffer;  // NOLINT
};

using dynamic_buffer = buffer<default_buffer_alloc>;
using fixed_size_buffer = buffer<fixed_size_buffer_allocator>;

buffer()->buffer<default_buffer_alloc>;
buffer(void*, std::size_t)->buffer<fixed_size_buffer_allocator>;

}  // namespace nk

#endif