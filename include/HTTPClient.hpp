#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include "Socket.hpp"

#define SERVICE_HTTP "80"

struct HTTPMetadata {
    size_t content_length = 0;
    bool accepts_ranges = false;
    int status_code = 0;
};

class HTTPClient {
   public:
    static HTTPMetadata get_metadata(const std::string& host, const std::string& path);

   private:
    static void prepare_range_request(Socket& sock, const std::string& host,
                                      const std::string& path, size_t start, size_t end);
};

#endif