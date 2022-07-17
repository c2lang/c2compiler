
add_library (git_version MODULE
    git_version.cpp
)

target_link_libraries(git_version common)
set_target_properties(git_version PROPERTIES PREFIX "")
set_target_properties(git_version PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS}")
target_link_options(git_version PRIVATE "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/symbols.expmap")

install(TARGETS git_version DESTINATION ${DESTDIR}/plugins/)
