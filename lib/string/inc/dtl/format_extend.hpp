
#ifndef DTL_DTL_FORMAT_EXTEND_HPP
#define DTL_DTL_FORMAT_EXTEND_HPP

#include <concepts>
#include <format>
#include <iostream>
#include <ostream>
#include <print>
#include <streambuf>
#include <type_traits>

namespace dtl {
template <typename T, typename Char, typename Traits = std::char_traits<Char>>
concept basic_ostreamable =
    requires(T const &t, std::basic_ostream<Char, Traits> &o) {
      { o << t } -> std::same_as<std::basic_ostream<Char, Traits> &>;
    };

template <typename T>
concept _has_do_safe_copy = requires(T &&t) {
  std::remove_cvref_t<T>::do_safe_copy(std::forward<T>(t));
};
template <typename T>
concept _has_do_unsafe_copy = requires(T &&t) {
  std::remove_cvref_t<T>::do_unsafe_copy(std::forward<T>(t));
};
template <typename T, typename U>
concept _cvref_same_as =
    std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

template <typename T> class stream_formatter {
  T v_;

public:
  template <typename... Args>
    requires(std::constructible_from<T, Args...>)
  constexpr explicit stream_formatter(Args &&...inits) noexcept(
      std::is_reference_v<T> || std::is_nothrow_constructible_v<T, Args...>)
      : v_(std::forward<Args>(inits)...) {}

  // We conditionally disable copy and move semantics for reference typed T:s.
  // The rationale is that non-reference T:s should be safe to copy and move,
  // but references should not be moved out of their constructed context unless
  // the developer (or compiler if we would have lifetime properties) can say
  // that whatever is referenced would still be alive.
  constexpr stream_formatter(stream_formatter &&) noexcept(
      std::is_nothrow_move_constructible_v<T>)
    requires(!std::is_reference_v<T>)
  = default;
  constexpr stream_formatter(stream_formatter const &) noexcept(
      std::is_nothrow_copy_constructible_v<T>)
    requires(!std::is_reference_v<T>)
  = default;
  constexpr stream_formatter &
  operator=(stream_formatter &&) noexcept(std::is_nothrow_move_assignable_v<T>)
    requires(!std::is_reference_v<T>)
  = default;
  constexpr stream_formatter &operator=(stream_formatter const &) noexcept(
      std::is_nothrow_copy_assignable_v<T>)
    requires(!std::is_reference_v<T>)
  = default;
  constexpr std::remove_reference_t<T> const &value() const noexcept {
    return v_;
  }

  // Explicit copy/move function implementations that are enabled even for
  // reference types.
  template <_cvref_same_as<stream_formatter> Self>
  static constexpr stream_formatter<std::remove_cvref_t<T>>
  do_safe_copy(Self &&s) {
    return stream_formatter<std::remove_cvref_t<T>>(std::forward<Self>(s).v_);
  }
  template <_cvref_same_as<stream_formatter> Self>
  static constexpr stream_formatter<T> do_unsafe_copy(Self &&s) {
    return stream_formatter<T>(std::forward<Self>(s).v_);
  }
};
// We may store a reference or a value here.
template <typename T> stream_formatter(T &&) -> stream_formatter<T>;

// Copy functions that developers can use when they know about the lifetime of
// references. Auto-return type as the returned type may differ from the
// original type.
template <_has_do_safe_copy T> constexpr auto safe_copy(T &&t) {
  return std::remove_cvref_t<T>::do_safe_copy(std::forward<T>(t));
}
template <_has_do_unsafe_copy T> constexpr T unsafe_copy(T &&t) {
  return std::remove_cvref_t<T>::do_unsafe_copy(std::forward<T>(t));
}

// this type may need some further 'love', and I think it should be 'exposition
// only' or even just 'implementation details'
template <typename Out>
class _iterator_ostreambuf : public std::basic_streambuf<char> {
  Out o_;

public:
  explicit _iterator_ostreambuf(Out o) : basic_streambuf<char>(), o_(o) {}

  constexpr Out &out() noexcept { return o_; }

protected:
  std::streamsize showmanyc() override {
    return std::numeric_limits<std::streamsize>::max();
  }
  int_type uflow() override { return {}; }
  std::streamsize xsgetn(char_type *, std::streamsize) override { return {}; }
  std::streamsize xsputn(const char_type *s, std::streamsize count) override {
    o_ = std::copy_n(s, count, o_);
    return count;
  }
};

template <typename S, typename... Args> class basic_lazy_format_string;

template <typename Char, typename... Args>
class basic_lazy_format_string<std::basic_format_string<Char, Args...>,
                               Args...> {
  using fmt_t = std::basic_format_string<Char, Args...>;
  fmt_t fmt_;
  std::tuple<Args...> vals_;

  static constexpr bool allow_copy = !(std::is_reference_v<Args> || ...);

public:
  constexpr explicit basic_lazy_format_string(fmt_t fmt_str, Args &&...args)
      : fmt_(fmt_str), vals_(std::forward<Args>(args)...) {}
  constexpr basic_lazy_format_string(fmt_t fmt_str, std::tuple<Args...> &&args)
      : fmt_(fmt_str), vals_(std::move(args)) {}
  constexpr basic_lazy_format_string(fmt_t fmt_str,
                                     std::tuple<Args...> const &args)
      : fmt_(fmt_str), vals_(args) {}

  template <typename Traits>
  friend inline std::basic_ostream<Char, Traits> &
  operator<<(std::basic_ostream<Char, Traits> &o,
             basic_lazy_format_string const &self) {
    std::apply(
        [&o, &self](auto &...vs) {
          std::format_to(std::ostreambuf_iterator<Char>(o), self.fmt_,
                         static_cast<Args>(vs)...);
        },
        self.vals_);
    return o;
  }

  // We conditionally disable copy and move semantics for reference typed T:s.
  // The rationale is that non-reference T:s should be safe to copy and move,
  // but references should not be moved out of their constructed context unless
  // the developer (or compiler if we would have lifetime properties) can say
  // that whatever is referenced would still be alive.
  constexpr basic_lazy_format_string(basic_lazy_format_string &&) noexcept(
      (std::is_nothrow_move_constructible_v<Args> && ...))
    requires(allow_copy)
  = default;
  constexpr basic_lazy_format_string(basic_lazy_format_string const &) noexcept(
      (std::is_nothrow_copy_constructible_v<Args> && ...))
    requires(allow_copy)
  = default;
  constexpr basic_lazy_format_string &
  operator=(basic_lazy_format_string &&) noexcept(
      (std::is_nothrow_move_assignable_v<Args> && ...))
    requires(allow_copy)
  = default;
  constexpr basic_lazy_format_string &
  operator=(basic_lazy_format_string const &) noexcept(
      (std::is_nothrow_copy_assignable_v<Args> && ...))
    requires(allow_copy)
  = default;

  template <_cvref_same_as<basic_lazy_format_string> Self>
  static constexpr basic_lazy_format_string do_unsafe_copy(Self &&s) {
    return basic_lazy_format_string(std::forward<Self>(s).fmt_,
                                    std::forward<Self>(s).vals_);
  }
};

template <typename... Args>
using lazy_format_string =
    basic_lazy_format_string<std::format_string<Args...>, Args...>;

template <typename... Args>
constexpr lazy_format_string<Args...>
lazy_format(std::format_string<Args...> fmt, Args &&...args) {
  return lazy_format_string<Args...>(fmt, std::forward<Args>(args)...);
}

} // namespace dtl

namespace std {
template <typename T, typename Char>
  requires(dtl::basic_ostreamable<T, Char>)
struct formatter<dtl::stream_formatter<T>, Char> {

  template <typename ParseContext>
  constexpr ParseContext::iterator parse(ParseContext &ctx) {
    if (ctx.end() != ctx.begin() && *ctx.begin() != '{') {
      throw std::format_error("invalid format-spec");
    }
    return ctx.begin();
  }
  template <class FmtContext>
  constexpr FmtContext::iterator format(dtl::stream_formatter<T> const &s,
                                        FmtContext &ctx) const {
    auto buf = ::dtl::_iterator_ostreambuf(ctx.out());
    auto stream = std::basic_ostream<char>(&buf);
    stream << s.value();
    return buf.out();
  }
};
} // namespace std

#endif
