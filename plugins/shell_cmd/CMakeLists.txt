
add_library (shell_cmd MODULE
    shell_cmd.cpp
)

target_link_libraries(shell_cmd common)
set_target_properties(shell_cmd PROPERTIES PREFIX "")
set_target_properties(shell_cmd PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS}")
target_link_options(shell_cmd PRIVATE "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/symbols.expmap")

install(TARGETS shell_cmd DESTINATION ${DESTDIR}/plugins/)
