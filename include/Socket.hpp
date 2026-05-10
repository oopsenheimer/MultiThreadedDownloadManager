#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/types.h>
#include <cstddef>
#include <string>

class Socket {
   private:
    int _sock_fd;
    bool _is_connected;

   public:
    Socket();
    ~Socket();

    bool connect_to_host(const std::string& host, const std::string& port);
    bool send_data(const std::string& data);
    std::string receive_header();

    ssize_t receive_data(char* buffer, size_t buffer_size);

};

#endif