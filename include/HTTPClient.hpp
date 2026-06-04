#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <sys/types.h>
#include <cstddef>
#include <functional>
#include <string>
#include "Socket.hpp"

#define SERVICE_HTTP "80"

class HTTPClient {
    std::string _host;
    std::string _path;
    size_t _content_length = 0;
    bool _accepts_ranges = false;
    bool _is_parsed = false;

   public:
    HTTPClient(const std::string& raw_url);
    ssize_t fetch_range_data(size_t start, size_t end, char* buffer, size_t buffer_size,
                             const std::function<void(const char*, size_t)>& write_callback) const;
    decltype(auto) get_content_length() const { return _content_length; }
    decltype(auto) supports_ranges() const { return _accepts_ranges; }
    [[nodiscard]] Socket setup_range_socket(size_t start, size_t end) const;

   private:
    bool parse_url(std::string raw_url);
    bool fetch_metadata();
    std::string get_range_request(const std::string& start_byte, const std::string& end_byte) const;
};

#endif