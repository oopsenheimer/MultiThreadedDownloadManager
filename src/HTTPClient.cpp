#include "HTTPClient.hpp"

#include <functional>
#include <stdexcept>
#include <string>

#include "Socket.hpp"

HTTPClient::HTTPClient(const std::string& raw_url) {
    if (!parse_url(raw_url)) {
        throw std::runtime_error("URL parsing failed");
    }
    if (!fetch_metadata()) {
        throw std::runtime_error("Metadata fetch failed");
    }
}

ssize_t HTTPClient::fetch_range_data(
    size_t start, size_t end, char* buffer, size_t buffer_size,
    const std::function<void(const char*, size_t)>& write_callback) const {
    Socket sock;
    if (!sock.connect_to_host(_host, SERVICE_HTTP)) {
        return -1;
    }

    if (!sock.send_data(get_range_request(std::to_string(start), std::to_string(end)))) {
        return -1;
    }

    auto headers = sock.receive_header();
    if (headers.empty()) {
        return -1;
    }

    auto total_bytes = end - start + 1;
    decltype(total_bytes) total_received_bytes = 0;

    while (total_received_bytes < total_bytes) {
        auto read_size = std::min(buffer_size, total_bytes - total_received_bytes);
        auto bytes_fetched = sock.receive_data(buffer, read_size);

        if (bytes_fetched < 0) {
            return -1;
        }

        if (bytes_fetched == 0) {
            break;
        }
        write_callback(buffer, bytes_fetched);
        total_received_bytes += bytes_fetched;
    }
    return total_received_bytes;
}

bool HTTPClient::parse_url(std::string raw_url) {
    std::string prefix = "http://";

    if (raw_url.starts_with(prefix)) {
        raw_url.erase(0, prefix.length());
    }

    size_t _slash_pos = raw_url.find('/');

    if (_slash_pos == std::string::npos) {
        _host = raw_url;
        _path = "/";
    } else {
        _host = raw_url.substr(0, _slash_pos);
        _path = raw_url.substr(_slash_pos);
    }

    return _is_parsed = !_host.empty();
}

bool HTTPClient::fetch_metadata() {
    Socket sock;

    if (!sock.connect_to_host(_host, SERVICE_HTTP)) {
        return false;
    }

    if (!sock.send_data(get_range_request("0", "0"))) {
        return false;
    }

    auto headers = sock.receive_header();

    if (headers.empty()) {
        return false;
    }

    if (headers.find("200 OK") != std::string::npos ||
        headers.find("206 Partial Content") != std::string::npos) {
        _accepts_ranges = (headers.find("206") != std::string::npos);
    }

    size_t pos = headers.find("Content-Range: bytes 0-0/");
    if (pos != std::string::npos) {
        _content_length = std::stoull(headers.substr(pos + 25));
    } else {
        size_t len_pos = headers.find("Content-Length: ");
        if (len_pos != std::string::npos) {
            _content_length = std::stoull(headers.substr(len_pos + 16));
        }
    }
    return _content_length > 0;
}

std::string HTTPClient::get_range_request(const std::string& start_byte,
                                          const std::string& end_byte) const {
    return "GET " + _path + " HTTP/1.1\r\n" + "Host: " + _host + "\r\n" +
           "Range: bytes=" + start_byte + "-" + end_byte + "\r\n" + "Connection: close\r\n\r\n";
}