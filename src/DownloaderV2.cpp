#include "DownloaderV2.hpp"

#include <curl/urlapi.h>
#include <sys/epoll.h>
#include <sys/types.h>

#include <cerrno>
#include <exception>
#include <memory>
#include <stdexcept>

#include "ThreadPool.hpp"

ssize_t DownloaderV2::download() {
    const auto file_size = _http_client.get_content_length();
    auto num_chunks = 2;

    if (!_http_client.supports_ranges()) {
        std::cout << "[!] SERVER DOES NOT SUPPORTS RANGES\n";
        num_chunks = 1;
    }

    std::cout << "[+] STARTING DOWNLOAD\n    FILE SIZE: " << file_size << '\n';
    ThreadPool pool(num_chunks);
    auto file = prepare_memory_map(file_size);
    auto epoll_fd = epoll_create1(0);

    if (epoll_fd == -1) {
        throw std::runtime_error("[-] EPOLL FAILED");
    }
    std::vector<std::unique_ptr<ChunkContext>> contexts;
    auto chunk_size = file_size / num_chunks;

    ssize_t start_byte = 0;
    for (decltype(num_chunks) i = 0; i < num_chunks; ++i) {
        auto end_byte = (i == num_chunks - 1) ? (file_size - 1) : (start_byte + chunk_size - 1);
        auto ctx = std::make_unique<ChunkContext>();

        ctx->_current_offset = start_byte;
        ctx->_end_byte = end_byte;

        try {
            ctx->_sock = _http_client.setup_range_socket(start_byte, end_byte);
            epoll_event ev;
            ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
            ev.data.ptr = ctx.get();

            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ctx->_sock.get_sock_fd(), &ev) == -1) {
                throw std::runtime_error("[-] EPOLL_CTL_ADD FAILED");
            }
            contexts.push_back(std::move(ctx));
        } catch (const std::exception& e) {
            std::cerr << "[-] Chunk setup failed: " << e.what() << '\n';
        }

        start_byte += chunk_size;
    }

    const int MAX_EVENTS = 16;
    std::array<epoll_event, MAX_EVENTS> incomming_events;

    auto pending_chunks = [&]() {
        int n = 0;
        for (const auto& ctx : contexts) {
            if (!ctx->_finished.load(std::memory_order_acquire)) {
                ++n;
            }
        }
        return n;
    };

    while (pending_chunks() > 0) {
        constexpr int epoll_timeout_ms = 100;
        auto ready_cnt =
            epoll_wait(epoll_fd, incomming_events.data(), MAX_EVENTS, epoll_timeout_ms);
        if (ready_cnt < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (ready_cnt == 0) {
            continue;
        }

        for (int ev = 0; ev < ready_cnt; ++ev) {
            auto* ctx = static_cast<ChunkContext*>(incomming_events[ev].data.ptr);
            pool.enqueue(&DownloaderV2::download_mmap_worker, this, std::ref(file), ctx, epoll_fd);
        }
    }

    std::cout << "[+] DOWNLOAD COMPLETE\n    FILE NAME: " << _file_name << '\n';

    close(epoll_fd);
    return file_size;
}

MemoryMappedFile DownloaderV2::prepare_memory_map(const size_t& file_size) {
    return MemoryMappedFile{_file_name, file_size};
}

void DownloaderV2::download_mmap_worker(MemoryMappedFile& mmap, ChunkContext* ctx, int epoll_fd) {
    if (ctx->_finished.load(std::memory_order_acquire)) {
        return;
    }

    std::array<char, 64 * 1024> scratch_buffer;

    auto finish_chunk = [&]() {
        bool expected = false;
        if (!ctx->_finished.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
            return;
        }
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ctx->_sock.get_sock_fd(), nullptr);
    };

    try {
        while (ctx->_current_offset <= ctx->_end_byte) {
            auto bytes_to_read =
                std::min(sizeof(scratch_buffer), ctx->_end_byte - ctx->_current_offset + 1);

            auto bytes_fetched = ctx->_sock.receive_data(scratch_buffer.data(), bytes_to_read);

            if (bytes_fetched == -2) {
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                ev.data.ptr = ctx;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ctx->_sock.get_sock_fd(), &ev);
                return;
            }

            if (bytes_fetched <= 0) {
                break;
            }

            mmap.write(ctx->_current_offset, scratch_buffer.data(), bytes_fetched);
            ctx->_current_offset += bytes_fetched;
        }

        finish_chunk();
    } catch (const std::exception& e) {
        std::cerr << "[-] Worker exception: " << e.what() << "\n";
        finish_chunk();
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
        init.write("\0", 1);
    }
    return true;
}

void DownloaderV2::download_worker(unsigned int thread_id, size_t start, size_t end) {
    std::fstream file(_file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[-] THREAD " << thread_id << " FAILED TO OPEN FILE\n";
        return;
    }
    file.seekp(start);
    char scratch_buffer[8192 / 2];

    try {
        _http_client.fetch_range_data(start, end, scratch_buffer, sizeof(scratch_buffer),
                                      [&file](const char* chunk_bytes, size_t chunk_size) {
                                          file.write(chunk_bytes, chunk_size);
                                      });

    } catch (const std::exception& e) {
        std::cerr << "[-] Thread " << thread_id << " crashed: " << e.what() << "\n";
    }
}