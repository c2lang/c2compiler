
ADD_DEFINITIONS(-Wall -Wextra -Wshadow -Wno-unused-parameter)
#-D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)

include_directories(${CMAKE_SOURCE_DIR}/c2c)

ADD_EXECUTABLE(c2tags
    c2tags/main.cpp
    c2tags/TagReader.cpp
    ${CMAKE_SOURCE_DIR}/c2c/Builder/RootFinder.cpp
    ${CMAKE_SOURCE_DIR}/c2c/FileUtils/FileMap.cpp
)

ADD_EXECUTABLE(tester
    tester/main.cpp
    ${CMAKE_SOURCE_DIR}/c2c/FileUtils/FileMap.cpp
    ${CMAKE_SOURCE_DIR}/c2c/Utils/StringBuilder.cpp
)

install(TARGETS c2tags DESTINATION ${DESTDIR}/)

