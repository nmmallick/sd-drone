cmake_minimum_required(VERSION 3.0)
project(sockets VERSION 0.1)

set(SOURCES src/server.cc)
add_executable(main src/server.cc)
