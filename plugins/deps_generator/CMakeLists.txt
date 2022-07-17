
add_library (deps_generator MODULE
    DepGenerator.cpp
    DepsPlugin.cpp
)

target_link_libraries(deps_generator common)
set_target_properties(deps_generator PROPERTIES PREFIX "")
set_target_properties(deps_generator PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS}")
target_link_options(deps_generator PRIVATE "LINKER:--version-script=${CMAKE_CURRENT_SOURCE_DIR}/symbols.expmap")

install(TARGETS deps_generator DESTINATION ${DESTDIR}/plugins/)
