#include "DownloaderV2.hpp"

ssize_t DownloaderV2::download() {
    const auto file_size = _http_client.get_content_length();
    auto num_thread = std::thread::hardware_concurrency();

    if (!_http_client.supports_ranges()) {
        std::cout << "[!] SERVER DOES NOT SUPPORTS RANGES\n";
        num_thread = 1;
    }

    std::cout << "[+] STARTING DOWNLOAD\n    FILE SIZE: " << file_size << '\n';
    auto file = prepare_memory_map(file_size);

    std::vector<std::thread> workers;
    auto chunk_size = file_size / num_thread;
    size_t start_byte = 0;

    for (decltype(num_thread) i = 0; i < num_thread; ++i) {
        auto end_byte = (i == num_thread - 1) ? (file_size - 1) : (start_byte + chunk_size - 1);
        workers.emplace_back(&DownloaderV2::download_mmap_worker, this, i, std::ref(file),
                             start_byte, end_byte);
        start_byte += chunk_size;
    }

    for (auto& worker : workers) {
        worker.join();
    }

    std::cout << "[+] DOWNLOAD COMPLETE\n    FILE NAME: " << _file_name << '\n';
    return file_size;
}

MemoryMappedFile DownloaderV2::prepare_memory_map(const size_t& file_size) {
    return MemoryMappedFile{_file_name, file_size};
}

void DownloaderV2::download_mmap_worker(unsigned int thread_id, MemoryMappedFile& mmap, size_t start, size_t end) {

    char scratch_buffer[128 * 1024];
    size_t current_offset = start;
    try {
        _http_client.fetch_range_data(
            start, end, scratch_buffer, sizeof(scratch_buffer),
            [&mmap, &current_offset](const char* chunk_bytes, size_t chunk_size) {
                mmap.write(current_offset, chunk_bytes, chunk_size);
                current_offset+= chunk_size;
            });

    } catch (const std::exception& e) {
        std::cerr << "[-] Thread " << thread_id << " crashed: " << e.what() << "\n";
    }
}

bool DownloaderV2::prepare_file_stream(const size_t& file_size) {
    std::ofstream init(_file_name, std::ios::binary | std::ios::trunc);
    if (!init) {
        std::cerr << "FAILED TO CREATE OUTPUT FILE\n";
        return false;
    }
    if (file_size > 0) {
        init.seekp(file_size - 1);
        init.write("\0",1);
    }
    return true;
}

void DownloaderV2::download_worker(unsigned int thread_id ,size_t start, size_t end) {
    std::fstream file(_file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[-] THREAD " << thread_id << " FAILED TO OPEN FILE\n";
        return;
    }
    file.seekp(start);
    char scratch_buffer[8192/2];

    try {
        _http_client.fetch_range_data(start, end, scratch_buffer, sizeof(scratch_buffer), 
            [&file](const char* chunk_bytes, size_t chunk_size) {
                file.write(chunk_bytes, chunk_size);
            });
            
    } catch (const std::exception& e) {
        std::cerr << "[-] Thread " << thread_id << " crashed: " << e.what() << "\n";
    }
}