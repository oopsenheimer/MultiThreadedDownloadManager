// #include "ThreadPool.hpp"

// ThreadPool::ThreadPool(size_t num_threads) {
//     for (size_t i = 0; i < num_threads; i++) {
//         _workers.emplace_back(&ThreadPool::worker_loop, this);
//     }
// }

// ThreadPool::~ThreadPool() {
//     {
//         std::lock_guard<std::mutex> lock(_queue_mutex);
//         _stop  = true;
//     }

//     _cv.notify_all();

//     for (auto& worker : _workers) {
//         if (worker.joinable()) {
//             worker.join();
//         }
//     }
// }

// void ThreadPool::enqueue(std::function<void()> task) {
//     {
//         std::lock_guard<std::mutex> lock(_queue_mutex);
//         _tasks.push(task);
//     }
//     _cv.notify_one();
// }

// void ThreadPool::worker_loop() {
//     while (true) {
//         std::function<void()> task;
//         {
//             std::unique_lock<std::mutex> lock(_queue_mutex);

//             _cv.wait(lock, [this]() { return _stop || !_tasks.empty(); });

//             if (_stop && _tasks.empty()) {
//                 return;
//             }

//             task = std::move(_tasks.front());
//             _tasks.pop();
//         }
//         task();
//     }
// }