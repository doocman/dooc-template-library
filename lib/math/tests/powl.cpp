

#include <concepts>

#include <gmock/gmock.h>

namespace dtl {
using namespace ::testing;

template <typename T>
concept strict_multipliable = requires(T& tr, T const& tc)
{
  {tc * tc} -> std::convertible_to<T>;
  {tr *= tc} -> std::convertible_to<T&>;
};

template <strict_multipliable B, std::integral E>
constexpr B lpow(B base, E const& exp) {
  return {};
}

template <typename T>
struct LPowTests : Test {
  static constexpr decltype(auto) call(auto&& base, auto&& exp) {
    return T::call(base, exp);
  }
};

struct basic_lpow {
  static constexpr decltype(auto) call(auto&& b, auto&& e) {
    return lpow(b, e);
  }
};

using types = Types<basic_lpow>;
TYPED_TEST_SUITE(LPowTests, types);

TYPED_TEST(LPowTests, OnePowOneIsOne) // NOLINT
{
  auto v = this->call(1, 1);
}
}
