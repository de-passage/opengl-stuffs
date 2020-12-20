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
  using Alloc::alloc;

 public:
  template <class... Args,
            std::enable_if_t<std::is_default_constructible_v<Alloc>, int> = 0>
  explicit buffer(Args&&... args) noexcept(
      noexcept(alloc(_buffer, std::forward<Args>(args)...))) {
    alloc(_buffer, std::forward<Args>(args)...);
  }

  template <class A = Alloc,
            class... Args,
            std::enable_if_t<std::is_constructible_v<Alloc, A>, int> = 0>
  explicit buffer(A&& alloc, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<Alloc, A>&& noexcept(
          alloc(_buffer, std::forward<Args>(args)...)))
      : Alloc{std::forward<A>(alloc)} {
    alloc(_buffer, std::forward<Args>(args)...);
  }

  buffer(const buffer&) = delete;
  buffer(buffer&& buf) noexcept(std::is_nothrow_move_constructible_v<Alloc>)
      : Alloc(static_cast<Alloc&&>(buf)),
        _buffer{std::exchange(buf._buffer, nk_buffer{})} {}
  buffer& operator=(const buffer&) = delete;
  buffer& operator=(buffer&& buf) noexcept {
    buffer{std::move(buf)}.swap(*this);
    return *this;
  }

  inline void swap(buffer& other) noexcept {
    using std::swap;
    std::swap(_buffer, other._buffer);
  }

  ~buffer() noexcept { Alloc::dealloc(_buffer); }

  [[nodiscard]] const nk_buffer& buf() const noexcept { return _buffer; }
  [[nodiscard]] nk_buffer& buf() noexcept { return _buffer; }

  inline void clear() noexcept { nk_buffer_clear(&_buffer); }
  inline void memory_info(nk_memory_status& info) noexcept {
    nk_buffer_memory_info(&info, &_buffer);
  }

  [[nodiscard]] inline nk_memory_status memory_info() noexcept {
    nk_memory_status st;
    memory_info(st);
    return st;
  }

  inline void push_front(const void* mem,
                         nk_size size,
                         nk_size align) noexcept {
    _push(NK_BUFFER_FRONT, mem, size, align);
  }

  inline void push_back(const void* mem, nk_size size, nk_size align) noexcept {
    _push(NK_BUFFER_BACK, mem, size, align);
  }

  inline void mark_front() noexcept { _mark(NK_BUFFER_FRONT); }

  inline void mark_back() noexcept { _mark(NK_BUFFER_BACK); }

  inline void reset_front() noexcept { _reset(NK_BUFFER_FRONT); }

  inline void reset_back() noexcept { _reset(NK_BUFFER_BACK); }

  [[nodiscard]] inline void* memory() noexcept {
    return nk_buffer_memory(&_buffer);
  }

  [[nodiscard]] inline const void* memory() const noexcept {
    return nk_buffer_memory_const(&_buffer);
  }

  [[nodiscard]] inline nk_size memory_total() noexcept {
    return nk_buffer_memory_total(&_buffer);
  }

 private:
  inline void _push(nk_buffer_allocation_type type,
                    const void* mem,
                    nk_size size,
                    nk_size align) noexcept {
    nk_buffer_push(&_buffer, type, mem, size, align);
  }

  inline void _mark(nk_buffer_allocation_type type) noexcept {
    nk_buffer_mark(&buffer, type);
  }

  inline void _reset(nk_buffer_allocation_type type) noexcept {
    nk_buffer_reset(&buffer, type);
  }

  nk_buffer _buffer;  // NOLINT
};

using dynamic_buffer = buffer<default_buffer_alloc>;
using fixed_size_buffer = buffer<fixed_size_buffer_allocator>;

buffer()->buffer<default_buffer_alloc>;
buffer(void*, std::size_t)->buffer<fixed_size_buffer_allocator>;

}  // namespace nk

#endif