set(datafiles
    _CodiraCxxInteroperability.h
    _CodiraStdlibCxxOverlay.h
    experimental-interoperability-version.json
)

set(LANGUAGELIB_DIR
    "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib/language")
set(output_dir "${LANGUAGELIB_DIR}/languageToCxx")

add_custom_command(
    OUTPUT "${output_dir}"
    COMMAND ${CMAKE_COMMAND} "-E" "make_directory" "${output_dir}")

set (outputs)

foreach(input ${datafiles})
  set(source "${CMAKE_CURRENT_SOURCE_DIR}/${input}")
  set(dest "${output_dir}/${input}")
  add_custom_command(OUTPUT
                       "${output_dir}/${input}"
                     DEPENDS
                       "${CMAKE_CURRENT_SOURCE_DIR}/${input}"
                     COMMAND
                       "${CMAKE_COMMAND}" "-E" "copy" "${source}" "${dest}")
  list(APPEND outputs "${output_dir}/${input}")
endforeach()
list(APPEND outputs "${output_dir}")

add_custom_target("copy_languageToCxx_shims"
    DEPENDS "${output_dir}" "${outputs}"
    COMMENT "Copying Codira to C++ shims to ${output_dir}")

language_install_in_component(FILES ${datafiles}
                           DESTINATION "lib/language/languageToCxx"
                           COMPONENT compiler)

add_language_host_library(languagePrintAsClang STATIC
  ClangSyntaxPrinter.cpp
  DeclAndTypePrinter.cpp
  ModuleContentsWriter.cpp
  PrimitiveTypeMapping.cpp
  PrintAsClang.cpp
  PrintClangClassType.cpp
  PrintClangFunction.cpp
  PrintClangValueType.cpp
  PrintCodiraToClangCoreScaffold.cpp
  CodiraToClangInteropContext.cpp)
target_link_libraries(languagePrintAsClang PRIVATE
  languageAST
  languageClangImporter
  languageFrontend
  languageIDE
  languageIRGen)

add_dependencies(languagePrintAsClang
  "copy_languageToCxx_shims")

set_language_toolchain_is_available(languagePrintAsClang)
