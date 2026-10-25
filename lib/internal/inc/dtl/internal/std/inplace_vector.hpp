
#ifndef DOOC_TEMPLATE_LIBRARY_DTL_INTERNAL_STD_INPLACE_VECTOR_HPP
#define DOOC_TEMPLATE_LIBRARY_DTL_INTERNAL_STD_INPLACE_VECTOR_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <ranges>

#if __has_attribute(indeterminate)
#define DTL_ATTR_INDETERMINATE [[indeterminate]]
#else
#define DTL_ATTR_INDETERMINATE
#endif

namespace dtl::internal {
struct empty_placeholder {};
template <std::size_t size, std::size_t align> struct aligned_storage {
  alignas(align) std::array<empty_placeholder, size> unused_;
};
constexpr char const *char_ccast(auto const *ptr) {
  return static_cast<char const *>(static_cast<void const *>(ptr));
}
struct mover {
  constexpr auto &&operator()(auto &&v) const {
    return std::move(v); // NOLINT(*-move-forwarding-reference)
  }
};
template <typename T> struct alignas(T) uninitialized_storage {
  union {
    empty_placeholder dummy{};
    T value;
  };
};
template <typename T, std::size_t sz> class inplace_vector;
template <typename T, bool is_const>
class inplace_vector_iterator : public std::contiguous_iterator_tag {
  template <typename T2, std::size_t sz2> friend class inplace_vector;
  template <typename T2, bool c2> friend class inplace_vector_iterator;
  using raw_type = uninitialized_storage<T>;
  using c_pointer = std::conditional_t<is_const, raw_type const *, raw_type *>;

  c_pointer ptr_{};

  constexpr explicit inplace_vector_iterator(c_pointer p) : ptr_(p) {}

public:
  using reference = decltype(std::declval<c_pointer>()->value);
  using pointer = std::add_pointer_t<reference>;
  using difference_type = std::ptrdiff_t;
  constexpr inplace_vector_iterator() = default;

  template <bool c2>
    requires(c2 != is_const && is_const)
  constexpr inplace_vector_iterator(inplace_vector_iterator<T, c2> other)
    requires(is_const)
      : ptr_(other.ptr_) {}

  constexpr inplace_vector_iterator &operator++() {
    ++ptr_;
    return *this;
  }
  constexpr inplace_vector_iterator operator++(int) {
    auto res = *this;
    ++ptr_;
    return res;
  }
  constexpr inplace_vector_iterator &operator--() {
    --ptr_;
    return *this;
  }
  constexpr inplace_vector_iterator operator--(int) {
    auto res = *this;
    --ptr_;
    return res;
  }
  constexpr inplace_vector_iterator &operator+=(difference_type v) {
    ptr_ += v;
    return *this;
  }
  constexpr inplace_vector_iterator &operator-=(difference_type v) {
    ptr_ -= v;
    return *this;
  }
  constexpr reference operator*() const { return ptr_->value; }
  template <typename U, bool c1, bool c2>
  friend constexpr difference_type
  operator-(inplace_vector_iterator<U, c1> lhs,
            inplace_vector_iterator<U, c2> rhs);
};

template <typename U, bool c1, bool c2>
constexpr std::ptrdiff_t operator-(inplace_vector_iterator<U, c1> lhs,
                                   inplace_vector_iterator<U, c2> rhs) {
  return lhs.ptr_ - rhs.ptr_;
}
template <typename T, bool c>
constexpr inplace_vector_iterator<T, c>
operator+(inplace_vector_iterator<T, c> it, std::ptrdiff_t v) {
  it += v;
  return it;
}
template <typename T, bool c>
constexpr inplace_vector_iterator<T, c>
operator-(inplace_vector_iterator<T, c> it, std::ptrdiff_t v) {
  it -= v;
  return it;
}

template <typename T, bool c1, bool c2>
constexpr bool operator==(inplace_vector_iterator<T, c1> const &lhs,
                          inplace_vector_iterator<T, c2> const &rhs) noexcept {
  return std::to_address(lhs) == std::to_address(rhs);
}
template <typename T, bool c1, bool c2>
constexpr bool operator<=>(inplace_vector_iterator<T, c1> const &lhs,
                           inplace_vector_iterator<T, c2> const &rhs) noexcept {
  return std::to_address(lhs) == std::to_address(rhs);
}

template <typename T, std::size_t sz> class inplace_vector {
  using storage_t = aligned_storage<sizeof(T), alignof(T)>;
  alignas(T) std::array<storage_t, sz> raw_storage_{};
  T *current_end_{};

  constexpr void do_cpmv(auto &&other, auto &&element_fetcher) {
    auto dest = raw_storage_.begin();
    auto read_it = std::ranges::begin(other);
    auto new_end = begin();
    for (auto &existing : *this) {
      if (read_it == std::ranges::end(other)) {
        existing.~T();
      } else {
        existing = *read_it;
        ++read_it;
        ++new_end;
        ++dest;
      }
    }
    for (auto const &e :
         std::ranges::subrange(read_it, std::ranges::end(other))) {
      new_end = new (std::to_address(dest)) T(element_fetcher(e)) + 1;
      ++dest;
    }
    assert(char_ccast(new_end) <=
           char_ccast(std::to_address(raw_storage_.end())));
    assert(char_ccast(new_end) >=
               char_ccast(std::to_address(raw_storage_.begin())) ||
           new_end == nullptr);
    current_end_ = new_end;
  }

  constexpr T *get_pointer(T const *ptr) {
    auto diff = current_end_ - ptr;
    return current_end_ - diff;
  }

public:
  // using iterator = inplace_vector_iterator<T, false>;
  // using const_iterator = inplace_vector_iterator<T, true>;

  using reference = T &;
  using const_reference = T const &;
  using pointer = std::add_pointer_t<reference>;
  using const_pointer = std::add_pointer_t<const_reference>;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using size_type = std::size_t;
  using value_type = T;
  constexpr inplace_vector() noexcept = default;
  constexpr inplace_vector(inplace_vector const &other) noexcept(
      std::is_nothrow_copy_constructible_v<T>) {
    do_cpmv(other, std::identity{});
  }
  constexpr inplace_vector(inplace_vector &&other) noexcept(
      std::is_nothrow_move_constructible_v<T>) {
    do_cpmv(other, mover{});
  }
  constexpr inplace_vector(std::initializer_list<T> list) noexcept(
      std::is_nothrow_copy_constructible_v<T>) {
    do_cpmv(list, std::identity{});
  }
  constexpr inplace_vector &operator=(inplace_vector const &other) noexcept(
      std::is_nothrow_copy_constructible_v<T> &&
      std::is_nothrow_copy_assignable_v<T>) {
    if (&other != this) {
      do_cpmv(other, std::identity{});
    }
    return *this;
  }
  constexpr inplace_vector &operator=(inplace_vector &&other) noexcept(
      std::is_nothrow_copy_constructible_v<T> &&
      std::is_nothrow_copy_assignable_v<T>) {
    if (&other != this) {
      do_cpmv(other, mover{});
    }
    return *this;
  }
  constexpr ~inplace_vector() noexcept(std::is_nothrow_destructible_v<T>) {
    for (auto &e : *this | std::views::reverse) {
      e.~T();
    }
  }
  constexpr iterator begin() { return current_end_ - ssize(); }
  constexpr const_iterator cbegin() const { return current_end_ - ssize(); }
  constexpr const_iterator begin() const { return current_end_ - ssize(); }
  constexpr iterator end() { return current_end_; }
  constexpr const_iterator end() const { return current_end_; }
  constexpr const_iterator cend() const { return current_end_; }
  constexpr size_type size() const {
    if (current_end_ == nullptr) {
      return 0;
    }
    auto raw_begin = char_ccast(std::to_address(raw_storage_.begin()));
    auto raw_end = char_ccast(current_end_);
    return (raw_end - raw_begin) / sizeof(T);
  }
  constexpr std::make_signed_t<size_type> ssize() const {
    return static_cast<std::make_signed_t<size_type>>(size());
  }
  static constexpr size_type capacity() { return sz; }
  template <typename... Args>
    requires std::constructible_from<T, Args...>
  constexpr reference emplace_back(Args &&...args) {
    if (size() == capacity()) {
      throw std::bad_alloc();
    }
    auto &dest = raw_storage_[size()];
    T *result = new (&dest) T(std::forward<Args>(args)...);
    current_end_ = result + 1;
    return *result;
  }
  constexpr reference push_back(T &&e) { return emplace_back(std::move(e)); }
  constexpr reference push_back(T const &e) { return emplace_back(e); }
  constexpr reference operator[](std::size_t index) {
    assert(index < size());
    return current_end_[static_cast<std::ptrdiff_t>(index) - ssize()].value;
  }
  constexpr reference back() {
    assert(!empty());
    return (*this)[size() - 1];
  }
  constexpr const_reference operator[](std::size_t index) const {
    assert(index < size());
    return current_end_[static_cast<std::ptrdiff_t>(index) - ssize()];
  }
  constexpr const_reference back() const {
    assert(!empty());
    return (*this)[size() - 1];
  }
  constexpr iterator erase(const_iterator it) {
    auto erase_pointer = get_pointer(it);
    assert(erase_pointer >= begin());
    assert(erase_pointer <= end());
    if (it == cend()) {
      return iterator(erase_pointer);
    }
    current_end_ =
        std::ranges::move(erase_pointer + 1, current_end_, erase_pointer).out;
    return iterator(erase_pointer);
  }
  constexpr pointer data() { return current_end_ - ssize(); }
  constexpr const_pointer data() const { return current_end_ - ssize(); }
  constexpr bool empty() const {
    return char_ccast(current_end_) == char_ccast(raw_storage_.begin()) ||
           current_end_ == nullptr;
  }
};
} // namespace dtl::internal

#endif
