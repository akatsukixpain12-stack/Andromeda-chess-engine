cmake_minimum_required(VERSION 3.15)
project(Andromeda VERSION 3.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/attacks.cpp
    src/bitboard.cpp
    src/evaluate.cpp
    src/movegen.cpp
    src/movepick.cpp
    src/position.cpp
    src/search.cpp
    src/timeman.cpp
    src/tt.cpp
    src/uci.cpp
)

add_executable(andromeda ${SOURCES})

target_compile_options(andromeda PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-O3 -flto -march=native -Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/O2 /W4>
)

find_package(Threads REQUIRED)
target_link_libraries(andromeda PRIVATE Threads::Threads)
