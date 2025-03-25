
#include <dtl/lpow.hpp>

#include <complex>

#include <gmock/gmock.h>

namespace dtl {
using namespace ::testing;

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
  EXPECT_THAT(v, Eq(1));
}
TYPED_TEST(LPowTests, OnePowTwoIsOne) // NOLINT
{
  auto v = this->call(1, 2);
  EXPECT_THAT(v, Eq(1));
}
TYPED_TEST(LPowTests, TwoPowZeroIsOne) // NOLINT
{
  auto v = this->call(2, 0);
  EXPECT_THAT(v, Eq(1));
}
TYPED_TEST(LPowTests, ThreePowTwoIsNine) // NOLINT
{
  auto v = this->call(3, 2);
  EXPECT_THAT(v, Eq(9));
}
TYPED_TEST(LPowTests, ThreePowNegThreeIsOneOverTwentyseven) // NOLINT
{
  auto v = this->call(3., -3);
  EXPECT_THAT(v, DoubleEq(1./27.));
}
TYPED_TEST(LPowTests, NoNegativeOverflow) // NOLINT
{
  auto v = this->call(2., std::numeric_limits<std::int_least8_t>::min());
  EXPECT_THAT(v, Eq(std::pow(2., std::numeric_limits<std::int_least8_t>::min())));
}
TYPED_TEST(LPowTests, AccuracyOnePlusEps) // NOLINT
{
  auto input = 1. + std::numeric_limits<double>::epsilon();
  auto v = this->call(input, 1024);
  EXPECT_THAT(v, Eq(std::pow(input, 1024)));
}
TYPED_TEST(LPowTests, AccuracyOneMinusEps) // NOLINT
{
  auto input = 1. - std::numeric_limits<double>::epsilon();
  auto v = this->call(input, 1024);
  EXPECT_THAT(v, Eq(std::pow(input, 1024)));
}
TYPED_TEST(LPowTests, IntegerOneExpNegOne) // NOLINT
{
  auto v = this->call(1, -1);
  EXPECT_THAT(v, Eq(1));
}
TYPED_TEST(LPowTests, ComplexImaginarySquare) // NOLINT
{
  auto v = this->call(std::complex<float>(0.f, 1.f), 2);
  EXPECT_THAT(v.real(), Eq(-1.f));
  EXPECT_THAT(v.imag(), Eq(0.f));
}
TYPED_TEST(LPowTests, ComplexImaginaryCubic) // NOLINT
{
  auto v = this->call(std::complex<float>(0.f, 1.f), 3);
  EXPECT_THAT(v.real(), Eq(0.f));
  EXPECT_THAT(v.imag(), Eq(-1.f));
}
TYPED_TEST(LPowTests, ComplexImaginaryInvCubic) // NOLINT
{
  auto v = this->call(std::complex<float>(0.f, 1.f), -3);
  EXPECT_THAT(v.real(), Eq(0.f));
  EXPECT_THAT(v.imag(), Eq(1.f));
}
}
