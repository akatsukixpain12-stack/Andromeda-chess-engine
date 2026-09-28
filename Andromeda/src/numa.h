#pragma once
#include <cstddef>
namespace Andromeda {
inline void numa_init() {}
inline void numa_bind_thread(int) {}
inline std::size_t numa_available_memory() { return static_cast<std::size_t>(-1); }
}