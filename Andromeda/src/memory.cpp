#include "memory.h"
#include <cstdlib>
#if defined(_WIN32)
#include <malloc.h>
#endif
namespace Andromeda {
void* aligned_large_pages_alloc(std::size_t size) {
#if defined(_WIN32)
    return _aligned_malloc(size, 64);
#else
    void* p = nullptr;
    if (posix_memalign(&p, 64, size) != 0) return nullptr;
    return p;
#endif
}
void aligned_large_pages_free(void* ptr) {
#if defined(_WIN32)
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}
}
