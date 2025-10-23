cmake_minimum_required(VERSION 3.13)
project(tp1-sistemas-distribuidos LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)

# Add all libraries
add_subdirectory(lamport)
add_subdirectory(proto)
add_subdirectory(server)
add_subdirectory(client)
