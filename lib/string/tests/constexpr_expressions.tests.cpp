
#include "gmock/gmock.h"

#ifdef DTL_MODULES
import dtl.string;
#else
#include "dtl/constexpr_expression.hpp"
#endif

namespace dtl {
using namespace ::testing;
static_assert(nttp::string("hello") == nttp::string("hello"));

template <nttp::string>
struct dummy {};
[[maybe_unused]] static constexpr auto dummy_v = dummy<"hello">{};
static_assert(std::ranges::contiguous_range<nttp::string<2>&>);
static_assert(std::ranges::sized_range<nttp::string<2>&>);
static_assert(std::convertible_to<nttp::string<2>&, std::string_view>);
static_assert(nttp::string("hello").operator std::string_view() == std::string_view("hello"));

static_assert(
  nttp::concat("he", "l", "lo") == nttp::string("hello")
  );

TEST(Expression, FormatCtString) {
  auto constexpr expression = nttp::string("This is ok");
  EXPECT_THAT(std::format("{}", expression), StrEq("This is ok"));
}

TEST(Expression, TranformUnderscoreToSpace) {
  auto constexpr expression = nttp::transform("hello_world", [] (char c) {
    if (c == '_') { return ' '; }
    return c;
  });
  EXPECT_THAT(expression.eval(), StrEq("hello world"));
}
TEST(Expression, TransformCanBeFormatted) {
  auto constexpr expression = nttp::transform("hello_world", [] (char c) {
    if (c == '_') { return ' '; }
    return c;
  });
  EXPECT_THAT(std::format("{}", expression), StrEq("hello world"));
}
static_assert(nttp::transform("hi", ascii_to_upper) == nttp::transform("HI", ascii_to_upper));
static_assert(nttp::transform("hi1", ascii_to_upper) != nttp::transform("HI", ascii_to_upper));
static_assert(nttp::has_peek_as_view<nttp::string<1>>);
TEST(Expression, TransformAfterSpace) {
  auto constexpr expression = nttp::pre_transform("hello world", nttp::constant<1>, [] (char c, std::span<char const> prev) {
    if (empty(prev) || prev.back() == ' ') {
      return ascii_to_upper(c);
    }
    return c;
  });
  EXPECT_THAT(std::format("{}", expression), StrEq("Hello World"));
}
TEST(Expression, NumberAsStringFormat) {
  auto constexpr expression = nttp::as_string(44);
  EXPECT_THAT(std::format("{}", expression), StrEq("44"));
}
TEST(Expression, NumberAsStringCompareWithResultingString) {
  auto constexpr expression = nttp::as_string(156);
  EXPECT_THAT(expression, Eq(nttp::string("156")));
  EXPECT_THAT(std::format("{}", expression), StrEq("156"));
}
TEST(Expression, ConcatHeterogeneousStrings) {
  auto constexpr expression = nttp::concat(
    "hello",
    nttp::as_string(5)
    );
  EXPECT_THAT(std::format("{}", expression), StrEq("hello5"));
}
TEST(Expression, ConcatWithSimpleStrings) {
  auto constexpr expression = nttp::concat_with(
    ".-",
    "hello",
    "4"
    );
  EXPECT_THAT(std::format("{}", expression), StrEq("hello.-4"));
}
TEST(Expression, ConcatWithHetereogeneousStrings) {
  auto constexpr expression = nttp::concat_with(
    ".-",
    "hello",
    nttp::as_string(45)
    );
  EXPECT_THAT(std::format("{}", expression), StrEq("hello.-45"));
}
}
