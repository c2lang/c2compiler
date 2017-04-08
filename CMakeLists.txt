CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(c2compiler)

if(CYGWIN OR WIN32)
# There is a bug with clang and libstdc++ with the __float128 type which, 
# quite frankly, affects C2C compilation, hence why we need to use GCC.
	SET(CMAKE_CXX_COMPILER "g++") 
else(CYGWIN OR WIN32)
	SET(CMAKE_CXX_COMPILER "clang++")
endif(CYGWIN OR WIN32)

#SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_BUILD_TYPE Debug)
#SET(CMAKE_VERBOSE_MAKEFILE true)
ADD_DEFINITIONS(-Wall)
#ADD_DEFINITIONS(-Wall -Wextra -Wshadow -Wno-overloaded-virtual -Wno-unused-parameter)

SET(CMAKE_VERBOSE_MAKEFILE false)

ADD_SUBDIRECTORY(c2c)
ADD_SUBDIRECTORY(tools)

ADD_CUSTOM_TARGET(tests
    COMMAND ${CMAKE_BINARY_DIR}/tools/tester test
    DEPENDS c2c tester
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

