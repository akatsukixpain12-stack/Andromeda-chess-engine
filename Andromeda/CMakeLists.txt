cmake_minimum_required(VERSION 3.15)
project(Andromeda VERSION 4.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
    src/main.cpp
    src/attacks.cpp
    src/bitboard.cpp
    src/evaluate.cpp
    src/engine.cpp
    src/memory.cpp
    src/misc.cpp
    src/movegen.cpp
    src/movepick.cpp
    src/position.cpp
    src/search.cpp
    src/score.cpp
    src/thread.cpp
    src/timeman.cpp
    src/tt.cpp
    src/uci.cpp
    src/ucioption.cpp
)

add_executable(andromeda ${SOURCE_FILES})

target_compile_options(andromeda PRIVATE
    -O3
    -flto
    -Wall
    -Wextra
    -DNDEBUG
)

find_package(Threads REQUIRED)
target_link_libraries(andromeda PRIVATE Threads::Threads)
