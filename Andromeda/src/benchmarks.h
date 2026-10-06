#ifndef BENCHMARK_H_INCLUDED
#define BENCHMARK_H_INCLUDED

#include <iosfwd>
#include <string>
#include <vector>

namespace Andromeda::Benchmark {

std::vector<std::string> setup_bench(const std::string&, std::istream&);

struct BenchmarkSetup {
    int                      ttSize;
    int                      threads;
    std::vector<std::string> commands;
    std::string              originalInvocation;
    std::string              filledInvocation;
};

BenchmarkSetup setup_benchmark(std::istream&);

}  // namespace Stockfish::Benchmark

#endif  // #ifndef BENCHMARK_H_INCLUDED
