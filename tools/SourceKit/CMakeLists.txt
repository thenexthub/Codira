include(CheckSymbolExists)

# Append our own modules to the module path.
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(SEND_ERROR "SourceKit cannot be built standalone")
endif()

set(SOURCEKIT_LANGUAGE_SWAP_COMPILER FALSE)
include(CodiraWindowsSupport)
language_swap_compiler_if_needed("SourceKit")

if (DARWIN_TOOLCHAIN_VERSION)
  set(SOURCEKIT_VERSION_STRING "${DARWIN_TOOLCHAIN_VERSION}")
else()
  set(SOURCEKIT_VERSION_STRING "${LANGUAGE_COMPILER_VERSION}")
endif()

set(SOURCEKIT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SOURCEKIT_BINARY_DIR ${LANGUAGE_BINARY_DIR})

set(SOURCEKIT_RUNTIME_OUTPUT_INTDIR "${LANGUAGE_RUNTIME_OUTPUT_INTDIR}")
set(SOURCEKIT_LIBRARY_OUTPUT_INTDIR "${LANGUAGE_LIBRARY_OUTPUT_INTDIR}")

check_symbol_exists(dispatch_block_create "dispatch/dispatch.h" HAVE_DISPATCH_BLOCK_CREATE)
configure_file(
  ${SOURCEKIT_SOURCE_DIR}/include/SourceKit/Config/config.h.cmake
  ${SOURCEKIT_BINARY_DIR}/include/SourceKit/Config/config.h)

set(SOURCEKIT_DEPLOYMENT_OS "${LANGUAGE_HOST_VARIANT}")
set(SOURCEKIT_DEPLOYMENT_TARGET "${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_DEPLOYMENT_VERSION}")

set(SOURCEKIT_TOOLCHAIN_NAME "${LANGUAGE_DARWIN_XCRUN_TOOLCHAIN}")
if("${SOURCEKIT_TOOLCHAIN_NAME}" MATCHES "XcodeDefault")
  set(SOURCEKIT_TOOLCHAIN_NAME "default")
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" AND NOT CMAKE_CROSSCOMPILING)
  set(CMAKE_OSX_SYSROOT "${LANGUAGE_SDK_${LANGUAGE_HOST_VARIANT_SDK}_PATH}")
  set(CMAKE_OSX_ARCHITECTURES "${LANGUAGE_HOST_VARIANT_ARCH}")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "")
endif()

# If we were don't have XPC, just build inproc.
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" OR NOT HAVE_XPC_H)
  set(LANGUAGE_SOURCEKIT_USE_INPROC_LIBRARY TRUE)
endif()

# Now include AddCodiraSourceKit
include(AddCodiraSourceKit)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  # Choose a deployment target if none was set.
  if (NOT SOURCEKIT_DEPLOYMENT_TARGET)
    execute_process(COMMAND sw_vers -productVersion
                    OUTPUT_VARIABLE SOURCEKIT_DEPLOYMENT_TARGET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCH "[0-9]+\\.[0-9]+" SOURCEKIT_DEPLOYMENT_TARGET "${SOURCEKIT_DEPLOYMENT_TARGET}")
  endif()

  # Sadly there are two OS naming conventions.
  # xcrun SDK name:   macosx iphoneos iphonesimulator (+ "internal" or version)
  # triple name:      macosx ios      ios-simulator

  if (SOURCEKIT_DEPLOYMENT_OS MATCHES "^iphoneos")
    set(version_min_os "ios")
    set(triple_os "ios")
    set(SOURCEKIT_DEFAULT_TARGET_SDK "IOS")
  elseif (SOURCEKIT_DEPLOYMENT_OS MATCHES "^iphonesimulator")
    set(version_min_os "ios-simulator")
    set(triple_os "ios")
    set(SOURCEKIT_DEFAULT_TARGET_SDK "IOS_SIMULATOR")
  elseif (SOURCEKIT_DEPLOYMENT_OS MATCHES "^macosx")
    set(version_min_os "macosx")
    set(triple_os "macosx")
    set(SOURCEKIT_DEFAULT_TARGET_SDK "OSX")
  else()
    message(FATAL_ERROR "Can't understand SOURCEKIT_DEPLOYMENT_OS '${SOURCEKIT_DEPLOYMENT_OS}';")
  endif()
endif()

include_directories(BEFORE
  ${SOURCEKIT_BINARY_DIR}/include
  ${SOURCEKIT_SOURCE_DIR}/include
  ${SOURCEKIT_BINARY_DIR}/tools/SourceKit/include
)

if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  string(TOUPPER "${CMAKE_SYSTEM_NAME}" SOURCEKIT_DEFAULT_TARGET_SDK)
endif()

if(LANGUAGE_BUILD_HOST_DISPATCH)
  if(LANGUAGE_HOST_VARIANT_SDK STREQUAL "WINDOWS")
    set(SOURCEKIT_RUNTIME_DIR bin)
  else()
    set(SOURCEKIT_RUNTIME_DIR lib)
  endif()
  add_dependencies(sourcekit-inproc BlocksRuntime dispatch)
  if("${LANGUAGE_HOST_VARIANT_SDK}" MATCHES "OSX|WINDOWS")
    language_install_in_component(FILES
      $<TARGET_FILE:dispatch>
      $<TARGET_FILE:BlocksRuntime>
      DESTINATION ${SOURCEKIT_RUNTIME_DIR}
      COMPONENT sourcekit-inproc)
  endif()
  if(LANGUAGE_HOST_VARIANT_SDK STREQUAL "WINDOWS")
    language_install_in_component(FILES
      $<TARGET_LINKER_FILE:dispatch>
      $<TARGET_LINKER_FILE:BlocksRuntime>
      DESTINATION lib
      COMPONENT sourcekit-inproc)
  endif()
endif()

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(tools)
