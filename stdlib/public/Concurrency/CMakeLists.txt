#===--- CMakeLists.txt - Concurrency support library ---------------------===#
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2019 - 2020 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
#
#===----------------------------------------------------------------------===#

set(LANGUAGE_RUNTIME_CONCURRENCY_C_FLAGS)
set(LANGUAGE_RUNTIME_CONCURRENCY_LANGUAGE_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/InternalShims)

set(language_concurrency_private_link_libraries)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  list(APPEND language_concurrency_private_link_libraries
    Synchronization
    mincore.lib # For QueryInterruptTime()
  )
endif()

set(language_concurrency_incorporate_object_libraries_so languageThreading)

if("${LANGUAGE_CONCURRENCY_GLOBAL_EXECUTOR}" STREQUAL "dispatch")
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    include_directories(AFTER
                          ${LANGUAGE_PATH_TO_LIBDISPATCH_SOURCE})

    # FIXME: we can't rely on libdispatch having been built for the
    # target at this point in the process.  Currently, we're relying
    # on soft-linking.
    list(APPEND language_concurrency_link_libraries
      dispatch)
  endif()
elseif("${LANGUAGE_CONCURRENCY_GLOBAL_EXECUTOR}" STREQUAL "singlethreaded" OR
       "${LANGUAGE_CONCURRENCY_GLOBAL_EXECUTOR}" STREQUAL "hooked" OR
       "${LANGUAGE_CONCURRENCY_GLOBAL_EXECUTOR}" STREQUAL "none")
  list(APPEND LANGUAGE_RUNTIME_CONCURRENCY_C_FLAGS
    "-DLANGUAGE_CONCURRENCY_ENABLE_DISPATCH=0")
else()
  message(FATAL_ERROR "Invalid value for LANGUAGE_CONCURRENCY_GLOBAL_EXECUTOR (\"${LANGUAGE_CONCURRENCY_GLOBAL_EXECUTOR}\").")
endif()

if(NOT language_concurrency_async_fp_mode)
  set(language_concurrency_async_fp_mode "always")
endif()

# Don't emit extended frame info on platforms other than darwin, system
# backtracer and system debugger are unlikely to support it.
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  list(APPEND LANGUAGE_RUNTIME_CONCURRENCY_C_FLAGS
    "-flanguage-async-fp=${language_concurrency_async_fp_mode}")
  list(APPEND LANGUAGE_RUNTIME_CONCURRENCY_LANGUAGE_FLAGS
    "-Xfrontend"
    "-language-async-frame-pointer=${language_concurrency_async_fp_mode}")
else()
  list(APPEND LANGUAGE_RUNTIME_CONCURRENCY_C_FLAGS "-flanguage-async-fp=never")
endif()

# Do not remove until IsolatedAny is on by default in all supported compilers.
list(APPEND LANGUAGE_RUNTIME_CONCURRENCY_LANGUAGE_FLAGS
  "-enable-experimental-feature"
  "IsolatedAny"
  )

list(APPEND LANGUAGE_RUNTIME_CONCURRENCY_LANGUAGE_FLAGS "-strict-memory-safety")
list(APPEND LANGUAGE_RUNTIME_CONCURRENCY_LANGUAGE_FLAGS "-enable-experimental-feature" "AllowUnsafeAttribute")

list(APPEND LANGUAGE_RUNTIME_CONCURRENCY_LANGUAGE_FLAGS "-enable-experimental-feature" "Extern")

list(APPEND LANGUAGE_RUNTIME_CONCURRENCY_C_FLAGS
  "-D__STDC_WANT_LIB_EXT1__=1")

set(LANGUAGE_RUNTIME_CONCURRENCY_C_SOURCES
  ../CompatibilityOverride/CompatibilityOverride.cpp
  Actor.cpp
  AsyncLet.cpp
  Clock.cpp
  GlobalExecutor.cpp
  ConcurrencyHooks.cpp
  EmbeddedSupport.cpp
  Error.cpp
  ExecutorBridge.cpp
  ExecutorChecks.cpp
  Setup.cpp
  Task.cpp
  TaskAlloc.cpp
  TaskStatus.cpp
  TaskGroup.cpp
  TaskLocal.cpp
  ThreadingError.cpp
  TracingSignpost.cpp
  AsyncStream.cpp
  linker-support/magic-symbols-for-install-name.c
)

set(LANGUAGE_RUNTIME_CONCURRENCY_LANGUAGE_SOURCES
  Actor.code
  AsyncLet.code
  CheckedContinuation.code
  Errors.code
  Executor.code
  ExecutorBridge.code
  ExecutorAssertions.code
  AsyncCompactMapSequence.code
  AsyncDropFirstSequence.code
  AsyncDropWhileSequence.code
  AsyncFilterSequence.code
  AsyncFlatMapSequence.code
  AsyncIteratorProtocol.code
  AsyncMapSequence.code
  AsyncPrefixSequence.code
  AsyncPrefixWhileSequence.code
  AsyncSequence.code
  AsyncThrowingCompactMapSequence.code
  AsyncThrowingDropWhileSequence.code
  AsyncThrowingFilterSequence.code
  AsyncThrowingFlatMapSequence.code
  AsyncThrowingMapSequence.code
  AsyncThrowingPrefixWhileSequence.code
  PartialAsyncTask.code
  GlobalActor.code
  GlobalConcurrentExecutor.code
  MainActor.code
  PriorityQueue.code
  SourceCompatibilityShims.code
  Task.code
  Task+PriorityEscalation.code
  Task+TaskExecutor.code
  TaskCancellation.code
  TaskGroup.code
  DiscardingTaskGroup.code
  TaskLocal.code
  TaskSleep.code
  AsyncStreamBuffer.code
  AsyncStream.code
  AsyncThrowingStream.code
  Deque/_DequeBuffer.code
  Deque/_DequeBufferHeader.code
  Deque/_DequeSlot.code
  Deque/_UnsafeWrappedBuffer.code
  Deque/Compatibility.code
  Deque/Deque+Storage.code
  Deque/Deque+UnsafeHandle.code
  Deque/Deque.code
  Deque/Deque+Codable.code
  Deque/Deque+Collection.code
  Deque/Deque+CustomDebugStringConvertible.code
  Deque/Deque+CustomReflectable.code
  Deque/Deque+CustomStringConvertible.code
  Deque/Deque+Equatable.code
  Deque/Deque+ExpressibleByArrayLiteral.code
  Deque/Deque+Extras.code
  Deque/Deque+Hashable.code
  Deque/Deque+Testing.code
  Deque/UnsafeMutableBufferPointer+Utilities.code
  Clock.code
  ContinuousClock.code
  SuspendingClock.code
  TaskSleepDuration.code
  UnimplementedExecutor.code
  CooperativeExecutor.code
  PlatformExecutorDarwin.code
  PlatformExecutorLinux.code
  PlatformExecutorWindows.code
  PlatformExecutorOpenBSD.code
  PlatformExecutorFreeBSD.code
)

set(LANGUAGE_RUNTIME_CONCURRENCY_NONEMBEDDED_C_SOURCES
  ExecutorImpl.cpp
)

set(LANGUAGE_RUNTIME_CONCURRENCY_EXECUTOR_SOURCES)
set(LANGUAGE_RUNTIME_CONCURRENCY_NONEMBEDDED_LANGUAGE_SOURCES)

if("${LANGUAGE_CONCURRENCY_GLOBAL_EXECUTOR}" STREQUAL "dispatch")
  set(LANGUAGE_RUNTIME_CONCURRENCY_EXECUTOR_SOURCES
    DispatchGlobalExecutor.cpp
  )
  set(LANGUAGE_RUNTIME_CONCURRENCY_NONEMBEDDED_LANGUAGE_SOURCES
    DispatchExecutor.code
    CFExecutor.code
    ExecutorImpl.code
  )
elseif("${LANGUAGE_CONCURRENCY_GLOBAL_EXECUTOR}" STREQUAL "singlethreaded")
  set(LANGUAGE_RUNTIME_CONCURRENCY_NONEMBEDDED_LANGUAGE_SOURCES
    ExecutorImpl.code
    PlatformExecutorCooperative.code
    )
else()
  set(LANGUAGE_RUNTIME_CONCURRENCY_NONEMBEDDED_LANGUAGE_SOURCES
    ExecutorImpl.code
    PlatformExecutorNone.code
  )
endif()

set(LANGUAGE_RUNTIME_CONCURRENCY_EMBEDDED_LANGUAGE_SOURCES
  PlatformExecutorNone.code
  )

set(TOOLCHAIN_OPTIONAL_SOURCES
  DispatchGlobalExecutor.cpp
  CooperativeGlobalExecutor.cpp
  DispatchGlobalExecutor.cpp
)

set(LANGUAGE_CONCURRENCY_DEPENDENCIES)
if((LANGUAGE_BUILD_CLANG_OVERLAYS
    OR LANGUAGE_BUILD_TEST_SUPPORT_MODULES)
   AND (NOT DEFINED LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT
        OR NOT LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT))
  set(LANGUAGE_CONCURRENCY_DEPENDENCIES _Builtin_float)
endif()
set(LANGUAGE_CONCURRENCY_GYB_SOURCES
  Task+init.code.gyb
  TaskGroup+addTask.code.gyb
  Task+immediate.code.gyb)

# Build with a minimum deployment target of 10.15
if(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_OSX VERSION_LESS "10.15")
  set(osx_deployment_target "10.15")
else()
  set(osx_deployment_target "${LANGUAGE_DARWIN_DEPLOYMENT_VERSION_OSX}")
endif()

add_language_target_library(language_Concurrency ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  ${LANGUAGE_RUNTIME_CONCURRENCY_C_SOURCES}
  ${LANGUAGE_RUNTIME_CONCURRENCY_EXECUTOR_SOURCES}
  ${LANGUAGE_RUNTIME_CONCURRENCY_NONEMBEDDED_C_SOURCES}
  ${LANGUAGE_RUNTIME_CONCURRENCY_LANGUAGE_SOURCES}
  ${LANGUAGE_RUNTIME_CONCURRENCY_NONEMBEDDED_LANGUAGE_SOURCES}

  GYB_SOURCES
    ${LANGUAGE_CONCURRENCY_GYB_SOURCES}

  LANGUAGE_MODULE_DEPENDS ${LANGUAGE_CONCURRENCY_DEPENDENCIES}
  LANGUAGE_MODULE_DEPENDS_ANDROID Android
  LANGUAGE_MODULE_DEPENDS_LINUX Glibc
  LANGUAGE_MODULE_DEPENDS_LINUX_STATIC Musl
  LANGUAGE_MODULE_DEPENDS_FREEBSD Glibc
  LANGUAGE_MODULE_DEPENDS_OPENBSD Glibc
  LANGUAGE_MODULE_DEPENDS_CYGWIN Glibc
  LANGUAGE_MODULE_DEPENDS_HAIKU Glibc
  LANGUAGE_MODULE_DEPENDS_WINDOWS CRT

  PRIVATE_LINK_LIBRARIES ${language_concurrency_private_link_libraries}
  INCORPORATE_OBJECT_LIBRARIES_SHARED_ONLY
    ${language_concurrency_incorporate_object_libraries_so}
  LINK_LIBRARIES ${language_concurrency_link_libraries}

  DEPLOYMENT_VERSION_OSX ${osx_deployment_target}

  C_COMPILE_FLAGS
    -Dlanguage_Concurrency_EXPORTS ${LANGUAGE_RUNTIME_CONCURRENCY_C_FLAGS}
    -I${LANGUAGE_SOURCE_DIR}/stdlib/include
  LANGUAGE_COMPILE_FLAGS
    ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
    -parse-stdlib
    ${LANGUAGE_RUNTIME_CONCURRENCY_LANGUAGE_FLAGS}
  ${language_concurrency_options}
  LINK_FLAGS "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}"
  INSTALL_IN_COMPONENT stdlib
  MACCATALYST_BUILD_FLAVOR zippered
)

# Embedded Codira Concurrency library
set(LANGUAGE_SHOULD_BUILD_EMBEDDED_CONCURRENCY TRUE)
is_build_type_optimized("${LANGUAGE_STDLIB_BUILD_TYPE}" language_optimized)
if(NOT language_optimized)
  set(LANGUAGE_SHOULD_BUILD_EMBEDDED_CONCURRENCY FALSE)
elseif(BOOTSTRAPPING_MODE STREQUAL "OFF")
  set(LANGUAGE_SHOULD_BUILD_EMBEDDED_CONCURRENCY FALSE)
elseif(LANGUAGE_HOST_VARIANT STREQUAL "windows") # For now, don't build embedded Concurrency on Windows
  set(LANGUAGE_SHOULD_BUILD_EMBEDDED_CONCURRENCY FALSE)
endif()
if(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB AND LANGUAGE_SHOULD_BUILD_EMBEDDED_CONCURRENCY)
  add_custom_target(embedded-concurrency)
  add_dependencies(embedded-libraries embedded-concurrency)

  set(LANGUAGE_ENABLE_REFLECTION OFF)
  set(LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT OFF)
  set(LANGUAGE_STDLIB_STABLE_ABI OFF)
  set(LANGUAGE_STDLIB_ENABLE_OBJC_INTEROP OFF)
  set(LANGUAGE_CONCURRENCY_USES_DISPATCH FALSE)
  set(LANGUAGE_STDLIB_SINGLE_THREADED_CONCURRENCY TRUE)
  set(LANGUAGE_STDLIB_CONCURRENCY_TRACING FALSE)
  set(LANGUAGE_STDLIB_HAS_ENVIRON FALSE)
  set(LANGUAGE_STDLIB_HAS_ASL FALSE)
  list(APPEND TOOLCHAIN_OPTIONAL_SOURCES ExecutorImpl.cpp)

  foreach(entry ${EMBEDDED_STDLIB_TARGET_TRIPLES})
    string(REGEX REPLACE "[ \t]+" ";" list "${entry}")
    list(GET list 0 arch)
    list(GET list 1 mod)
    list(GET list 2 triple)

    set(extra_c_compile_flags)
    set(extra_language_compile_flags)

    if (LANGUAGE_HOST_VARIANT STREQUAL "linux")
      if(NOT "${mod}" MATCHES "-linux-gnu$")
        continue()
      endif()
    elseif (LANGUAGE_HOST_VARIANT STREQUAL "macosx")
      if(NOT "${mod}" MATCHES "x86_64|arm64|arm64e|armv7|armv7m|armv7em")
        continue()
      endif()

      if(NOT "${mod}" MATCHES "-apple-" OR "${mod}" MATCHES "-none-macho" OR "${arch}" STREQUAL "armv7m" OR "${arch}" STREQUAL "armv7em")
        # Host is macOS with a macOS SDK. To be able to build the C++ Concurrency runtime for non-Darwin targets using the macOS SDK,
        # we need to pass some extra flags and search paths.
        set(extra_c_compile_flags -stdlib=libc++ -isystem${LANGUAGE_SDK_OSX_PATH}/usr/include/c++/v1 -isystem${LANGUAGE_SDK_OSX_PATH}/usr/include -D__APPLE__)
      endif()
    endif()

    set(LANGUAGE_SDK_embedded_THREADING_PACKAGE none)
    set(LANGUAGE_SDK_embedded_ARCH_${arch}_MODULE "${mod}")
    set(LANGUAGE_SDK_embedded_ARCH_${mod}_MODULE "${mod}")
    set(LANGUAGE_SDK_embedded_LIB_SUBDIR "embedded")
    set(LANGUAGE_SDK_embedded_ARCH_${arch}_TRIPLE "${triple}")
    set(LANGUAGE_SDK_embedded_ARCH_${mod}_TRIPLE "${triple}")

    # lib/language/embedded/_Concurrency.codemodule
    # lib/language/embedded/<triple>/liblanguage_Concurrency.a
    add_language_target_library_single(
      embedded-concurrency-${mod}
      language_Concurrency
      STATIC
      IS_STDLIB IS_FRAGILE

      ${LANGUAGE_RUNTIME_CONCURRENCY_C_SOURCES}
      ${LANGUAGE_RUNTIME_CONCURRENCY_LANGUAGE_SOURCES}
      ${LANGUAGE_RUNTIME_CONCURRENCY_EMBEDDED_LANGUAGE_SOURCES}

      GYB_SOURCES
        ${LANGUAGE_CONCURRENCY_GYB_SOURCES}

      LANGUAGE_COMPILE_FLAGS
        ${extra_language_compile_flags} -enable-experimental-feature Embedded
        -parse-stdlib -DLANGUAGE_CONCURRENCY_EMBEDDED
        -Xfrontend -emit-empty-object-file
        ${LANGUAGE_RUNTIME_CONCURRENCY_LANGUAGE_FLAGS}
      C_COMPILE_FLAGS
        ${extra_c_compile_flags} ${LANGUAGE_RUNTIME_CONCURRENCY_C_FLAGS} -DLANGUAGE_CONCURRENCY_EMBEDDED=1 -DLANGUAGE_RUNTIME_EMBEDDED=1
        -ffunction-sections -fdata-sections -fno-exceptions -fno-cxx-exceptions -fno-unwind-tables
      MODULE_DIR "${CMAKE_BINARY_DIR}/lib/language/embedded"
      SDK "embedded"
      ARCHITECTURE "${arch}"
      ARCHITECTURE_SUBDIR_NAME "${mod}"
      DEPENDS embedded-stdlib-${mod}
      INSTALL_IN_COMPONENT stdlib
      )
    language_install_in_component(
      TARGETS embedded-concurrency-${mod}
      DESTINATION "lib/language/embedded/${mod}"
      COMPONENT "stdlib"
      )
    language_install_in_component(
      FILES "${LANGUAGELIB_DIR}/embedded/${mod}/liblanguage_Concurrency.a"
      DESTINATION "lib/language/embedded/${mod}/"
      COMPONENT "stdlib"
      PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
      )

    if(NOT "${arch}" MATCHES "wasm32")
      set_property(TARGET embedded-concurrency-${mod} PROPERTY OSX_ARCHITECTURES "${arch}")
    endif()
    add_dependencies(embedded-concurrency embedded-concurrency-${mod})

    # lib/language/embedded/<triple>/liblanguage_ConcurrencyDefaultExecutor.a
    add_language_target_library_single(
      embedded-concurrency-default-executor-${mod}
      language_ConcurrencyDefaultExecutor
      STATIC
      IS_FRAGILE

      CooperativeGlobalExecutor.cpp

      C_COMPILE_FLAGS ${extra_c_compile_flags}
      MODULE_DIR "${CMAKE_BINARY_DIR}/lib/language/embedded"
      SDK "embedded"
      ARCHITECTURE "${mod}"
      DEPENDS embedded-stdlib-${mod}
      INSTALL_IN_COMPONENT stdlib
      )
    language_install_in_component(
      TARGETS embedded-concurrency-default-executor-${mod}
      DESTINATION "lib/language/embedded/${mod}"
      COMPONENT "stdlib"
      )
    language_install_in_component(
      FILES "${LANGUAGELIB_DIR}/embedded/${mod}/liblanguage_ConcurrencyDefaultExecutor.a"
      DESTINATION "lib/language/embedded/${mod}/"
      COMPONENT "stdlib"
      PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
      )
    set_property(TARGET embedded-concurrency-default-executor-${mod} PROPERTY OSX_ARCHITECTURES "${arch}")
    add_dependencies(embedded-concurrency embedded-concurrency-default-executor-${mod})
  endforeach()

  # Copy the ExecutorImpl.h header into the local include directory
  # and install it in the compiler toolchain
  add_custom_command(
    OUTPUT "${LANGUAGE_INCLUDE_DIR}/language/ExecutorImpl.h"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ExecutorImpl.h"
    COMMAND "${CMAKE_COMMAND}" "-E" "copy"
            "${CMAKE_CURRENT_SOURCE_DIR}/ExecutorImpl.h"
            "${LANGUAGE_INCLUDE_DIR}/language")

  add_custom_target("copy_executor_impl_header"
    DEPENDS "${LANGUAGE_INCLUDE_DIR}/language/ExecutorImpl.h"
    COMMENT "Copying executor implementation header to ${LANGUAGE_INCLUDE_DIR}/language")

  language_install_in_component(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/ExecutorImpl.h"
    DESTINATION "include/language"
    COMPONENT compiler)

  add_dependencies(embedded-concurrency "copy_executor_impl_header")
endif()
