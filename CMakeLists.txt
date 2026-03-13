cmake_minimum_required(VERSION 3.10)
project(MultiThreadDownloader VERSION 1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use C++20 (Crucial for modern concurrency)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find libcurl (We need this for HTTP)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

# Include the headers
include_directories(include)

# Define the executable
add_executable(downloader src/main.cpp src/Downloader.cpp)

# Link libcurl and threads
target_link_libraries(downloader CURL::libcurl Threads::Threads)