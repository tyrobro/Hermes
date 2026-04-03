cmake_minimum_required(VERSION 3.14)
project(Hermes CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include(FetchContent)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY  https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark internal tests" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable googletest fetch" FORCE)
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "Disable -Werror" FORCE)

FetchContent_MakeAvailable(benchmark)

add_executable(bench_queue benchmarks/bench_queue.cpp)
target_include_directories(bench_queue PRIVATE src)

target_link_libraries(bench_queue PRIVATE benchmark::benchmark benchmark::benchmark_main Threads::Threads)

add_executable(test_affinity benchmarks/test_affinity.cpp)
target_include_directories(test_affinity PRIVATE src)

add_executable(test_engine benchmarks/test_engine.cpp)
target_include_directories(test_engine PRIVATE src)

add_executable(historical_parser benchmarks/historical_parser.cpp)
target_include_directories(historical_parser PRIVATE src)

add_executable(ram_benchmark benchmarks/ram_benchmark.cpp)
target_include_directories(ram_benchmark PRIVATE src)

add_executable(test_queue benchmarks/test_queue.cpp)
target_include_directories(test_queue PRIVATE src)

add_executable(latency_benchmark benchmarks/latency_benchmark.cpp)
target_include_directories(latency_benchmark PRIVATE src)