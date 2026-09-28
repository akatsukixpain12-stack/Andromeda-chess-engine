#include "thread.h"
namespace Andromeda {
void Thread::start(std::function<void()> fn){ worker_=std::thread(std::move(fn)); }
void Thread::join(){ if(worker_.joinable()) worker_.join(); }
void ThreadPool::resize(unsigned n){ join(); threads_.clear(); threads_.reserve(n); for(unsigned i=0;i<n;++i) threads_.emplace_back(i); }
void ThreadPool::join(){ for(auto& t:threads_) t.join(); }
}
