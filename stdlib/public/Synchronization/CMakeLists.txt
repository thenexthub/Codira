#===--- CMakeLists.txt - Synchronization library ---------------------------===#
#
# Copyright (c) NeXTHub Corporation. All rights reserved.
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# This code is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# version 2 for more details (a copy is included in the LICENSE file that
# accompanied this code).
#
# Author(-s): Tunjay Akbarli

#
#===----------------------------------------------------------------------===#

set(LANGUAGE_SYNCHRONIZATION_ATOMIC_SOURCES
  Atomics/Atomic.code
  Atomics/AtomicBool.code
  Atomics/AtomicFloats.code
  Atomics/AtomicLazyReference.code
  Atomics/AtomicMemoryOrderings.code
  Atomics/AtomicOptional.code
  Atomics/AtomicPointers.code
  Atomics/AtomicRepresentable.code
  Atomics/WordPair.code
)

set(LANGUAGE_SYNCHRONIZATION_SOURCES
  ${LANGUAGE_SYNCHRONIZATION_ATOMIC_SOURCES}

  Cell.code
)

set(LANGUAGE_SYNCHRONIZATION_GYB_SOURCES
  Atomics/AtomicIntegers.code.gyb
  Atomics/AtomicStorage.code.gyb
)

# Darwin dependencies and sources

set(LANGUAGE_SYNCHRONIZATION_DARWIN_DEPENDENCIES)

if((LANGUAGE_BUILD_CLANG_OVERLAYS
    OR LANGUAGE_BUILD_TEST_SUPPORT_MODULES)
   AND (NOT DEFINED LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT
        OR NOT LANGUAGE_BUILD_CLANG_OVERLAYS_SKIP_BUILTIN_FLOAT))
  # This library imports Darwin from the SDK, which re-exports
  # _Builtin_float which is part of this build.
  list(APPEND LANGUAGE_SYNCHRONIZATION_DARWIN_DEPENDENCIES _Builtin_float)
endif()

set(LANGUAGE_SYNCHRONIZATION_DARWIN_SOURCES
  Mutex/DarwinImpl.code
  Mutex/Mutex.code
)

# Linux and Android sources

set(LANGUAGE_SYNCHRONIZATION_LINUX_SOURCES
  Mutex/LinuxImpl.code
  Mutex/Mutex.code
  Mutex/SpinLoopHint.code
)

# FreeBSD sources

set(LANGUAGE_SYNCHRONIZATION_FREEBSD_SOURCES
  Mutex/FreeBSDImpl.code
  Mutex/Mutex.code
)

# Wasm sources

set(LANGUAGE_SYNCHRONIZATION_WASM_SOURCES
  Mutex/Mutex.code
  Mutex/WasmImpl.code
)

# Windows sources

set(LANGUAGE_SYNCHRONIZATION_WINDOWS_SOURCES
  Mutex/Mutex.code
  Mutex/WindowsImpl.code
)

set(LANGUAGE_SYNCHRNOIZATION_LANGUAGE_FLAGS
  ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
  "-enable-builtin-module"
  "-enable-experimental-feature" "RawLayout"
  "-enable-experimental-feature" "StaticExclusiveOnly"
  "-enable-experimental-feature" "Extern"
  "-strict-memory-safety"
)

add_language_target_library(languageSynchronization ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB IMPORTS_NON_OSSA 
  ${LANGUAGE_SYNCHRONIZATION_SOURCES}

  GYB_SOURCES
    ${LANGUAGE_SYNCHRONIZATION_GYB_SOURCES}

  LANGUAGE_SOURCES_DEPENDS_MACOS
    ${LANGUAGE_SYNCHRONIZATION_DARWIN_SOURCES}
  LANGUAGE_SOURCES_DEPENDS_IOS
    ${LANGUAGE_SYNCHRONIZATION_DARWIN_SOURCES}
  LANGUAGE_SOURCES_DEPENDS_TVOS
    ${LANGUAGE_SYNCHRONIZATION_DARWIN_SOURCES}
  LANGUAGE_SOURCES_DEPENDS_WATCHOS
    ${LANGUAGE_SYNCHRONIZATION_DARWIN_SOURCES}
  LANGUAGE_SOURCES_DEPENDS_VISIONOS
    ${LANGUAGE_SYNCHRONIZATION_DARWIN_SOURCES}
  LANGUAGE_SOURCES_DEPENDS_LINUX
    ${LANGUAGE_SYNCHRONIZATION_LINUX_SOURCES}
  LANGUAGE_SOURCES_DEPENDS_LINUX_STATIC
    ${LANGUAGE_SYNCHRONIZATION_LINUX_SOURCES}
  LANGUAGE_SOURCES_DEPENDS_WASI
    ${LANGUAGE_SYNCHRONIZATION_WASM_SOURCES}
  LANGUAGE_SOURCES_DEPENDS_WINDOWS
    ${LANGUAGE_SYNCHRONIZATION_WINDOWS_SOURCES}
  LANGUAGE_SOURCES_DEPENDS_FREEBSD
    ${LANGUAGE_SYNCHRONIZATION_FREEBSD_SOURCES}
  LANGUAGE_SOURCES_DEPENDS_FREESTANDING
    Mutex/MutexUnavailable.code

  LANGUAGE_MODULE_DEPENDS_OSX
    ${LANGUAGE_SYNCHRONIZATION_DARWIN_DEPENDENCIES}
  LANGUAGE_MODULE_DEPENDS_IOS
    ${LANGUAGE_SYNCHRONIZATION_DARWIN_DEPENDENCIES}
  LANGUAGE_MODULE_DEPENDS_TVOS
    ${LANGUAGE_SYNCHRONIZATION_DARWIN_DEPENDENCIES}
  LANGUAGE_MODULE_DEPENDS_WATCHOS
    ${LANGUAGE_SYNCHRONIZATION_DARWIN_DEPENDENCIES}
  LANGUAGE_MODULE_DEPENDS_XROS
    ${LANGUAGE_SYNCHRONIZATION_DARWIN_DEPENDENCIES}
  LANGUAGE_MODULE_DEPENDS_MACCATALYST
    ${LANGUAGE_SYNCHRONIZATION_DARWIN_DEPENDENCIES}
  LANGUAGE_MODULE_DEPENDS_LINUX
    Glibc
  LANGUAGE_MODULE_DEPENDS_LINUX_STATIC
    Musl
  LANGUAGE_MODULE_DEPENDS_ANDROID
    Android
  LANGUAGE_MODULE_DEPENDS_WINDOWS
    WinSDK
  LANGUAGE_MODULE_DEPENDS_FREEBSD
    Glibc

  LANGUAGE_COMPILE_FLAGS
    ${LANGUAGE_SYNCHRNOIZATION_LANGUAGE_FLAGS}
  LINK_FLAGS
    "${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}"
  INSTALL_IN_COMPONENT
    stdlib
  MACCATALYST_BUILD_FLAVOR
    "zippered"
)

# Embedded Synchronization - embedded libraries are built as .codemodule only,
# i.e. there is no .o or .a file produced (no binary code is actually produced)
# and only users of a library are going to actually compile any needed code.
if(LANGUAGE_SHOULD_BUILD_EMBEDDED_STDLIB)
  add_custom_target(embedded-synchronization)
  add_dependencies(embedded-libraries embedded-synchronization)

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

    # Disable the Synchronization library on AVR for now.
    if("${arch}" MATCHES "avr")
      continue()
    endif()

    set(LANGUAGE_SDK_embedded_ARCH_${arch}_MODULE "${mod}")
    set(LANGUAGE_SDK_embedded_LIB_SUBDIR "embedded")
    set(LANGUAGE_SDK_embedded_ARCH_${arch}_TRIPLE "${triple}")
    add_language_target_library_single(
      embedded-synchronization-${mod}
      languageSynchronization
      ONLY_LANGUAGEMODULE
      IS_FRAGILE
      
      ${LANGUAGE_SYNCHRONIZATION_ATOMIC_SOURCES}
      GYB_SOURCES
        ${LANGUAGE_SYNCHRONIZATION_GYB_SOURCES}

      LANGUAGE_COMPILE_FLAGS
        ${LANGUAGE_SYNCHRNOIZATION_LANGUAGE_FLAGS}
        -Xcc -ffreestanding -enable-experimental-feature Embedded

      MODULE_DIR "${CMAKE_BINARY_DIR}/lib/language/embedded"
      SDK "embedded"
      ARCHITECTURE "${arch}"
      DEPENDS embedded-stdlib-${mod}
      INSTALL_IN_COMPONENT stdlib
      )
    add_dependencies(embedded-synchronization embedded-synchronization-${mod})
  endforeach()
endif()
