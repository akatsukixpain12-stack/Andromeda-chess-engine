#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <functional>
#include <utility>
#include <vector>
#include "thread_native.h"
namespace Andromeda {
class Thread {
public:
    explicit Thread(unsigned id=0):id_(id){}
    unsigned id() const { return id_; }
    void start(std::function<void()> fn);
    void join();
private:
    unsigned id_;
    std::thread worker_;
};
class ThreadPool {
public:
    unsigned size() const { return static_cast<unsigned>(threads_.size()); }
    void resize(unsigned n);
    void join();
private:
    std::vector<Thread> threads_;
};
}
#endif
