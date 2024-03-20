cmake_minimum_required(VERSION 3.27)
project(snescontrollerdriver)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_executable(snescontrollerdriver src/main.cpp
        src/driver.cpp
        src/driver.hpp
)

target_link_libraries(snescontrollerdriver PRIVATE Threads::Threads)