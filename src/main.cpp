#include <sys/types.h>

#include <chrono>

#include "DownloaderV2.hpp"

constexpr auto DOWNLOAD_URL = "http://david.choffnes.com/classes/cs5700f22/10MB.log";

template <typename Func, typename... Args>
void perf_testing(const std::string& task_name, Func&& download_function, Args&&... args) {
    auto start_time = std::chrono::high_resolution_clock::now();
    auto file_size =
        std::invoke(std::forward<Func>(download_function), std::forward<Args>(args)...);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    auto duration_sec = duration_ms > 0 ? duration_ms / 1000.0 : 0.001;

    double file_size_mb = static_cast<double>(file_size) / (1024.0 * 1024.0);
    double throughput_mb_s = file_size_mb / duration_sec;

    std::cout << "PERF = " << throughput_mb_s << " MB/s\n";
}

int main() {
    DownloaderV2 mydownloader(DOWNLOAD_URL);
    perf_testing("DOWNLOADERV2", &DownloaderV2::download, &mydownloader);
}