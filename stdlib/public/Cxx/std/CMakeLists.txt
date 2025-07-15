#
# API Notes for the C++ Standard Library
#
set(output_dir "${LANGUAGELIB_DIR}/apinotes")
add_custom_command_target(unused_var
    CUSTOM_TARGET_NAME CxxStdlib-apinotes
    COMMAND ${CMAKE_COMMAND} "-E" "make_directory" "${output_dir}"
    COMMAND ${CMAKE_COMMAND} "-E" "copy_if_different" "${CMAKE_CURRENT_SOURCE_DIR}/std.apinotes" "${output_dir}"
    COMMENT "Copying CxxStdlib API Notes to ${output_dir}"
    OUTPUT "${output_dir}/std.apinotes"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/std.apinotes")

language_install_in_component(FILES std.apinotes
    DESTINATION "lib/language/apinotes"
    COMPONENT compiler)

set_property(TARGET CxxStdlib-apinotes PROPERTY FOLDER "Miscellaneous")
add_dependencies(sdk-overlay CxxStdlib-apinotes)
add_dependencies(compiler CxxStdlib-apinotes)


set(language_cxxstdlib_dependencies)
if(NOT DEFINED LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT OR NOT LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT)
  set(language_cxxstdlib_dependencies _Builtin_float)
endif()

#
# C++ Standard Library Overlay.
#
# The overlay is fragile (i.e. it does not use library evolution)
# as it's not ABI stable.
add_language_target_library(languageCxxStdlib STATIC NO_LINK_NAME IS_STDLIB IS_LANGUAGE_ONLY
    std.code
    Chrono.code
    String.code

    LANGUAGE_MODULE_DEPENDS Cxx ${language_cxxstdlib_dependencies}
    LANGUAGE_MODULE_DEPENDS_ANDROID Android

    LANGUAGE_COMPILE_FLAGS ${LANGUAGE_RUNTIME_LANGUAGE_COMPILE_FLAGS} ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
    -cxx-interoperability-mode=default
    -Xfrontend -module-interface-preserve-types-as-written

    # This flag is unnecessary when building with newer compilers that allow
    # using C++ symbols in resilient overlays (see f4204568).
    -enable-experimental-feature AssumeResilientCxxTypes

    -enable-experimental-feature AllowUnsafeAttribute
    -strict-memory-safety

    # The varying modularization of the C++ standard library on different
    # platforms makes it difficult to enable MemberImportVisibility for this
    # module
    -disable-upcoming-feature MemberImportVisibility

    LANGUAGE_COMPILE_FLAGS_LINUX
    ${LANGUAGE_SDK_LINUX_CXX_OVERLAY_LANGUAGE_COMPILE_FLAGS}

    DEPLOYMENT_VERSION_OSX ${COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_OSX}
    DEPLOYMENT_VERSION_IOS ${COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_IOS}
    DEPLOYMENT_VERSION_TVOS ${COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_TVOS}
    DEPLOYMENT_VERSION_WATCHOS ${COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_WATCHOS}
    DEPLOYMENT_VERSION_XROS ${COMPATIBILITY_MINIMUM_DEPLOYMENT_VERSION_XROS}

    LINK_FLAGS "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}"
    TARGET_SDKS ALL_APPLE_PLATFORMS LINUX LINUX_STATIC WINDOWS ANDROID FREEBSD
    MACCATALYST_BUILD_FLAVOR zippered
    INSTALL_IN_COMPONENT compiler
    INSTALL_BINARY_LANGUAGEMODULE NON_DARWIN_ONLY
    INSTALL_WITH_SHARED
    DEPENDS libstdcxx-modulemap libcxxshim_modulemap CxxStdlib-apinotes)
