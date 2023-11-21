cmake_minimum_required(VERSION 3.0)
project(sockets VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests enabled" OFF)

find_package(MAVSDK REQUIRED)
find_package(Threads REQUIRED)

set(SOURCES
  src/System.cc
  src/Socket.cc
  src/Utillities.cc
  src/DroneInterface.cc
  src/NetworkInterface.cc
  )

add_library(offboard ${SOURCES})
target_include_directories(offboard PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_link_libraries(offboard MAVSDK::mavsdk Threads::Threads)

if (BUILD_TESTS)
  add_executable(arm_drone tests/arm_drone.cc)
  target_link_libraries(arm_drone offboard Threads::Threads)

  add_executable(takeoff_land tests/takeoff_land.cc)
  target_link_libraries(takeoff_land offboard Threads::Threads)
endif()



add_executable(main src/Main.cc)

# target_include_directories(offboard PRIVATE
#   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#   $<INSTALL_INTERFACE:include>
#   )

target_link_libraries(main offboard)
install(TARGETS offboard
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  )
