#pragma once

#include <cstddef>

class ThreadPool {

   public:
    ThreadPool(std::size_t num_threads);
    ~ThreadPool();
};