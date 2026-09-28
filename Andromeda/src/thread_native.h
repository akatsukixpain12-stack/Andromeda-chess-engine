#ifndef THREAD_NATIVE_H_INCLUDED
#define THREAD_NATIVE_H_INCLUDED
#include <thread>
#include <algorithm>
namespace Andromeda {
inline unsigned native_thread_count(){ return std::max(1u,std::thread::hardware_concurrency()); }
}
#endif
