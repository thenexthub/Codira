
# Embedded Codira Unicode library
if(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB)
  add_custom_target(embedded-unicode)
  add_dependencies(embedded-libraries embedded-unicode)

  foreach(entry ${EMBEDDED_STDLIB_TARGET_TRIPLES})
    string(REGEX REPLACE "[ \t]+" ";" list "${entry}")
    list(GET list 0 arch)
    list(GET list 1 mod)
    list(GET list 2 triple)

    if("${mod}" MATCHES "-windows-msvc$")
      continue()
    endif()

    if("${arch}" MATCHES "avr")
      continue()
    endif()

    if (LANGUAGE_HOST_VARIANT STREQUAL "linux")
      set(extra_c_compile_flags -ffreestanding)
    elseif (LANGUAGE_HOST_VARIANT STREQUAL "macosx")
      set(extra_c_compile_flags -ffreestanding)
    endif()
    list(APPEND extra_c_compile_flags -nostdinc++)

    set(LANGUAGE_SDK_embedded_ARCH_${mod}_MODULE "${mod}")
    set(LANGUAGE_SDK_embedded_LIB_SUBDIR "embedded")
    set(LANGUAGE_SDK_embedded_ARCH_${mod}_TRIPLE "${triple}")

    add_language_target_library_single(
      embedded-unicode-${mod}
      languageUnicodeDataTables
      STATIC
      IS_FRAGILE

      UnicodeData.cpp
      UnicodeGrapheme.cpp
      UnicodeNormalization.cpp
      UnicodeScalarProps.cpp
      UnicodeWord.cpp

      C_COMPILE_FLAGS ${extra_c_compile_flags}
      MODULE_DIR "${CMAKE_BINARY_DIR}/lib/language/embedded"
      SDK "embedded"
      ARCHITECTURE "${mod}"
      DEPENDS embedded-stdlib-${mod}
      INSTALL_IN_COMPONENT stdlib
      )
    language_install_in_component(
      TARGETS embedded-unicode-${mod}
      DESTINATION "lib/language/embedded/${mod}"
      COMPONENT "stdlib"
      )
    language_install_in_component(
      FILES "${LANGUAGELIB_DIR}/embedded/${mod}/liblanguageUnicodeDataTables.a"
      DESTINATION "lib/language/embedded/${mod}/"
      COMPONENT "stdlib"
      PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
      )
    set_property(TARGET embedded-unicode-${mod} PROPERTY OSX_ARCHITECTURES "${arch}")

    add_dependencies(embedded-unicode embedded-unicode-${mod})
  endforeach()
endif()
