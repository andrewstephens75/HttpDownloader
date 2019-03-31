cmake_minimum_required(VERSION 3.00)

add_executable(curlexample src/main.cpp HttpDownloader.cpp)

include_directories(.)

target_link_libraries(curlexample curl)

set_target_properties(curlexample PROPERTIES CXX_STANDARD 17)
