

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wno-unused-parameter")

include_directories(${CMAKE_SOURCE_DIR}/c2c)

add_executable(c2tags
    c2tags/main.cpp
    c2tags/TagReader.cpp
    ${CMAKE_SOURCE_DIR}/c2c/Builder/RootFinder.cpp
    ${CMAKE_SOURCE_DIR}/c2c/FileUtils/FileMap.cpp
)

add_executable(tester
    tester/main.cpp
    ${CMAKE_SOURCE_DIR}/c2c/FileUtils/FileMap.cpp
    ${CMAKE_SOURCE_DIR}/c2c/Utils/StringBuilder.cpp
)
target_link_libraries(tester pthread)


install(TARGETS c2tags DESTINATION ${DESTDIR}/)

