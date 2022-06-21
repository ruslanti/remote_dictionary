cmake_minimum_required(VERSION 3.22)
project(dictionary)

set(CMAKE_CXX_STANDARD 17)

include_directories("bitsery/include")

add_executable(dictionary src/server.cpp src/connection.cpp)

add_library(client_lib OBJECT src/client.cpp)

add_executable(client src/bench.cpp)
target_link_libraries(client PUBLIC client_lib)