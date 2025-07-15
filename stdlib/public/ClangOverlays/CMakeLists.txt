if(NOT DEFINED LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT OR NOT LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT)
  set(BUILTIN_FLOAT_SOURCES
    linker-support/magic-symbols-for-install-name.c
  )

  set(BUILTIN_FLOAT_GYB_SOURCES
    float.code.gyb
  )

  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(BUILTIN_FLOAT_LANGUAGE_FLAGS -Xfrontend -module-abi-name -Xfrontend Darwin)
  else()
    set(BUILTIN_FLOAT_LANGUAGE_FLAGS)
  endif()

  add_language_target_library(language_Builtin_float
      ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES}
      IS_SDK_OVERLAY
      
      ${BUILTIN_FLOAT_SOURCES}

      GYB_SOURCES
        ${BUILTIN_FLOAT_GYB_SOURCES}

      LANGUAGE_COMPILE_FLAGS
        ${LANGUAGE_RUNTIME_LANGUAGE_COMPILE_FLAGS}
        ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
        ${BUILTIN_FLOAT_LANGUAGE_FLAGS}

      LINK_FLAGS
        ${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}

      INSTALL_IN_COMPONENT stdlib
      MACCATALYST_BUILD_FLAVOR zippered)

  # Embedded clang overlays - embedded libraries are built as .codemodule only,
  # i.e. there is no .o or .a file produced (no binary code is actually produced)
  # and only users of a library are going to actually compile any needed code.
  if(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB)
    add_custom_target(embedded-builtin_float)
    add_dependencies(embedded-libraries embedded-builtin_float)
  
    set(LANGUAGE_ENABLE_REFLECTION OFF)
    set(LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT OFF)
    set(LANGUAGE_STDLIB_STABLE_ABI OFF)
    set(LANGUAGE_STDLIB_ENABLE_OBJC_INTEROP OFF)
    set(LANGUAGE_STDLIB_ENABLE_VECTOR_TYPES OFF)

    foreach(entry ${EMBEDDED_STDLIB_TARGET_TRIPLES})
      string(REGEX REPLACE "[ \t]+" ";" list "${entry}")
      list(GET list 0 arch)
      list(GET list 1 mod)
      list(GET list 2 triple)
  
      set(LANGUAGE_SDK_embedded_ARCH_${arch}_MODULE "${mod}")
      set(LANGUAGE_SDK_embedded_LIB_SUBDIR "embedded")
      set(LANGUAGE_SDK_embedded_ARCH_${arch}_TRIPLE "${triple}")
      add_language_target_library_single(
        embedded-builtin_float-${mod}
        language_Builtin_float
        ONLY_LANGUAGEMODULE
        IS_FRAGILE
        
        ${BUILTIN_FLOAT_SOURCES}
        GYB_SOURCES
          ${BUILTIN_FLOAT_GYB_SOURCES}
  
        LANGUAGE_COMPILE_FLAGS
          ${BUILTIN_FLOAT_LANGUAGE_FLAGS}
          -Xcc -ffreestanding -enable-experimental-feature Embedded
  
        MODULE_DIR "${CMAKE_BINARY_DIR}/lib/language/embedded"
        SDK "embedded"
        ARCHITECTURE "${arch}"
        DEPENDS embedded-stdlib-${mod}
        INSTALL_IN_COMPONENT stdlib
        )
      add_dependencies(embedded-builtin_float embedded-builtin_float-${mod})
    endforeach()
  endif()
endif()
