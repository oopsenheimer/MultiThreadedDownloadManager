#include "Socket.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <cstddef>
#include <cstring>

Socket::Socket()
    : _sock_fd(-1), _is_connected(false) {}

Socket::~Socket() {
    if (_sock_fd != -1) {
        close(_sock_fd);
        _is_connected = false;
    }
}

bool Socket::connect_to_host(const std::string& host, const std::string& service) {
    addrinfo config, *server;

    memset(&config, 0, sizeof config);
    config.ai_family = AF_UNSPEC;
    config.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), service.c_str(), &config, &server) != 0) {
        return false;
    }

    _sock_fd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

    if (_sock_fd < 0) {
        return false;
    }

    if (connect(_sock_fd, server->ai_addr, server->ai_addrlen) < 0) {
        freeaddrinfo(server);
        close(_sock_fd);
        _sock_fd = -1;
        return false;
    }
    freeaddrinfo(server);
    return _is_connected = true;
}
bool Socket::send_data(const std::string& data) {
    size_t total_sent = 0;
    while (total_sent < data.length()) {
        ssize_t data_sent = send(_sock_fd, data.c_str() + total_sent, data.length() - total_sent, 0);
        if (data_sent < 0) {
            return false;
        }
        total_sent += data_sent;
    }
    return true;
}
std::string Socket::receive_header() {
    std::string header;
    char chunk[1024];
    while (true) {
        auto bytes_read = recv(_sock_fd, chunk, sizeof(chunk), 0);
        if (bytes_read <= 0) {
            break;
        }
        header.append(chunk, bytes_read);

        auto header_end = header.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            auto header_end_idx = header_end + 4;

            if (header_end_idx < header.size()) {
                _header_overflow_buffer.assign(header.begin() + header_end_idx, header.end());
            }
            header.erase(header_end_idx);
            break;
        }
    }
    return header;
}

ssize_t Socket::receive_data(char* buffer, size_t buffer_size) {
    if (!_header_overflow_buffer.empty()) {
        auto bytes_to_copy = std::min(buffer_size, _header_overflow_buffer.size());
        std::memcpy(buffer, _header_overflow_buffer.data(), bytes_to_copy);
        _header_overflow_buffer.erase(_header_overflow_buffer.begin(),
                                      _header_overflow_buffer.begin() + bytes_to_copy);
        return bytes_to_copy;
    }

    return recv(_sock_fd, buffer, buffer_size, 0);
}