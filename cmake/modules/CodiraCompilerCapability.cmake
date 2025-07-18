# Test if the Codira compiler returns success for supplied compiler arguments....
function(language_supports_compiler_arguments out_var)
  file(WRITE "${CMAKE_BINARY_DIR}/tmp/dummy.code" "")
  execute_process(
    COMMAND "${CMAKE_Codira_COMPILER}" -parse ${ARGN} -
    INPUT_FILE "${CMAKE_BINARY_DIR}/tmp/dummy.code"
    OUTPUT_QUIET ERROR_QUIET
    RESULT_VARIABLE result
  )
  if(NOT result)
    set("${out_var}" "TRUE" PARENT_SCOPE)
  else()
    set("${out_var}" "FALSE" PARENT_SCOPE)
  endif()
endfunction()

# Test if the Codira compiler supports -disable-implicit-<module>-module-import.
macro(language_supports_implicit_module module_name out_var)
  language_supports_compiler_arguments(${out_var}
    -Xfrontend -disable-implicit-${module_name}-module-import
  )
endmacro()

function(language_get_languagelang_version out_var)
  execute_process(
    COMMAND "${CMAKE_Codira_COMPILER}" -version
    OUTPUT_VARIABLE output ERROR_VARIABLE output
    RESULT_VARIABLE result
    TIMEOUT 10
  )

  if(output MATCHES [[languagelang-([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)]])
    set("${out_var}" "${CMAKE_MATCH_1}" PARENT_SCOPE)
  endif()
endfunction()

# Get "package cross-module-optimization" compiler arguments suitable for the compiler.
function(language_get_package_cmo_support out_var)
  # > 6.0 : Fixed feature.
  language_supports_compiler_arguments(result
    -package-name my-package
    -enable-library-evolution
    -Xfrontend -package-cmo
    -Xfrontend -allow-non-resilient-access
  )
  if(result)
    set(${out_var} IMPLEMENTED PARENT_SCOPE)
    return()
  endif()

  # == 6.0 : Experimental.
  language_supports_compiler_arguments(result
    -package-name my-package
    -Xfrontend -experimental-package-cmo
    -Xfrontend -experimental-allow-non-resilient-access
    -Xfrontend -experimental-package-bypass-resilience
  )
  if(result)
    # Package CMO is implmented in Xcode 16 Beta 4 (languagelang-6.0.0.6.8) or later.
    # Consider it's not supported in non Xcode toolchain with "-experimental" options.
    language_get_languagelang_version(languagelang_version)
    if(languagelang_version AND languagelang_version VERSION_GREATER_EQUAL 6.0.0.6)
      set(${out_var} EXPERIMENTAL PARENT_SCOPE)
      return()
    endif()
  endif()

  # < 6.0 : Not supported.
  set(${out_var} NO PARENT_SCOPE)
endfunction()
