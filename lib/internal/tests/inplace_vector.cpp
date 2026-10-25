
#include "dtl/internal/std/inplace_vector.hpp"

#include "gmock/gmock.h"

namespace dtl {
using namespace internal;
using namespace ::testing;
TEST(InplaceVector, DefaultIsEmpty) {
  auto v = inplace_vector<char, 1>{};
  ASSERT_THAT(v.empty(), IsTrue());
}
TEST(InplaceVector, CopiedDefaultIsEmpty) {
  auto v = inplace_vector<char, 1>{};
  auto vc = v;
  ASSERT_THAT(vc.empty(), IsTrue());
}
TEST(InplaceVector, ConstructFromInitList) {
  auto v = inplace_vector<int, 2>{1};
  EXPECT_THAT(v, ElementsAre(Eq(1)));
}
} // namespace dtl
