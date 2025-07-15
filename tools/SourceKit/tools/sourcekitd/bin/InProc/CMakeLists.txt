if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(EXPORTED_SYMBOL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/sourcekitdInProc-darwin.exports")
else()
  set(EXPORTED_SYMBOL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/sourcekitdInProc.exports")
endif()

option(SOURCEKITD_BUILD_STATIC_INPROC
  "Build sourcekitdInProc as a static library (in addition to a shared one)" OFF)

language_is_installing_component(sourcekit-inproc SOURCEKIT_INSTALLING_INPROC)

set(sourcekitdInProc_args
  sourcekitdInProc.cpp
  CodeCompletionCodiraInterop.cpp
  TOOLCHAIN_LINK_COMPONENTS support coverage
)

if (SOURCEKIT_INSTALLING_INPROC)
  if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    add_sourcekit_framework(sourcekitdInProc
      ${SOURCEKITD_SOURCE_DIR}/include/sourcekitd/sourcekitd.h
      ${SOURCEKITD_SOURCE_DIR}/include/sourcekitd/plugin.h
      ${CMAKE_CURRENT_SOURCE_DIR}/CodeCompletionCodiraInterop.h
      ${sourcekitdInProc_args}
      MODULEMAP module.modulemap
      INSTALL_IN_COMPONENT sourcekit-inproc
      HAS_LANGUAGE_MODULES
    )
    set_property(TARGET sourcekitdInProc APPEND_STRING PROPERTY LINK_FLAGS " -fapplication-extension")
    if (SOURCEKIT_DEPLOYMENT_OS MATCHES "^macosx")
      add_definitions(-DSOURCEKIT_VERSIONED_FRAMEWORK_BUNDLE)
    else()
      add_definitions(-DSOURCEKIT_UNVERSIONED_FRAMEWORK_BUNDLE)
    endif()
  else()
    add_sourcekit_library(sourcekitdInProc
      ${sourcekitdInProc_args}
      HEADERS
        ${SOURCEKITD_SOURCE_DIR}/include/sourcekitd/sourcekitd.h
      INSTALL_IN_COMPONENT sourcekit-inproc
      SHARED
      HAS_LANGUAGE_MODULES
    )
  endif()
else()
  add_sourcekit_library(sourcekitdInProc
    ${sourcekitdInProc_args}
    HEADERS
      ${SOURCEKITD_SOURCE_DIR}/include/sourcekitd/sourcekitd.h
    INSTALL_IN_COMPONENT sourcekit-inproc
    SHARED
    HAS_LANGUAGE_MODULES
  )
endif()
set_target_properties(sourcekitdInProc PROPERTIES
  DEFINE_SYMBOL sourcekitd_EXPORTS)
target_link_libraries(sourcekitdInProc PRIVATE
  SourceKitCodiraLang
  sourcekitdAPI
  sourcekitdService
  languageIDETool)

# While it is possible to build this as a static library,
# to get the runtime paths correct, it must be linked into a binary
# that is in the same directory as the language library directory.
if (SOURCEKITD_BUILD_STATIC_INPROC)
  add_sourcekit_library(sourcekitdInProc_Static
    ${sourcekitdInProc_args}
    HEADERS
      ${SOURCEKITD_SOURCE_DIR}/include/sourcekitd/sourcekitd.h
  )
  target_link_libraries(sourcekitdInProc_Static PRIVATE
    SourceKitCodiraLang
    sourcekitdAPI)
endif()

if (SOURCEKIT_BUILT_STANDALONE)
  # Create the symlinks necessary to find the language runtime.
  add_custom_command(TARGET sourcekitdInProc PRE_BUILD
    COMMAND ln "-fhs" "${PATH_TO_LANGUAGE_BUILD}/lib/language" "${SOURCEKIT_BINARY_DIR}/lib")
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  if(LANGUAGE_COMPILER_VERSION)
    set_property(TARGET sourcekitdInProc APPEND_STRING PROPERTY
                 LINK_FLAGS " -Wl,-current_version -Wl,${LANGUAGE_COMPILER_VERSION}")
  endif()
endif()

