#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

class ThreadPool {
   private:
    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> _tasks;
    std::mutex _queue_mutex;
    std::condition_variable _cv;
    bool _stop = false;

   public:
    explicit ThreadPool(size_t num_threads) {
        for (decltype(num_threads) i = 0; i < num_threads; ++i) {
            _workers.emplace_back(&ThreadPool::worker_loop, this);
        }
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(_queue_mutex);
            _stop = true;
        }
        _cv.notify_all();

        for (auto& worker : _workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    template <typename Func, typename... Args>
    decltype(auto) enqueue(Func&& func, Args&&... args) {
        using return_type = typename std::invoke_result<Func, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::lock_guard<std::mutex> lock(_queue_mutex);

            if (_stop) {
                throw std::runtime_error("Enqueue Stopped");
            }

            _tasks.emplace([task]() { (*task)(); });
        }
        _cv.notify_one();
        return res;
    }

   private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(_queue_mutex);

                _cv.wait(lock, [this]() { return _stop || !_tasks.empty(); });

                if (_stop && _tasks.empty()) {
                    return;
                }

                task = std::move(_tasks.front());
                _tasks.pop();
            }
            task();
        }
    }
};

#endif