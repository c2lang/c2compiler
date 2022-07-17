cmake_minimum_required(VERSION 3.1)
project(c2compiler CXX)

if(CYGWIN OR WIN32)
# There is a bug with clang and libstdc++ with the __float128 type which,
# quite frankly, affects C2C compilation, hence why we need to use GCC.
	set(CMAKE_CXX_COMPILER "g++")
else(CYGWIN OR WIN32)
	set(CMAKE_CXX_COMPILER "clang++")
endif(CYGWIN OR WIN32)

set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wno-overloaded-virtual -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(CMAKE_VERBOSE_MAKEFILE false)

add_subdirectory(c2c)
add_subdirectory(tools)
add_subdirectory(plugins)

add_custom_target(tests
    COMMAND ${CMAKE_BINARY_DIR}/tools/tester test
    DEPENDS c2c tester
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

