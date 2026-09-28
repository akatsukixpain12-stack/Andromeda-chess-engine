#pragma once
#include "shm.h"
namespace Andromeda {
inline bool shm_create(SharedMemory& m,const char*,std::size_t n){m.size=n;return false;}
inline void shm_close(SharedMemory& m){m.data=nullptr;m.size=0;}
}