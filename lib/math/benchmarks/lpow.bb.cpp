
#include <dtl/lpow.hpp>

#include <random>

#include <benchmark/benchmark.h>

namespace dtl {

struct gen_base {
  std::minstd_rand engine{std::random_device{}()};
  std::uniform_real_distribution<double> distribution{-10., 10.};
  constexpr double do_get_value() noexcept {
    return distribution(engine);
  }
};

static void BM_pow_with_lpow(benchmark::State& s) {
  auto gen = gen_base{};
  auto v = gen.do_get_value();
  for (auto _ : s) {
    benchmark::DoNotOptimize(lpow(v, s.range(0)));
  }
}
static void BM_pow_with_std_pow(benchmark::State& s) {
  auto gen = gen_base{};
  auto v = gen.do_get_value();
  for (auto _ : s) {
    benchmark::DoNotOptimize(std::pow(v, s.range(0)));
  }
}
BENCHMARK(BM_pow_with_lpow)->RangeMultiplier(3)->Range(3, 3 * lpow(3, 11));
BENCHMARK(BM_pow_with_std_pow)->RangeMultiplier(3)->Range(3, 3 * lpow(3, 11));

}
