configure_file("${CodiraCore_LANGUAGEC_SOURCE_DIR}/utils/availability-macros.def"
               "${CMAKE_CURRENT_BINARY_DIR}/availability-macros.def"
               COPYONLY)
file(STRINGS "${CMAKE_CURRENT_BINARY_DIR}/availability-macros.def" availability_defs)
list(FILTER availability_defs EXCLUDE REGEX "^\\s*(#.*)?$")
foreach(def ${availability_defs})
  list(APPEND availability_definitions "-Xfrontend -define-availability -Xfrontend \"${def}\"")

  if("${def}" MATCHES "CodiraStdlib .*")
    # For each CodiraStdlib x.y, also define StdlibDeploymentTarget x.y, which,
    # will expand to the current `-target` platform if the macro defines a
    # newer platform as its availability.
    #
    # There is a setting, CodiraCore_ENABLE_STRICT_AVAILABILITY, which if set
    # ON will cause us to use the "proper" availability instead.
    string(REPLACE "CodiraStdlib" "StdlibDeploymentTarget" current "${def}")
    if(NOT CodiraCore_ENABLE_STRICT_AVAILABILITY AND CodiraCore_LANGUAGE_AVAILABILITY_PLATFORM)
      if("${CodiraCore_LANGUAGE_AVAILABILITY_PLATFORM}" STREQUAL "macOS" AND "${CodiraCore_VARIANT_AVAILABILITY_PLATFORM}" STREQUAL "iOS")
        string(REGEX MATCH "iOS ([0-9]+(\.[0-9]+)+)" ios_platform_version "${def}")
        string(REGEX MATCH "[0-9]+(\.[0-9]+)+" ios_version "${ios_platform_version}")
        string(REGEX MATCH "macOS ([0-9]+(\.[0-9]+)+)" macos_platform_version "${def}")
        string(REGEX MATCH "[0-9]+(\.[0-9]+)+" macos_version "${macos_platform_version}")
        if((NOT macos_version STREQUAL "9999" OR NOT ios_version STREQUAL "9999") AND (macos_version VERSION_GREATER CMAKE_OSX_DEPLOYMENT_TARGET OR ios_version VERSION_GREATER CodiraCore_VARIANT_DEPLOYMENT_VERSION))
          string(REGEX REPLACE ":.*" ": macOS ${CMAKE_OSX_DEPLOYMENT_VERSION}, iOS ${CodiraCore_VARIANT_DEPLOYMENT_VERSION}" current "${current}")
        endif()
      else()
        string(REGEX MATCH "${CodiraCore_LANGUAGE_AVAILABILITY_PLATFORM} ([0-9]+(\.[0-9]+)+)" platform_version "${def}")
        string(REGEX MATCH "[0-9]+(\.[0-9]+)+" version "${platform_version}")
        if(NOT version STREQUAL "9999" AND version VERSION_GREATER CMAKE_OSX_DEPLOYMENT_TARGET)
          string(REGEX REPLACE ":.*" ":${CodiraCore_LANGUAGE_AVAILABILITY_PLATFORM} ${CMAKE_OSX_DEPLOYMENT_TARGET}" current "${current}")
        endif()
      endif()
    endif()
    list(APPEND availability_definitions "-Xfrontend -define-availability -Xfrontend \"${current}\"")
  endif()
endforeach()

list(JOIN availability_definitions "\n" availability_definitions)
file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/availability-macros.rsp"
  CONTENT "${availability_definitions}")
add_compile_options("$<$<COMPILE_LANGUAGE:Codira>:SHELL:${CMAKE_Codira_RESPONSE_FILE_FLAG}${CMAKE_CURRENT_BINARY_DIR}/availability-macros.rsp>")
