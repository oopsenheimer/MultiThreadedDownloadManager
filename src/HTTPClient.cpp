#include "HTTPClient.hpp"
#include <iostream>

HTTPMetadata HTTPClient::get_metadata(const std::string& host, const std::string& path) {
    Socket sock;
    HTTPMetadata meta_data;

    if (!sock.connect_to_host(host, SERVICE_HTTP)) return {};

    std::string request = 
        "GET " + path + " HTTP/1.1\r\n" +
        "Host: " + host + "\r\n" +
        "Range: bytes=0-0\r\n" +       
        "Connection: close\r\n\r\n";

    sock.send_data(request);
    auto headers = sock.receive_header();

    if (headers.empty()) {
        std::cout << "DEBUG: Server closed connection without sending headers." << std::endl;
        return meta_data;
    }

    if (headers.find("200 OK") != std::string::npos || headers.find("206 Partial Content") != std::string::npos) {
        meta_data.status_code = (headers.find("206") != std::string::npos) ? 206 : 200;
        meta_data.accepts_ranges = (meta_data.status_code == 206);
    }

    size_t pos = headers.find("Content-Range: bytes 0-0/");
    if (pos != std::string::npos) {
        meta_data.content_length = std::stoull(headers.substr(pos + 25));
    }

    return meta_data;
}

void HTTPClient::prepare_range_request(Socket& sock, const std::string& host,
                                       const std::string& path, size_t start, size_t end) {
    
}