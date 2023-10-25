cmake_minimum_required(VERSION 3.0)
project(sockets VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MAVSDK REQUIRED)

set(SOURCES
  src/Socket.cc
  src/Utillities.cc
  )

add_executable(offboard src/Offboard.cc ${SOURCES})

target_include_directories(offboard PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

target_link_libraries(offboard MAVSDK::mavsdk)

install(TARGETS offboard
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  )
