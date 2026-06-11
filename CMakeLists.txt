cmake_minimum_required(VERSION 3.16)

project(HashTableEngine
    VERSION 1.0.0
    DESCRIPTION "Open-addressed C++ hash table with incremental rehashing"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cache_core
    src/cache.cpp
)

target_include_directories(cache_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(cache_core PRIVATE /W4)
else()
    target_compile_options(cache_core PRIVATE -Wall -Wextra -pedantic)
endif()

add_library(naive_cache_core
    src/naive_cache.cpp
)

target_link_libraries(naive_cache_core
    PUBLIC
        cache_core
)

if(MSVC)
    target_compile_options(naive_cache_core PRIVATE /W4)
else()
    target_compile_options(naive_cache_core PRIVATE -Wall -Wextra -pedantic)
endif()

add_executable(cache_tests
    tests/cache_tests.cpp
)

target_link_libraries(cache_tests
    PRIVATE
        cache_core
)

add_executable(naive_cache_tests
    tests/naive_cache_tests.cpp
)

target_link_libraries(naive_cache_tests
    PRIVATE
        naive_cache_core
)

add_executable(benchmark_utils_tests
    tests/benchmark_utils_tests.cpp
)

target_link_libraries(benchmark_utils_tests
    PRIVATE
        cache_core
)

add_executable(randomized_cache_tests
    tests/randomized_cache_tests.cpp
)

target_link_libraries(randomized_cache_tests
    PRIVATE
        cache_core
)

add_executable(cache_stats_tests
    tests/cache_stats_tests.cpp
)

target_link_libraries(cache_stats_tests
    PRIVATE
        cache_core
)

add_executable(cache_demo
    src/demo.cpp
)

target_link_libraries(cache_demo
    PRIVATE
        cache_core
)

add_executable(cache_benchmark
    benchmarks/benchmark.cpp
)

target_link_libraries(cache_benchmark
    PRIVATE
        cache_core
        naive_cache_core
)

enable_testing()

add_test(NAME cache_tests COMMAND cache_tests)
add_test(NAME naive_cache_tests COMMAND naive_cache_tests)
add_test(NAME benchmark_utils_tests COMMAND benchmark_utils_tests)
add_test(NAME randomized_cache_tests COMMAND randomized_cache_tests)
add_test(NAME cache_stats_tests COMMAND cache_stats_tests)
