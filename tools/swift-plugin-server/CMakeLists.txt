if (LANGUAGE_BUILD_LANGUAGE_SYNTAX)
  add_pure_language_host_tool(language-plugin-server
    Sources/language-plugin-server/language-plugin-server.code
    LANGUAGE_COMPONENT
      compiler
    LANGUAGE_DEPENDENCIES
      CodiraCompilerPluginMessageHandling
      CodiraLibraryPluginProvider
    PACKAGE_NAME Toolchain
  )

  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LANGUAGE_HOST_LIBRARIES_DEST_DIR}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${LANGUAGE_HOST_LIBRARIES_DEST_DIR}")
  add_pure_language_host_library(CodiraInProcPluginServer SHARED
    Sources/CodiraInProcPluginServer/InProcPluginServer.code
    LANGUAGE_DEPENDENCIES
      CodiraCompilerPluginMessageHandling
      CodiraLibraryPluginProvider
    PACKAGE_NAME Toolchain
  )

  if(LANGUAGE_HOST_VARIANT_SDK IN_LIST LANGUAGE_DARWIN_PLATFORMS)
    set_property(TARGET CodiraInProcPluginServer
      APPEND PROPERTY INSTALL_RPATH
      "@loader_path")
  elseif(LANGUAGE_HOST_VARIANT_SDK MATCHES "LINUX|ANDROID|OPENBSD|FREEBSD")
    set_property(TARGET CodiraInProcPluginServer
      APPEND PROPERTY INSTALL_RPATH
        "$ORIGIN")
    _set_pure_language_link_flags(CodiraInProcPluginServer "../../")
  endif()

  set_property(TARGET ${name}
    PROPERTY BUILD_WITH_INSTALL_RPATH YES)

  add_dependencies(compiler CodiraInProcPluginServer)
  language_install_in_component(TARGETS CodiraInProcPluginServer
    ARCHIVE DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/host" COMPONENT compiler
    LIBRARY DESTINATION "lib${TOOLCHAIN_LIBDIR_SUFFIX}/language/host" COMPONENT compiler
    RUNTIME DESTINATION "bin" COMPONENT compiler)
endif()
