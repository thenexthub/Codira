
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}")
if (NOT GEN_INCLUDE_DIR)
    message(FATAL_ERROR "GEN_INCLUDE_DIR variable is not defined")
endif()

panda_isa_gen(TEMPLATES profiling_gen.h.erb
    TARGET_NAME profiling_gen
    SOURCE ${CMAKE_CURRENT_LIST_DIR}
    DESTINATION ${GEN_INCLUDE_DIR}
)

declare_plugin_file("read_profile.h")
declare_plugin_file("destroy_profile.h")
declare_plugin_file("find_method_in_profile.h")
declare_plugin_file("dump_profile.h")
declare_plugin_file("clear_profile.h")
declare_plugin_file("profiling_includes.h")
declare_plugin_file("profiling_includes_disasm.h")
declare_plugin_file("get_profiling_any_type.h")