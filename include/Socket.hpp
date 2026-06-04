#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/types.h>
#include <cstddef>
#include <string>

class Socket {
   private:
    int _sock_fd;
    bool _is_connected;
    std::string _header_overflow_buffer;

   public:
    Socket();
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    decltype(_sock_fd) get_sock_fd() const;
    bool connect_to_host(const std::string& host, const std::string& service);
    bool send_data(const std::string& data);
    std::string receive_header();
    ssize_t receive_data(char* buffer, size_t buffer_size);
    [[nodiscard]] bool set_to_non_blocking() const;
};

#endif