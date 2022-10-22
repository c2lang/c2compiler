
include_directories(${CMAKE_SOURCE_DIR})

add_library (refs_generator MODULE
    ${CMAKE_SOURCE_DIR}/common/QuickSort.cpp
    ${CMAKE_SOURCE_DIR}/common/Refs.cpp
    TagWriter.cpp
    TagsPlugin.cpp
)

set_target_properties(refs_generator PROPERTIES PREFIX "")
set_target_properties(refs_generator PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS}")
target_link_options(refs_generator PRIVATE "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/symbols.expmap")

install(TARGETS refs_generator DESTINATION ${DESTDIR}/plugins/)
