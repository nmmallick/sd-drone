cmake_minimum_required(VERSION 3.0)
project(sockets VERSION 0.1)

set(SOURCES src/Socket.cc)

add_executable(offboard src/Offboard.cc ${SOURCES})

target_include_directories(offboard PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )

install(TARGETS offboard
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  )
