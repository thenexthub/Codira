set(driver_sources_and_options
                driver.cpp
                sil_opt_main.cpp
                sil_func_extractor_main.cpp
                sil_nm_main.cpp
                sil_toolchain_gen_main.cpp
                sil_passpipeline_dumper_main.cpp
                language_dependency_tool_main.cpp
                language_toolchain_opt_main.cpp
                autolink_extract_main.cpp
                modulewrap_main.cpp
                language_api_digester_main.cpp
                language_cache_tool_main.cpp
                language_symbolgraph_extract_main.cpp
                language_synthesize_interface_main.cpp
                language_parse_test_main.cpp)

set(driver_common_libs
                languageAPIDigester
                languageDriver
                languageFrontendTool
                languageSymbolGraphGen
                LLVMBitstreamReader)

add_language_host_library(languageDriverTool STATIC
  ${driver_sources_and_options}
)
target_link_libraries(languageDriverTool
                      PUBLIC
                      ${driver_common_libs})

if (LANGUAGE_BUILD_LANGUAGE_SYNTAX)
  target_link_libraries(languageDriverTool
    PRIVATE
    languageASTGen
  )

  add_dependencies(languageDriverTool
    languageASTGen
  )
endif()

# If building as part of clang, make sure the headers are installed.
if(NOT LANGUAGE_BUILT_STANDALONE)
  add_dependencies(languageDriverTool clang-resource-headers)
endif()

set_language_toolchain_is_available(languageDriverTool)

set(TOOLCHAIN_TARGET_DEFINITIONS CodiraCacheToolOptions.td)
language_tablegen(CodiraCacheToolOptions.inc -gen-opt-parser-defs)
language_add_public_tablegen_target(CodiraCacheToolOptions)

add_dependencies(languageDriverTool CodiraCacheToolOptions)
