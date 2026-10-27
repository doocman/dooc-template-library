
#ifndef DTL_CONSTEXPR_EXPRESSION_HPP
#define DTL_CONSTEXPR_EXPRESSION_HPP

#include "dtl/internal/export.hpp"
#include "dtl/internal/std.hpp"

namespace dtl {
#define DTL_CONTRACT_ASSERT(X)                                                 \
  do {                                                                         \
    if (!(X)) {                                                                \
      std::abort();                                                            \
    }                                                                          \
  } while (0)

DTL_EXPORT constexpr auto ascii_to_upper = [](char c) {
  if (c >= 'a' && c <= 'z') {
    return static_cast<char>(c - 'a' + 'A');
  }
  return c;
};
DTL_EXPORT namespace nttp {
  template <typename>
  concept string_expression = true;
  template <typename> using expression_value_t = char;
  template <typename T>
  concept expression_operator =
      std::is_empty_v<T> && std::is_trivially_constructible_v<T> &&
      std::is_trivially_destructible_v<T>;
  template <typename T, typename... Vs>
  concept transform_operator =
      expression_operator<T> && std::invocable<T, Vs...>;
  template <auto> struct constant_t {};
  template <auto v> inline constexpr constant_t<v> constant{};
}
namespace internal {
struct sentinel_for_self_contained_iterators {};

template <nttp::string_expression Str, std::size_t sz, typename F>
class pre_transform_iterator {
  using str_v_t = std::ranges::range_value_t<Str>;
  using cached_t = etd::inplace_vector<str_v_t, sz + 1>;
  std::ranges::iterator_t<Str> underlying_internal_{};
  std::ranges::sentinel_t<Str> underlying_end_internal_{};
  cached_t cached_internal_{};

public:
  using value_type = std::invoke_result_t<F, str_v_t, std::span<str_v_t const>>;
  using reference = value_type const &;
  using difference_type =
      decltype(underlying_end_internal_ - underlying_internal_);
  constexpr pre_transform_iterator() = default;
  constexpr explicit pre_transform_iterator(Str const &str)
      : underlying_internal_(std::ranges::begin(str)),
        underlying_end_internal_(std::ranges::end(str)),
        cached_internal_([this, &str] {
          auto res = cached_t{};
          if (underlying_internal_ != underlying_end_internal_) {
            res.push_back(
                F{}(*underlying_internal_, std::span<str_v_t const>{}));
          }
          return res;
        }()) {}

  constexpr bool operator==(const pre_transform_iterator &other) const {
    return underlying_internal_ == other.underlying_internal_;
  }
  constexpr bool
  operator==(internal::sentinel_for_self_contained_iterators) const {
    return underlying_internal_ == underlying_end_internal_;
  }
  constexpr auto operator<=>(pre_transform_iterator const &other) const {
    return underlying_internal_ <=> other.underlying_internal_;
  }
  friend constexpr auto operator<=>(pre_transform_iterator const &lhs,
                                    sentinel_for_self_contained_iterators) {
    return lhs.underlying_internal_ <=> lhs.underlying_end_internal_;
  }
  friend constexpr auto operator<=>(sentinel_for_self_contained_iterators,
                                    pre_transform_iterator const &rhs) {
    return rhs.underlying_end_internal_ <=> rhs.underlying_internal_;
  }
  friend constexpr difference_type
  operator-(sentinel_for_self_contained_iterators,
            pre_transform_iterator const &rhs) {
    return rhs.underlying_end_internal_ - rhs.underlying_internal_;
  }
  friend constexpr difference_type
  operator-(pre_transform_iterator const &lhs,
            sentinel_for_self_contained_iterators) {
    return lhs.underlying_internal_ - lhs.underlying_end_internal_;
  }
  friend constexpr difference_type
  operator-(pre_transform_iterator const &lhs,
            pre_transform_iterator const &rhs) {
    return lhs.underlying_internal_ - rhs.underlying_internal_;
  }
  constexpr pre_transform_iterator &operator++() {
    ++underlying_internal_;
    if (underlying_internal_ != underlying_end_internal_) {
      if (cached_internal_.capacity() == cached_internal_.size()) {
        cached_internal_.erase(std::ranges::begin(cached_internal_));
      }
      cached_internal_.push_back(F{}(
          *underlying_internal_, std::span(std::as_const(cached_internal_))));
    }
    return *this;
  }
  constexpr pre_transform_iterator operator++(int) {
    auto org = *this;
    ++(*this);
    return org;
  }
  constexpr reference operator*() const {
    assert(*this != sentinel_for_self_contained_iterators{});
    return cached_internal_.back();
  }
};

} // namespace internal
template <typename T, std::size_t sz> using c_array = T[sz];

template <typename R, typename... Ts> struct invocable_helper {
  R operator()(Ts...) const;
};

template <typename Char> struct _peek_as_view_invocable {
  void operator()(std::ranges::range auto &&r) const
    requires(std::same_as<std::ranges::range_value_t<decltype(r)>, Char>);
};

namespace internal {
template <typename T>
concept can_be_safely_removed =
    sizeof(T) == 0 && std::is_trivially_constructible_v<T> &&
    std::is_trivially_destructible_v<T>;
template <typename From, typename To>
concept explicitly_convertible_to = std::constructible_from<From, To>;
template <typename T, typename... Args>
concept implicit_constructible_from =
    std::constructible_from<T, Args...> &&
    requires(Args &&...args) { [](T) {}({std::forward<Args>(args)...}); };

template <template <typename...> typename R, typename... Ts, std::size_t... is>
auto reverse_types_helper(std::index_sequence<is...>) -> std::type_identity<
    R<std::tuple_element_t<sizeof...(Ts) - 1 - is, std::tuple<Ts...>>...>>;
template <template <typename...> typename R, typename... Ts>
using reverse_types = decltype(reverse_types_helper<R, Ts...>(
    std::make_index_sequence<sizeof...(Ts)>()))::type;

template <typename Src, typename Dst>
using _tuple_qual_helper = decltype([] {
  if constexpr (std::is_reference_v<Dst>) {
    return std::type_identity<Dst>();
  } else {
    return std::type_identity<decltype(std::forward_like<Src>(
        std::declval<Dst>()))>();
  }
}())::type;

template <typename...> struct _correct_init_tuple_helper;
template <> struct _correct_init_tuple_helper<> {
  constexpr _correct_init_tuple_helper() noexcept = default;
  template <typename Tup>
  constexpr _correct_init_tuple_helper(
      std::integral_constant<std::size_t, std::size_t{} - 1>,
      Tup const &) noexcept {}
  constexpr bool operator==(const _correct_init_tuple_helper &) const = default;
  constexpr auto
  operator<=>(const _correct_init_tuple_helper &) const = default;
};
template <typename Tup, std::size_t i, typename T, typename MyElement,
          typename Base>
concept _indexed_tuple_ctor_concept =
    i < std::tuple_size_v<Tup> && std::constructible_from<T, MyElement> &&
    std::constructible_from<Base, std::integral_constant<std::size_t, i - 1>,
                            Tup>;
template <typename Tup, std::size_t i>
using _indexed_tuple_ctor_element_t = decltype([] {
  if constexpr (i < std::tuple_size_v<Tup>) {
    return std::type_identity<decltype(std::forward_like<Tup>(
        std::declval<std::tuple_element_t<i, std::remove_cvref_t<Tup>>>()))>();
  } else {
    return std::type_identity<std::nullopt_t>();
  }
}())::type;

template <typename T, typename... Ts>
struct _correct_init_tuple_helper<T, Ts...>
    : _correct_init_tuple_helper<Ts...> {
  T my_value_;

private:
  using _base_t = _correct_init_tuple_helper<Ts...>;
  constexpr _base_t &&as_base() && { return std::move(*this); }
  constexpr _base_t const &&as_base() const && { return std::move(*this); }
  constexpr _base_t &as_base() & { return *this; }
  constexpr _base_t const &as_base() const & { return *this; }

public:
  constexpr _correct_init_tuple_helper() noexcept(
      std::is_nothrow_default_constructible_v<T> &&
      std::is_nothrow_default_constructible_v<_base_t>) = default;

  template <std::size_t i, typename Tup,
            typename MyElement = _indexed_tuple_ctor_element_t<Tup, i>>
  constexpr explicit(
      std::convertible_to<MyElement, T> &&
      implicit_constructible_from<
          _base_t, std::integral_constant<std::size_t, i - 1>, Tup>)
      _correct_init_tuple_helper(
          std::integral_constant<std::size_t, i>,
          Tup &&tup) noexcept(std::is_nothrow_constructible_v<T, MyElement> &&
                              std::is_nothrow_constructible_v<
                                  _base_t,
                                  std::integral_constant<std::size_t, i - 1>,
                                  Tup>)
    requires(_indexed_tuple_ctor_concept<Tup, i, T, MyElement, _base_t>)
      : _base_t(std::integral_constant<std::size_t, i - 1>(),
                std::forward<Tup>(tup)),
        my_value_(std::get<i>(std::forward<Tup>(tup))) {}

  constexpr decltype(auto) do_get(this auto &&self,
                                  std::integral_constant<std::size_t, 0>) {
    return std::forward<decltype(self)>(self).my_value_;
  }
  template <std::size_t i>
    requires(i > 0 && i <= sizeof...(Ts))
  constexpr decltype(auto) do_get(this auto &&self,
                                  std::integral_constant<std::size_t, i>) {
    return std::forward<decltype(self)>(self).as_base().do_get(
        std::integral_constant<std::size_t, i - 1>{});
  }
  constexpr bool operator==(const _correct_init_tuple_helper &) const = default;
  constexpr auto
  operator<=>(const _correct_init_tuple_helper &) const = default;
};

template <typename... Ts>
struct correct_init_tuple
    : private reverse_types<_correct_init_tuple_helper, Ts...> {
private:
  using base_t = reverse_types<_correct_init_tuple_helper, Ts...>;
  constexpr base_t &as_base() noexcept { return *this; }
  constexpr base_t const &as_base() const noexcept { return *this; }
  using ctor_arg_t = std::integral_constant<std::size_t, sizeof...(Ts) - 1>;

public:
  template <std::size_t i>
  constexpr decltype(auto) do_get(this auto &&self,
                                  std::integral_constant<std::size_t, i> c) {
    return std::forward_like<decltype(self)>(self.as_base())
        .do_get(std::integral_constant<std::size_t, sizeof...(Ts) - 1 - i>());
  }
  constexpr decltype(auto)
  do_apply(this auto &&self,
           std::invocable<_tuple_qual_helper<decltype(self), Ts>...> auto &&f) {
    return [&f, &self]<std::size_t... is>(std::index_sequence<is...>) {
      return std::forward<decltype(f)>(f)(
          std::forward<decltype(self)>(self).do_get(
              std::integral_constant<std::size_t, is>())...);
    }(std::make_index_sequence<sizeof...(Ts)>());
  }
  constexpr correct_init_tuple() noexcept(
      (std::is_nothrow_default_constructible_v<Ts> && ...)) = default;
  template <typename... Us>
    requires std::constructible_from<base_t, ctor_arg_t, std::tuple<Us...>>
  constexpr explicit(
      !implicit_constructible_from<base_t, ctor_arg_t, std::tuple<Us...>>)
      correct_init_tuple(Us &&...args) noexcept(
          std::is_nothrow_constructible_v<base_t, ctor_arg_t,
                                          std::tuple<Us...>>)
      : base_t(ctor_arg_t(), std::forward_as_tuple(std::forward<Us>(args)...)) {
  }

  constexpr bool operator==(const correct_init_tuple &) const = default;
  constexpr auto operator<=>(const correct_init_tuple &) const = default;
};
constexpr decltype(auto) apply(auto &&f, auto &&tuple)
  requires(requires() {
    std::forward<decltype(tuple)>(tuple).do_apply(std::forward<decltype(f)>(f));
  })
{
  return std::forward<decltype(tuple)>(tuple).do_apply(
      std::forward<decltype(f)>(f));
}
template <std::size_t i>
constexpr decltype(auto) get(auto &&tuple)
  requires(requires() {
    std::forward<decltype(tuple)>(tuple).do_get(
        std::integral_constant<std::size_t, i>{});
  })
{
  return std::forward<decltype(tuple)>(tuple).do_get(
      std::integral_constant<std::size_t, i>{});
}

} // namespace internal

namespace nttp {
DTL_EXPORT_BEGIN
template <typename T, typename Sort>
concept is_c_array = std::is_bounded_array_v<std::remove_cvref_t<T>> &&
                     std::same_as<std::ranges::range_value_t<T>, Sort>;
template <is_c_array<char> A> constexpr auto to_c_string(A &&a) -> A {
  return std::forward<A>(a);
}
template <typename T>
constexpr decltype(auto) to_c_string(T &&t)
  requires requires() { std::forward<T>(t).to_c_string(); }
{
  return std::forward<T>(t).to_c_string();
}

template <typename T>
concept can_evaluate_to_c_array = is_c_array<T, char> || requires(T const &t) {
  { to_c_string(t) } -> is_c_array<char>;
};
template <can_evaluate_to_c_array T>
using c_string_from_t = decltype(std::declval<T>().to_c_string());
template <std::size_t sz>
struct string : std::ranges::view_interface<string<sz>> {
  char data_internal_[sz];

  constexpr char const *begin() const { return data_internal_; }
  constexpr char const *end() const { return std::end(data_internal_) - 1; }
  constexpr char const *c_str() const { return data_internal_; }

  constexpr explicit(false) string(char const (&string_lit)[sz]) {
    using namespace std::ranges;
    copy(string_lit, std::ranges::begin(data_internal_));
  }

  constexpr bool operator==(string const &other) const noexcept {
    return std::ranges::equal(data_internal_, other.data_internal_);
  }

  constexpr explicit(false) operator std::string_view() const noexcept {
    return {begin(), end()};
  }
  constexpr operator c_array<char, sz> const &() const {
    return data_internal_;
  }
  constexpr c_array<char, sz> const &to_c_string() const {
    return data_internal_;
  }
  constexpr auto format_to(auto dest) const -> decltype(dest) {
    return std::ranges::copy(std::ranges::begin(data_internal_),
                             std::ranges::end(data_internal_) - 1, dest)
        .out;
  }
};
template <typename T>
  requires requires(T const &t) {
    { t.as_view() } -> std::ranges::range;
  }
constexpr decltype(auto) as_view(T const &t) {
  return t.as_view();
}
template <typename T>
concept has_as_view = requires(T const &t) {
  { as_view(t) } -> std::ranges::range;
};
template <typename T> using as_view_t = decltype(as_view(std::declval<T>()));
template <typename T>
constexpr auto peek_as_view(T const &t, auto &&callback)
  requires((std::convertible_to<T const &, std::string_view> &&
            std::invocable<decltype(callback), std::string_view>) ||
           (has_as_view<T> &&
            std::invocable<decltype(callback), as_view_t<T>>) ||
           (can_evaluate_to_c_array<T> &&
            std::invocable<decltype(callback), c_string_from_t<T const &>>) ||
           requires { t.peek_as_view(callback); })
{
  if constexpr (can_evaluate_to_c_array<T const &>) {
    return callback(std::string_view(to_c_string(t)));
  } else if constexpr (std::convertible_to<T const &, std::string_view>) {
    return callback(static_cast<std::string_view>(t));
  } else if constexpr (has_as_view<T>) {
    return callback(as_view(t));
  } else {
    return t.peek_as_view(callback);
  }
}

template <typename T>
concept has_peek_as_view =
    requires(T const &t) { peek_as_view(t, _peek_as_view_invocable<char>{}); };

template <std::size_t... sz>
constexpr auto concat(c_array<char, sz> const &...strs) {
  char raw[(sz + ...) - sizeof...(sz) + 1] = {};
  using namespace std::ranges;
  copy(std::array{std::string_view(strs)...} | views::join, begin(raw));
  return string<size(raw)>(raw);
}
template <std::size_t sep_sz, std::size_t... szs,
          std::size_t result_size =
              (szs + ... + 1) - sizeof...(szs) +
              (sep_sz - 1) * (std::max(sizeof...(szs), std::size_t{1}) - 1)>
constexpr auto concat_with(c_array<char, sep_sz> const &sep,
                           c_array<char, szs> const &...strs)
    -> string<result_size> {
  char raw[result_size] = {};
  using namespace std::ranges;
  auto out = copy(std::array{std::string_view(strs)...} |
                      views::join_with(std::string_view(sep)),
                  begin(raw))
                 .out;
  DTL_CONTRACT_ASSERT(out == end(raw) - 1);
  *out = '\0';
  return raw;
}
DTL_EXPORT_END
template <typename T> constexpr auto _pre_concat_transform(T const &t) {
  if constexpr (requires { string(t); }) {
    return string(t);
  } else if constexpr (is_c_array<T, char>) {
    return string(t);
  } else {
    return t;
  }
}
template <typename T>
using _concat_deduction_helper = decltype([] {
  if constexpr (is_c_array<T, char>) {
    return std::type_identity<decltype(string(T{}))>();
  } else {
    return std::type_identity<T>();
  }
}());
template <typename T>
using _concat_deduction_helper_t = _concat_deduction_helper<T>::type;
DTL_EXPORT_BEGIN
template <has_peek_as_view... Expressions> struct concat_t {
  internal::correct_init_tuple<Expressions...> expressions_;
  constexpr concat_t() = default;
  constexpr concat_t(auto &&...expr)
      : expressions_(std::forward<decltype(expr)>(expr)...) {}

  constexpr bool operator==(const concat_t &) const = default;
  constexpr auto operator<=>(const concat_t &) const = default;

  constexpr auto format_to(std::output_iterator<char> auto dest) const
      -> decltype(dest) {
    if constexpr (sizeof...(Expressions) == 0) {
      return dest;
    } else {
      return apply(
          [dest](auto const &...es) mutable {
            constexpr auto apply_and_move_dest = [](auto &dest,
                                                    auto const &expr) {
              dest = std::format_to(dest, "{}", expr);
              return dest;
            };
            return std::array{apply_and_move_dest(dest, es)...}.back();
          },
          expressions_);
    }
  }
};
template <typename... Ts>
concat_t(Ts &&...)
    -> concat_t<_concat_deduction_helper_t<std::remove_cvref_t<Ts>>...>;
template <has_peek_as_view Sep, has_peek_as_view... Expressions>
struct concat_with_t {
  Sep separator_;
  internal::correct_init_tuple<Expressions...> expressions_;

  constexpr concat_with_t() noexcept(
      std::is_nothrow_default_constructible_v<Sep> &&
      std::is_nothrow_default_constructible_v<decltype(expressions_)>) =
      default;

  template <std::convertible_to<Sep> S, std::convertible_to<Expressions>... Es>
  constexpr explicit(sizeof...(Es) == 0)
      concat_with_t(S &&s, Es &&...expr) noexcept(
          std::is_nothrow_constructible_v<Sep, S> &&
          std::is_nothrow_constructible_v<decltype(expressions_), Es...>)
      : separator_(std::forward<S>(s)),
        expressions_(std::forward<Es>(expr)...) {}

  constexpr bool operator==(const concat_with_t &) const = default;
  constexpr auto operator<=>(const concat_with_t &) const = default;

  constexpr auto format_to(auto dest) const -> decltype(dest) {
    if constexpr (sizeof...(Expressions) == 0) {
      return dest;
    } else {
      dest = std::format_to(dest, "{}", get<0>(expressions_));
      return apply(
          [this, &dest](auto const &,
                        auto const &...rest) mutable -> decltype(dest) {
            using expander = char[sizeof...(rest)];
            auto do_format = [&](auto const &v) {
              dest = std::format_to(dest, "{}{}", separator_, v);
              return '\0';
            };
            (void)expander{do_format(rest...)};
            return dest;
          },
          expressions_);
    }
  }
};
template <has_as_view Sep, has_as_view... Exprs>
constexpr std::ranges::range auto
as_view(concat_with_t<Sep, Exprs...> const &expr) {
  if constexpr (sizeof...(Exprs) == 0) {
    return std::ranges::empty_view<char>{};
  } else {
    return apply(
        [&expr](auto const &str1, auto const &...rest) {
          return std::ranges::concat_view(
              as_view(str1), std::ranges::concat_view(as_view(expr.separator_),
                                                      as_view(rest))...);
        },
        expr.expressions_);
  }
}
// We just pick some arbitrary non-contiguous strings to ensure that the peek:er
// is somewhat generic.
template <typename T>
concept generic_peek_as_view_callback =
    std::invocable<T, std::ranges::empty_view<char const>> &&
    std::invocable<T, std::ranges::concat_view<
                          std::ranges::repeat_view<char, std::ptrdiff_t>,
                          std::string_view>>;
template <has_peek_as_view Expr1, has_peek_as_view... Rest,
          generic_peek_as_view_callback F>
constexpr decltype(auto) peak_multiple_as_view(F &&f, Expr1 const &e1,
                                               Rest const &...rest) {
  if constexpr (sizeof...(Rest) == 0) {
    return peek_as_view(e1, std::forward<F>(f));
  } else {
    return peek_as_view(e1, [&f, &rest...](auto expr1_str) -> decltype(auto) {
      return peak_multiple_as_view(
          [&f, expr1_str](auto... all_views) -> decltype(auto) {
            return f(expr1_str, all_views...);
          },
          rest...);
    });
  }
}
template <has_peek_as_view Sep, has_peek_as_view... Exprs>
constexpr std::ranges::range auto
peek_as_view(concat_with_t<Sep, Exprs...> const &expr,
             std::invocable<std::ranges::single_view<char>> auto &&callback) {
  if constexpr (sizeof...(Exprs) == 0) {
    return std::forward<decltype(callback)>(callback)(
        std::ranges::empty_view<char>());
  } else {
    return peek_as_view(
        get<0>(expr.expressions_),
        [&expr, &callback](auto &&v) -> decltype(auto) {
          if constexpr (sizeof...(Exprs) == 0) {
            return std::forward<decltype(callback)>(callback)(v);
          } else {
            return peak_as_view(
                expr.separator_, [&](auto separator) -> decltype(auto) {
                  return peak_multiple_as_view(
                      [&](auto... rest) -> decltype(auto) {
                        return std::forward<decltype(callback)>(
                            callback)(std::ranges::concat_view(
                            v, std::ranges::concat_view(separator, rest)...));
                      });
                });
          }
        });
  }
}
template <typename Sep, typename... Expr>
concat_with_t(Sep const &, Expr const &...)
    -> concat_with_t<_concat_deduction_helper_t<Sep>,
                     _concat_deduction_helper_t<Expr>...>;

template <can_evaluate_to_c_array... Ex>
constexpr can_evaluate_to_c_array auto concat(Ex const &...ex) {
  return string(concat(to_c_string(ex)...));
}
template <has_peek_as_view... Ex>
  requires(((!can_evaluate_to_c_array<Ex>) || ...))
constexpr auto concat(Ex &&...ex)
    -> decltype(concat_t(_pre_concat_transform(ex)...)) {
  return concat_t(_pre_concat_transform(ex)...);
}
template <can_evaluate_to_c_array Sep, can_evaluate_to_c_array... Ex>
constexpr can_evaluate_to_c_array auto concat_with(Sep const &s,
                                                   Ex const &...ex) {
  return string(concat_with(to_c_string(s), to_c_string(ex)...));
}
template <has_peek_as_view Sep, has_peek_as_view... Expr>
constexpr auto concat_with(Sep const &s, Expr const &...expressions)
    -> decltype(concat_with_t(s, expressions...)) {
  return concat_with_t(s, expressions...);
}

template <typename To, typename... Ts>
concept atleast_one_is_not_convertible_to =
    (!std::convertible_to<Ts, To> || ...);

template <has_peek_as_view L, has_peek_as_view R>
constexpr bool compare_by_peeking_as_string_view(L const &lhs, R const &rhs) {
  return peek_as_view(lhs, [&rhs](std::ranges::range auto &&lhs_s) {
    return peek_as_view(rhs, [lhs_s](std::ranges::range auto &&rhs_s) {
      return std::ranges::equal(lhs_s, rhs_s);
    });
  });
}

template <has_peek_as_view L, has_peek_as_view R>
  requires(atleast_one_is_not_convertible_to<std::string_view, L, R>)
constexpr bool operator==(L const &lhs, R const &rhs) noexcept {
  return compare_by_peeking_as_string_view(lhs, rhs);
}

template <string_expression Str,
          transform_operator<expression_value_t<Str>> Transform>
struct transform {
  Str string_internal_;

  using value_type =
      std::invoke_result_t<Transform, std::ranges::range_value_t<Str>>;

  constexpr transform(std::convertible_to<Str> auto &&s, Transform)
      : string_internal_(std::forward<decltype(s)>(s)) {}

  constexpr auto format_to(std::output_iterator<value_type> auto dest) const
      -> decltype(dest) {
    return std::ranges::transform(string_internal_, dest, Transform{}).out;
  }

  constexpr auto eval() const {
    using namespace std::ranges;
    auto result = std::string{};
    std::ranges::transform(string_internal_, std::back_insert_iterator(result),
                           Transform{});
    return result;
  }
  constexpr std::ranges::view auto as_view() const {
    return std::views::transform(string_internal_, Transform{});
  }
};

template <std::size_t sz, typename Transform>
transform(c_array<char, sz> const &, Transform)
    -> transform<string<sz>, Transform>;

template <std::size_t cache_length, string_expression Str,
          transform_operator<std::ranges::range_value_t<Str>,
                             std::span<std::ranges::range_value_t<Str> const>>
              F>
struct pre_transform
    : std::ranges::view_interface<pre_transform<cache_length, Str, F>> {
  Str string_internal_;

private:
  using str_v_t = std::ranges::range_value_t<Str>;

public:
  using value_type = std::invoke_result_t<F, str_v_t, std::span<str_v_t const>>;
  using iterator = internal::pre_transform_iterator<Str, cache_length, F>;
  using sentinel = internal::sentinel_for_self_contained_iterators;

  template <std::convertible_to<Str> S, std::integral auto v>
    requires(static_cast<std::size_t>(v) == cache_length)
  constexpr pre_transform(S &&s, constant_t<v>, F)
      : string_internal_(std::forward<S>(s)) {}

  [[nodiscard]] constexpr iterator begin() const {
    return iterator(string_internal_);
  }
  [[nodiscard]] constexpr sentinel end() const { return {}; }

  constexpr auto format_to(std::output_iterator<value_type> auto dest) const
      -> decltype(dest) {
    return std::ranges::copy(begin(), end(), dest).out;
  }
};

template <std::size_t sz, typename T, std::integral auto cache_length,
          typename F>
pre_transform(c_array<T, sz> const &, constant_t<cache_length>, F)
    -> pre_transform<cache_length, string<sz>, F>;
template <has_peek_as_view T, std::integral auto cache_length, typename F>
  requires(!std::is_array_v<T>)
pre_transform(T const &, constant_t<cache_length>, F)
    -> pre_transform<cache_length, T, F>;

template <typename> struct as_string;
template <std::integral I> struct as_string<I> {
  I value_;
  static constexpr auto format_buffer_size = [] {
#if __cpp_lib_constexpr_cmath >= 202306L
    if constexpr (std::is_signed_v<I>) {
      return std::lround(
                 std::ceil(std::log10(-std::numeric_limits<I>::min()))) +
             1;
    } else {
      return std::lround(std::ceil(std::log10(std::numeric_limits<I>::max())))
    }
#else
    return sizeof(I) * 3 + (std::is_signed_v<I> ? 1 : 0);
#endif
  }();
  constexpr auto peek_as_view(std::invocable<std::string_view> auto &&s) const
      -> decltype(std::invoke(s, std::string_view{})) {
    c_array<char, format_buffer_size> buffer{};
    auto out = std::to_chars(std::begin(buffer), std::end(buffer), value_);
    return s(std::string_view(std::begin(buffer), out.ptr));
  }
  constexpr auto format_to(std::output_iterator<char> auto dest) const
      -> decltype(dest) {
    return peek_as_view(
        [dest](std::string_view s) { return std::ranges::copy(s, dest).out; });
  }
};
template <std::integral I> as_string(I) -> as_string<I>;
template <std::invocable auto f>
  requires(
      std::ranges::range<std::invoke_result_t<decltype(f)>> &&
      std::same_as<
          char, std::ranges::range_value_t<std::invoke_result_t<decltype(f)>>>)
constexpr has_peek_as_view auto string_from_invocable() {
  auto constexpr sz = std::ranges::size(f());
  if constexpr (sz == 0 || *std::ranges::begin(f()) == '\0') {
    return string("");
  } else {
    auto constexpr arr_size =
        *(std::ranges::end(f()) - 1) == '\0' ? sz : sz + 1;
    char raw[arr_size] = {};
    std::ranges::copy(f(), std::ranges::begin(raw));
    return string(raw);
  }
}
template <std::invocable F>
  requires(
      std::is_empty_v<F> && std::ranges::range<std::invoke_result_t<F>> &&
      std::same_as<char, std::ranges::range_value_t<std::invoke_result_t<F>>>)
constexpr has_peek_as_view auto string_from_invocable(F = {}) {
  return string_from_invocable<F{}>();
}

DTL_EXPORT_END
} // namespace nttp

namespace internal {
struct format_to_formatter {
  template <typename ParseContext>
  constexpr ParseContext::iterator parse(ParseContext &ctx) {
    if (ctx.end() != ctx.begin() && *ctx.begin() != '}') {
      throw std::format_error("invalid format-spec");
    }
    return ctx.begin();
  }
  template <class T, class FmtContext>
  constexpr FmtContext::iterator format(T const &s, FmtContext &ctx) const {
    return s.format_to(ctx.out());
  }
};
} // namespace internal
} // namespace dtl

DTL_EXPORT template <typename T>
struct std::formatter<dtl::nttp::as_string<T>>
    : dtl::internal::format_to_formatter {};
DTL_EXPORT template <std::size_t sz>
struct std::formatter<dtl::nttp::string<sz>>
    : dtl::internal::format_to_formatter {};

DTL_EXPORT template <typename Str, typename F>
struct std::formatter<dtl::nttp::transform<Str, F>>
    : dtl::internal::format_to_formatter {};

DTL_EXPORT template <std::size_t sz, typename Str, typename F>
struct std::formatter<dtl::nttp::pre_transform<sz, Str, F>>
    : dtl::internal::format_to_formatter {};
DTL_EXPORT template <typename... Ts>
struct std::formatter<dtl::nttp::concat_t<Ts...>>
    : dtl::internal::format_to_formatter {};
DTL_EXPORT template <typename... Ts>
struct std::formatter<dtl::nttp::concat_with_t<Ts...>>
    : dtl::internal::format_to_formatter {};

#endif
