#pragma once
#include <cstddef>
namespace Andromeda {
struct SharedMemory { void* data=nullptr; std::size_t size=0; };
bool shm_create(SharedMemory&, const char*, std::size_t);
void shm_close(SharedMemory&);
}