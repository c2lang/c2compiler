
add_library (load_file MODULE
    load_file.cpp
)

target_link_libraries(load_file common)
set_target_properties(load_file PROPERTIES PREFIX "")
set_target_properties(load_file PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS}")
target_link_options(load_file PRIVATE "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/symbols.expmap")

install(TARGETS load_file DESTINATION ${DESTDIR}/plugins/)
