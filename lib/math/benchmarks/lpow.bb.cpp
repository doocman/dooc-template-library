
#include <dtl/lpow.hpp>

#include <complex>
#include <random>

#include <benchmark/benchmark.h>

namespace dtl {

struct gen_base {
  std::minstd_rand engine{std::random_device{}()};
  std::uniform_real_distribution<double> distribution{-10., 10.};
  double do_get_value() noexcept {
    return distribution(engine);
  }
  double operator()() noexcept {
    return do_get_value();
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
static void BM_pow_complex_with_lpow(benchmark::State& s) {
  auto gen = gen_base{};
  auto v = std::complex{gen(), gen()};
  for (auto _ : s) {
    benchmark::DoNotOptimize(lpow(v, s.range(0)));
  }
}
static void BM_pow_complex_with_std_pow(benchmark::State& s) {
  auto gen = gen_base{};
  auto v = std::complex{gen(), gen()};
  for (auto _ : s) {
    benchmark::DoNotOptimize(std::pow(v, s.range(0)));
  }
}
BENCHMARK(BM_pow_with_lpow)->RangeMultiplier(4)->Range(4ll, 2ll << 50);
BENCHMARK(BM_pow_with_std_pow)->RangeMultiplier(4)->Range(4ll, 2ll << 50);
BENCHMARK(BM_pow_complex_with_lpow)->RangeMultiplier(4)->Range(4ll, 2ll << 50);
BENCHMARK(BM_pow_complex_with_std_pow)->RangeMultiplier(4)->Range(4ll, 2ll << 50);

}
