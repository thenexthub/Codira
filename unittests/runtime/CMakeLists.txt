# We can't currently build the runtime tests under ASAN
# The problem is that we want to use the just\build compiler (just like the
# runtime) but ASAN will complain if we link against libgtest and LLVMSupport
# libraries because they were compiled with the host compiler.
if(("${LANGUAGE_HOST_VARIANT_SDK}" STREQUAL "${LANGUAGE_PRIMARY_VARIANT_SDK}") AND
  ("${LANGUAGE_HOST_VARIANT_ARCH}" STREQUAL "${LANGUAGE_PRIMARY_VARIANT_ARCH}") AND
  (NOT (TOOLCHAIN_USE_SANITIZER STREQUAL "Address")))

  if("${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    if(NOT LANGUAGE_BUILD_RUNTIME_WITH_HOST_COMPILER)
      set(CMAKE_CXX_COMPILER "${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}/clang++")
      set(CMAKE_C_COMPILER "${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}/clang")
    endif()
  elseif(LANGUAGE_BUILD_RUNTIME_WITH_HOST_COMPILER)
    message(FATAL_ERROR "Building the language runtime is not supported with ${CMAKE_C_COMPILER_ID}. Use the just-built clang instead.")
  else()
    message(WARNING "Building the language runtime using the host compiler, and not the just-built clang.")

    # If we use Clang-cl or MSVC, CMake provides default compiler and linker flags that are incompatible
    # with the frontend of Clang or Clang++.
    if(LANGUAGE_COMPILER_IS_MSVC_LIKE)
      set(CMAKE_CXX_COMPILER "${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}/clang-cl")
      set(CMAKE_C_COMPILER "${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}/clang-cl")
    else()
      set(CMAKE_CXX_COMPILER "${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}/clang++")
      set(CMAKE_C_COMPILER "${LANGUAGE_NATIVE_CLANG_TOOLS_PATH}/clang")
    endif()

    if(CMAKE_C_COMPILER_LAUNCHER MATCHES ".*distcc")
      set(CMAKE_C_COMPILER_LAUNCHER "")
    endif()
    if(CMAKE_CXX_COMPILER_LAUNCHER MATCHES ".*distcc")
      set(CMAKE_CXX_COMPILER_LAUNCHER "")
    endif()
  endif()

  add_subdirectory(LongTests)

  set(PLATFORM_SOURCES)
  set(PLATFORM_TARGET_LINK_LIBRARIES)
  if(LANGUAGE_HOST_VARIANT MATCHES "${LANGUAGE_DARWIN_VARIANTS}")
    find_library(FOUNDATION_LIBRARY Foundation)
    list(APPEND PLATFORM_SOURCES
      weak.mm
      Refcounting.mm
      )
    # We need to link languageCore on Darwin because the runtime still relies on
    # some stdlib hooks to implement CodiraObject.
    list(APPEND PLATFORM_TARGET_LINK_LIBRARIES
      ${FOUNDATION_LIBRARY}
      language_Concurrency${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
      languageStdlibUnittest${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
      )
  elseif(LANGUAGE_HOST_VARIANT STREQUAL "Linux")
    if(LANGUAGE_HOST_VARIANT_ARCH MATCHES "armv6|armv7|i686")
      list(APPEND PLATFORM_TARGET_LINK_LIBRARIES
        "atomic")
    endif()
  elseif(LANGUAGE_HOST_VARIANT STREQUAL "freebsd")
    find_library(EXECINFO_LIBRARY execinfo)
    list(APPEND PLATFORM_TARGET_LINK_LIBRARIES
      ${EXECINFO_LIBRARY}
      )
  elseif(LANGUAGE_HOST_VARIANT STREQUAL "windows")
    list(APPEND PLATFORM_TARGET_LINK_LIBRARIES DbgHelp;Synchronization)
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY)
    list(APPEND PLATFORM_SOURCES
      Actor.cpp
      TaskStatus.cpp
      )
    list(APPEND PLATFORM_TARGET_LINK_LIBRARIES
      language_Concurrency${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
      )

    if(NOT "${LANGUAGE_PRIMARY_VARIANT_SDK}" IN_LIST LANGUAGE_DARWIN_PLATFORMS)
      list(APPEND PLATFORM_TARGET_LINK_LIBRARIES
        dispatch${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
        BlocksRuntime${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
        )
    endif()
  endif()

  if(LANGUAGE_ENABLE_EXPERIMENTAL_DISTRIBUTED)
#    list(APPEND PLATFORM_SOURCES
#      DistributedActor.cpp
#      )
    list(APPEND PLATFORM_TARGET_LINK_LIBRARIES
      languageDistributed${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
      )
  endif()

  # Don't complain about these files not being in the sources list.
  set(TOOLCHAIN_OPTIONAL_SOURCES
    weak.mm
    Refcounting.mm
    Actor.cpp
    TaskStatus.cpp)

  add_language_unittest(CodiraRuntimeTests IS_TARGET_TEST
    Array.cpp
    CompatibilityOverrideRuntime.cpp
    CompatibilityOverrideConcurrency.cpp
    Concurrent.cpp
    Metadata.cpp
    Enum.cpp
    ExtendedExistential.cpp
    Heap.cpp
    PrebuiltStringMap.cpp
    Refcounting.cpp
    Stdlib.cpp
    StackAllocator.cpp
    TypeLayoutChecks.cpp
    ${PLATFORM_SOURCES}

    # The runtime tests link to internal runtime symbols, which aren't exported
    # from the languageCore dylib, so we need to link to both the runtime archive
    # and the stdlib.
    $<TARGET_OBJECTS:languageRuntimeCore${LANGUAGE_PRIMARY_VARIANT_SUFFIX}>
    $<TARGET_OBJECTS:languageLLVMSupport${LANGUAGE_PRIMARY_VARIANT_SUFFIX}>
    $<TARGET_OBJECTS:languageDemangling${LANGUAGE_PRIMARY_VARIANT_SUFFIX}>
    )

  # The local stdlib implementation provides definitions of the languageCore
  # interfaces to avoid pulling in languageCore itself.  Build the CodiraRuntimeTests
  # with languageCore_EXPORTS to permit exporting the stdlib interfaces.
  target_compile_definitions(CodiraRuntimeTests
                             PRIVATE
                               languageCore_EXPORTS
                               LANGUAGE_INLINE_NAMESPACE=__runtime)

  if(LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT)
    target_compile_definitions(CodiraRuntimeTests
                               PRIVATE LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT)
  endif()

  target_include_directories(CodiraRuntimeTests BEFORE PRIVATE
    ${LANGUAGE_SOURCE_DIR}/stdlib/include)
  target_include_directories(CodiraRuntimeTests BEFORE PRIVATE
    ${LANGUAGE_SOURCE_DIR}/stdlib/public)

  # FIXME: cross-compile for all variants.
  target_link_libraries(CodiraRuntimeTests
    PRIVATE
    languageCore${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
    languageThreading${LANGUAGE_PRIMARY_VARIANT_SUFFIX}
    ${PLATFORM_TARGET_LINK_LIBRARIES}
    )
endif()
