/*!
 * \file BenchMain.cpp
 * \project BetterQuant
 *
 * \author byrnexu
 * \date 2022/09/08
 *
 * \brief
 */

#include <benchmark/benchmark.h>

class FixtureTest : public benchmark::Fixture {
 public:
  void SetUp(const ::benchmark::State& state) {}
  void TearDown(const ::benchmark::State& state) {}
};

BENCHMARK_DEFINE_F(FixtureTest, test)(benchmark::State& st) {
  for (auto _ : st) {
    int times = st.range(0);
    for (int i = 0; i < times; ++i) {
    }
  }
}
BENCHMARK_REGISTER_F(FixtureTest, test)
    ->Unit(benchmark::kMicrosecond)
    ->Arg(1000);

BENCHMARK_MAIN();
