
set(LLVM_CONFIG_EXECUTABLE "llvm-config")
#set(CMAKE_BUILD_TYPE Release)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
  OUTPUT_VARIABLE LLVM_CXXFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")

#if(NOT DEFINED ENV{C2_SRCDIR})
#    message(FATAL_ERROR "C2_SRCDIR environment variable not set!")
#endif()

# TODO include cmake ROOT/c2c/
include_directories($ENV{C2_SRCDIR})
include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(common)
add_subdirectory(deps_generator)
add_subdirectory(git_version)
add_subdirectory(load_file)
add_subdirectory(refs_generator)
add_subdirectory(shell_cmd)


