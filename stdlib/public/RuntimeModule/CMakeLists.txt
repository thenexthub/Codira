#===--- CMakeLists.txt - Runtime module ------------------------------------===#
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
#===------------------------------------------------------------------------===#
#
#  The Runtime module isn't the runtime itself; that lives in liblanguageCore;
#  rather, it's a high level Codira interface to things in the runtime.
#
#===------------------------------------------------------------------------===#

set(language_runtime_link_libraries
  languageCore
  language_Concurrency
)

set(concurrency)
if(LANGUAGE_BUILD_STDLIB AND LANGUAGE_ENABLE_EXPERIMENTAL_CONCURRENCY)
  set(concurrency _Concurrency)
endif()

set(cxxstdlib_overlay)
if(LANGUAGE_ENABLE_EXPERIMENTAL_CXX_INTEROP)
  set(cxxstdlib_overlay CxxStdlib)
endif()

set(RUNTIME_SOURCES
  Address.code
  Backtrace.code
  Backtrace+Codable.code
  BacktraceFormatter.code
  Base64.code
  ByteSwapping.code
  CachingMemoryReader.code
  CompactBacktrace.code
  CompactImageMap.code
  Compression.code
  Context.code
  CoreSymbolication.code
  Dwarf.code
  EightByteBuffer.code
  Elf.code
  ElfImageCache.code
  FramePointerUnwinder.code
  Image.code
  ImageMap.code
  ImageMap+Darwin.code
  ImageMap+Linux.code
  ImageSource.code
  Libc.code
  LimitSequence.code
  MemoryReader.code
  OSReleaseScanner.code
  ProcMapsScanner.code
  Registers.code
  Runtime.code
  RichFrame.code
  SymbolicatedBacktrace.code
  Utils.code
  Win32Extras.cpp

  get-cpu-context.${LANGUAGE_ASM_EXT}
)

set(RUNTIME_COMPILE_FLAGS
  "-cxx-interoperability-mode=default"
  "-Xfrontend;-experimental-spi-only-imports"
  "-Xcc;-I${LANGUAGE_SOURCE_DIR}/include"
  "-Xcc;-I${CMAKE_BINARY_DIR}/include"
  "-Xcc;-I${LANGUAGE_STDLIB_SOURCE_DIR}/public/RuntimeModule/modules"
  "-disable-upcoming-feature;MemberImportVisibility")

###TODO: Add these when we add static linking support
#
#list(APPEND RUNTIME_COMPILE_FLAGS
#  "-Xcc;-I${LANGUAGE_PATH_TO_ZLIB_SOURCE}"
#  "-Xcc;-I${LANGUAGE_PATH_TO_ZSTD_SOURCE}/lib"
#  "-Xcc;-I${LANGUAGE_PATH_TO_LIBLZMA_SOURCE}/src/liblzma/api")

if(LANGUAGE_ASM_AVAILABLE)
  list(APPEND RUNTIME_SOURCES get-cpu-context.${LANGUAGE_ASM_EXT})
  list(APPEND RUNTIME_COMPILE_FLAGS "-DLANGUAGE_ASM_AVAILABLE")
else()
  message(warning "Assembly language not available on this platform; backtracing will fail.")
endif()

set(TOOLCHAIN_OPTIONAL_SOURCES
  get-cpu-context.S
  get-cpu-context.asm
)

# We have to build with a deployment target of at least 10.15, otherwise
# the tests will all fail because dyld will get confused at the use of
# @rpath (from magic-symbols-for-install-name.c) and the `some Sequence<Frame>`
# in Backtrace won't be accessible.
if(LANGUAGE_DARWIN_DEPLOYMENT_VERSION_OSX VERSION_LESS "10.15")
  set(osx_deployment_target "10.15")
else()
  set(osx_deployment_target "${LANGUAGE_DARWIN_DEPLOYMENT_VERSION_OSX}")
endif()

add_language_target_library(languageRuntime ${LANGUAGE_STDLIB_LIBRARY_BUILD_TYPES} IS_STDLIB
  ${RUNTIME_SOURCES}

  LANGUAGE_MODULE_DEPENDS ${concurrency} ${cxxstdlib_overlay}

  LANGUAGE_MODULE_DEPENDS_ANDROID Android
  LANGUAGE_MODULE_DEPENDS_LINUX Glibc
  LANGUAGE_MODULE_DEPENDS_LINUX_STATIC Musl
  LANGUAGE_MODULE_DEPENDS_FREEBSD Glibc
  LANGUAGE_MODULE_DEPENDS_OPENBSD Glibc
  LANGUAGE_MODULE_DEPENDS_CYGWIN Glibc
  LANGUAGE_MODULE_DEPENDS_HAIKU Glibc
  LANGUAGE_MODULE_DEPENDS_WINDOWS CRT

  PRIVATE_LINK_LIBRARIES ${language_runtime_link_libraries}

  DEPLOYMENT_VERSION_OSX ${osx_deployment_target}

  LANGUAGE_COMPILE_FLAGS
    ${LANGUAGE_STANDARD_LIBRARY_LANGUAGE_FLAGS}
    ${RUNTIME_COMPILE_FLAGS}
    -parse-stdlib

  LINK_FLAGS
    ${LANGUAGE_RUNTIME_LANGUAGE_LINK_FLAGS}

  INSTALL_IN_COMPONENT stdlib
  MACCATALYST_BUILD_FLAVOR "zippered"

  TARGET_SDKS OSX LINUX LINUX_STATIC
)
