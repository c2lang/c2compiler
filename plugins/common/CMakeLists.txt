project(common C)

add_library(common STATIC
    file_utils.c
    plugin_utils.c
    process_utils.c
    yaml_parser.c
)
set_target_properties(common PROPERTIES COMPILE_FLAGS "-fPIC")

