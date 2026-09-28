#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED
#include <cstddef>
namespace Andromeda {
void* aligned_large_pages_alloc(std::size_t size);
void aligned_large_pages_free(void* ptr);
}
#endif
