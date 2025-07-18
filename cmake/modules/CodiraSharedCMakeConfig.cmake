include(CMakeParseArguments)

include(CheckCXXCompilerFlag)

macro(language_common_standalone_build_config_toolchain product)
  option(TOOLCHAIN_ENABLE_WARNINGS "Enable compiler warnings." ON)

  # If we already have a cached value for TOOLCHAIN_ENABLE_ASSERTIONS, save the value.
  if(DEFINED TOOLCHAIN_ENABLE_ASSERTIONS)
    set(TOOLCHAIN_ENABLE_ASSERTIONS_saved "${TOOLCHAIN_ENABLE_ASSERTIONS}")
  endif()

  # Then we import LLVMConfig. This is going to override whatever cached value
  # we have for TOOLCHAIN_ENABLE_ASSERTIONS.
  find_package(LLVM CONFIG REQUIRED NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  list(APPEND CMAKE_MODULE_PATH "${TOOLCHAIN_CMAKE_DIR}")

  set(TOOLCHAIN_MAIN_SRC_DIR "${TOOLCHAIN_BUILD_MAIN_SRC_DIR}"
    CACHE PATH "Path to LLVM source tree")
  set(TOOLCHAIN_MAIN_INCLUDE_DIR "${TOOLCHAIN_BUILD_MAIN_INCLUDE_DIR}"
    CACHE PATH "Path to toolchain/include")

  # If we did not have a cached value for TOOLCHAIN_ENABLE_ASSERTIONS, set
  # TOOLCHAIN_ENABLE_ASSERTIONS_saved to be the ENABLE_ASSERTIONS value from LLVM so
  # we follow LLVMConfig.cmake's value by default if nothing is provided.
  if(NOT DEFINED TOOLCHAIN_ENABLE_ASSERTIONS_saved)
    set(TOOLCHAIN_ENABLE_ASSERTIONS_saved "${TOOLCHAIN_ENABLE_ASSERTIONS}")
  endif()

  # Then set TOOLCHAIN_ENABLE_ASSERTIONS with a default value of
  # TOOLCHAIN_ENABLE_ASSERTIONS_saved.
  #
  # The effect of this is that if TOOLCHAIN_ENABLE_ASSERTION did not have a cached
  # value, then TOOLCHAIN_ENABLE_ASSERTIONS_saved is set to LLVM's value, so we get a
  # default value from LLVM.
  set(TOOLCHAIN_ENABLE_ASSERTIONS "${TOOLCHAIN_ENABLE_ASSERTIONS_saved}"
    CACHE BOOL "Enable assertions")
  mark_as_advanced(TOOLCHAIN_ENABLE_ASSERTIONS)

  precondition(TOOLCHAIN_TOOLS_BINARY_DIR)
  precondition_translate_flag(TOOLCHAIN_BUILD_LIBRARY_DIR TOOLCHAIN_LIBRARY_DIR)
  precondition(TOOLCHAIN_LIBRARY_DIRS)

  # This could be computed using ${CMAKE_CFG_INTDIR} if we want to link Codira
  # against a matching LLVM build configuration.  However, we usually want to be
  # flexible and allow linking a debug Codira against optimized LLVM.
  set(TOOLCHAIN_RUNTIME_OUTPUT_INTDIR "${TOOLCHAIN_BINARY_DIR}")
  set(TOOLCHAIN_BINARY_OUTPUT_INTDIR "${TOOLCHAIN_TOOLS_BINARY_DIR}")
  set(TOOLCHAIN_LIBRARY_OUTPUT_INTDIR "${TOOLCHAIN_LIBRARY_DIR}")

  if(NOT CMAKE_CROSSCOMPILING)
    set(${product}_NATIVE_TOOLCHAIN_TOOLS_PATH "${TOOLCHAIN_TOOLS_BINARY_DIR}")
  endif()

  if(LANGUAGE_INCLUDE_TOOLS)
    if(TOOLCHAIN_TABLEGEN)
      set(TOOLCHAIN_TABLEGEN_EXE ${TOOLCHAIN_TABLEGEN})
    else()
      if(CMAKE_CROSSCOMPILING)
        set(TOOLCHAIN_NATIVE_BUILD_DIR "${TOOLCHAIN_BINARY_DIR}/NATIVE")
        if(NOT EXISTS "${TOOLCHAIN_NATIVE_BUILD_DIR}")
          message(FATAL_ERROR
            "Attempting to cross-compile language standalone but no native LLVM build
            found.  Please cross-compile LLVM as well.")
        endif()

        if(CMAKE_HOST_SYSTEM_NAME MATCHES Windows)
          set(HOST_EXECUTABLE_SUFFIX ".exe")
        endif()

        if(NOT CMAKE_CONFIGURATION_TYPES)
          set(TOOLCHAIN_TABLEGEN_EXE
            "${TOOLCHAIN_NATIVE_BUILD_DIR}/bin/toolchain-tblgen${HOST_EXECUTABLE_SUFFIX}")
        else()
          # NOTE: LLVM NATIVE build is always built Release, as is specified in
          # CrossCompile.cmake
          set(TOOLCHAIN_TABLEGEN_EXE
            "${TOOLCHAIN_NATIVE_BUILD_DIR}/Release/bin/toolchain-tblgen${HOST_EXECUTABLE_SUFFIX}")
        endif()
      else()
        find_program(TOOLCHAIN_TABLEGEN_EXE "toolchain-tblgen" HINTS ${TOOLCHAIN_TOOLS_BINARY_DIR}
          NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
        if(TOOLCHAIN_TABLEGEN_EXE STREQUAL "TOOLCHAIN_TABLEGEN_EXE-NOTFOUND")
          message(FATAL_ERROR "Failed to find tablegen in ${TOOLCHAIN_TOOLS_BINARY_DIR}")
        endif()
      endif()
    endif()
  endif()

  if(TOOLCHAIN_ENABLE_ZLIB)
    find_package(ZLIB REQUIRED)
  endif()

  # Work around a bug in the language-driver that causes the language-driver to not be
  # able to accept .tbd files when linking without passing in the .tbd file with
  # a -Xlinker flag.
  #
  # Both clang and languagec can accept an -Xlinker flag so we use that to pass the
  # value.
  if (APPLE)
    get_target_property(LLVMSUPPORT_INTERFACE_LINK_LIBRARIES LLVMSupport INTERFACE_LINK_LIBRARIES)
    get_target_property(LLVMSUPPORT_INTERFACE_LINK_OPTIONS LLVMSupport INTERFACE_LINK_OPTIONS)
    set(new_libraries)
    set(new_options)
    if (LLVMSUPPORT_INTERFACE_LINK_OPTIONS)
      set(new_options ${LLVMSUPPORT_INTERFACE_LINK_OPTIONS})
    endif()
    foreach(lib ${LLVMSUPPORT_INTERFACE_LINK_LIBRARIES})
      # The reason why we also fix link libraries that are specified as a full
      # target is since those targets can still be a tbd file.
      #
      # Example: ZLIB::ZLIB's library path is defined by
      # ZLIB_LIBRARY_{DEBUG,RELEASE} which can on Darwin have a tbd file as a
      # value. So we need to work around this until we get a newer languagec that
      # can accept a .tbd file.
      if (TARGET ${lib})
        list(APPEND new_options "LINKER:$<TARGET_FILE:${lib}>")
        continue()
      endif()

      # If we have an interface library dependency that is just a path to a tbd
      # file, pass the tbd file via -Xlinker so it gets straight to the linker.
      get_filename_component(LIB_FILENAME_COMPONENT ${lib} LAST_EXT)
      if ("${LIB_FILENAME_COMPONENT}" STREQUAL ".tbd")
        list(APPEND new_options "LINKER:${lib}")
        continue()
      endif()

      list(APPEND new_libraries "${lib}")
    endforeach()

    set_target_properties(LLVMSupport PROPERTIES INTERFACE_LINK_LIBRARIES "${new_libraries}")
    set_target_properties(LLVMSupport PROPERTIES INTERFACE_LINK_OPTIONS "${new_options}")
  endif()

  include(AddLLVM)
  include(AddCodiraTableGen) # This imports TableGen from LLVM.
  include(HandleLLVMOptions)

  # HACK: Not all targets support -z,defs as a linker flag. 
  #
  # Normally, LLVM would only add it as an option for known ELF targets;
  # however, due to the custom scheme Codira uses for cross-compilation, the 
  # CMAKE_SHARED_LINKER_FLAGS are determined based on the host system and 
  # then applied to all targets. This causes issues in cross-compiling to
  # Windows from a Linux host.
  # 
  # To work around this, we unconditionally remove the flag here and then
  # selectively add it to the per-target link flags; this is currently done in
  # add_language_host_library and add_language_target_library within AddCodira.cmake.
  string(REGEX REPLACE "-Wl,-z,defs" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
  string(REGEX REPLACE "-Wl,-z,nodelete" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
  # Android build on macOS cross-compile host don't support `-Wl,-headerpad_max_install_names` and `-dynamiclib` as a linker flags.
  string(REGEX REPLACE "-Wl,-headerpad_max_install_names" "" CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS}")
  string(REGEX REPLACE "-dynamiclib" "" CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS}")

  set(PACKAGE_VERSION "${TOOLCHAIN_PACKAGE_VERSION}")
  string(REGEX REPLACE "([0-9]+)\\.[0-9]+(\\.[0-9]+)?" "\\1" PACKAGE_VERSION_MAJOR
    ${PACKAGE_VERSION})
  string(REGEX REPLACE "[0-9]+\\.([0-9]+)(\\.[0-9]+)?" "\\1" PACKAGE_VERSION_MINOR
    ${PACKAGE_VERSION})

  set(LANGUAGE_LIBCLANG_LIBRARY_VERSION
    "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}" CACHE STRING
    "Version number that will be placed into the libclang library , in the form XX.YY")

  foreach(INCLUDE_DIR ${TOOLCHAIN_INCLUDE_DIRS})
    include_directories(${INCLUDE_DIR})
  endforeach ()

  # *NOTE* if we want to support separate Clang builds as well as separate LLVM
  # builds, the clang build directory needs to be added here.
  link_directories("${TOOLCHAIN_LIBRARY_DIR}")

  set(LIT_ARGS_DEFAULT "-sv")
  set(TOOLCHAIN_LIT_ARGS "${LIT_ARGS_DEFAULT}" CACHE STRING "Default options for lit")

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

  set(TOOLCHAIN_INCLUDE_TESTS TRUE)
  set(TOOLCHAIN_INCLUDE_DOCS TRUE)

  option(TOOLCHAIN_ENABLE_DOXYGEN "Enable doxygen support" FALSE)
  if (TOOLCHAIN_ENABLE_DOXYGEN)
    find_package(Doxygen REQUIRED)
  endif()
endmacro()

macro(language_common_standalone_build_config_clang product)
  find_package(Clang CONFIG REQUIRED NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)

  if (NOT CMAKE_CROSSCOMPILING AND NOT LANGUAGE_PREBUILT_CLANG)
    set(${product}_NATIVE_CLANG_TOOLS_PATH "${TOOLCHAIN_TOOLS_BINARY_DIR}")
  endif()

  include_directories(${CLANG_INCLUDE_DIRS})
endmacro()

macro(language_common_standalone_build_config_cmark product)
  set(${product}_PATH_TO_CMARK_SOURCE "${${product}_PATH_TO_CMARK_SOURCE}"
    CACHE PATH "Path to CMark source code.")
  set(${product}_PATH_TO_CMARK_BUILD "${${product}_PATH_TO_CMARK_BUILD}"
    CACHE PATH "Path to the directory where CMark was built.")
  set(${product}_CMARK_LIBRARY_DIR "${${product}_CMARK_LIBRARY_DIR}" CACHE PATH
    "Path to the directory where CMark was installed.")
  get_filename_component(PATH_TO_CMARK_BUILD "${${product}_PATH_TO_CMARK_BUILD}"
    ABSOLUTE)
  get_filename_component(CMARK_MAIN_SRC_DIR "${${product}_PATH_TO_CMARK_SOURCE}"
    ABSOLUTE)
  get_filename_component(CMARK_LIBRARY_DIR "${${product}_CMARK_LIBRARY_DIR}"
    ABSOLUTE)

  set(CMARK_MAIN_INCLUDE_DIR "${CMARK_MAIN_SRC_DIR}/src/include")
  set(CMARK_BUILD_INCLUDE_DIR "${PATH_TO_CMARK_BUILD}/src")

  file(TO_CMAKE_PATH "${CMARK_MAIN_INCLUDE_DIR}" CMARK_MAIN_INCLUDE_DIR)
  file(TO_CMAKE_PATH "${CMARK_BUILD_INCLUDE_DIR}" CMARK_BUILD_INCLUDE_DIR)

  include_directories("${CMARK_MAIN_INCLUDE_DIR}"
                      "${CMARK_BUILD_INCLUDE_DIR}")

  include(${PATH_TO_CMARK_BUILD}/src/cmarkTargets.cmake)
  add_definitions(-DCMARK_STATIC_DEFINE)
endmacro()

# Common cmake project config for standalone builds.
#
# Parameters:
#   product
#     The product name, e.g. Codira or SourceKit. Used as prefix for some
#     cmake variables.
macro(language_common_standalone_build_config product)
  language_common_standalone_build_config_toolchain(${product})
  if(LANGUAGE_INCLUDE_TOOLS)
    language_common_standalone_build_config_clang(${product})
    language_common_standalone_build_config_cmark(${product})
  endif()

  # Enable groups for IDE generators (Xcode and MSVC).
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endmacro()

# Common cmake project config for unified builds.
#
# Parameters:
#   product
#     The product name, e.g. Codira or SourceKit. Used as prefix for some
#     cmake variables.
macro(language_common_unified_build_config product)
  set(${product}_PATH_TO_CLANG_BUILD "${CMAKE_BINARY_DIR}")
  if (NOT CMAKE_CROSSCOMPILING)
    set(${product}_NATIVE_TOOLCHAIN_TOOLS_PATH "${CMAKE_BINARY_DIR}/bin")
    set(${product}_NATIVE_CLANG_TOOLS_PATH "${CMAKE_BINARY_DIR}/bin")
  endif()
  set(TOOLCHAIN_PACKAGE_VERSION ${PACKAGE_VERSION})
  set(TOOLCHAIN_CMAKE_DIR "${CMAKE_SOURCE_DIR}/cmake/modules")
  set(CLANG_INCLUDE_DIRS
    "${TOOLCHAIN_EXTERNAL_CLANG_SOURCE_DIR}/include"
    "${TOOLCHAIN_BINARY_DIR}/tools/clang/include")

  include_directories(${CLANG_INCLUDE_DIRS})

  include(AddCodiraTableGen) # This imports TableGen from LLVM.
endmacro()

# Common cmake project config for additional warnings.
#
macro(language_common_cxx_warnings)
  # Make unhandled switch cases be an error in assert builds
  if(DEFINED TOOLCHAIN_ENABLE_ASSERTIONS)
    check_cxx_compiler_flag("-Werror=switch" CXX_SUPPORTS_WERROR_SWITCH_FLAG)
    if(CXX_SUPPORTS_WERROR_SWITCH_FLAG)
      add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Werror=switch>)
    endif()

    if(MSVC)
      check_cxx_compiler_flag("/we4062" CXX_SUPPORTS_WE4062)
      if(CXX_SUPPORTS_WE4062)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/we4062>)
      endif()
    endif()
  endif()

  check_cxx_compiler_flag("-Werror -Wimplicit-fallthrough" CXX_SUPPORTS_IMPLICIT_FALLTHROUGH_FLAG)
  if(CXX_SUPPORTS_IMPLICIT_FALLTHROUGH_FLAG)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wimplicit-fallthrough>)
  endif()

  # Check for -Wunreachable-code-aggressive instead of -Wunreachable-code, as that indicates
  # that we have the newer -Wunreachable-code implementation.
  check_cxx_compiler_flag("-Werror -Wunreachable-code-aggressive" CXX_SUPPORTS_UNREACHABLE_CODE_FLAG)
  if(CXX_SUPPORTS_UNREACHABLE_CODE_FLAG)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
  endif()

  check_cxx_compiler_flag("-Werror -Woverloaded-virtual" CXX_SUPPORTS_OVERLOADED_VIRTUAL)
  if(CXX_SUPPORTS_OVERLOADED_VIRTUAL)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>)
  endif()

  check_cxx_compiler_flag("-Werror -Wnested-anon-types" CXX_SUPPORTS_NO_NESTED_ANON_TYPES_FLAG)
  if(CXX_SUPPORTS_NO_NESTED_ANON_TYPES_FLAG)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-nested-anon-types>)
  endif()

  # Check for '-fapplication-extension'.  On OS X/iOS we wish to link all
  # dynamic libraries with this flag.
  check_cxx_compiler_flag("-fapplication-extension" CXX_SUPPORTS_FAPPLICATION_EXTENSION)

  # Disable C4067: expected tokens following preprocessor directive - expected a
  # newline.
  #
  # Disable C4068: unknown pragma.
  #
  # This means that MSVC doesn't report hundreds of warnings across the
  # repository for IDE features such as #pragma mark "Title".
  if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/wd4067>)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/wd4068>)

    check_cxx_compiler_flag("/permissive-" CXX_SUPPORTS_PERMISSIVE_FLAG)
    if(CXX_SUPPORTS_PERMISSIVE_FLAG)
      add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/permissive->)
    endif()
  endif()

  # Disallow calls to objc_msgSend() with no function pointer cast.
  add_compile_definitions($<$<COMPILE_LANGUAGE:CXX>:OBJC_OLD_DISPATCH_PROTOTYPES=0>)

  if(BRIDGING_MODE STREQUAL "PURE")
    add_compile_definitions($<$<COMPILE_LANGUAGE:CXX>:PURE_BRIDGING_MODE>)
  endif()
endmacro()

# Like 'toolchain_config()', but uses libraries from the selected build
# configuration in LLVM.  ('toolchain_config()' selects the same build configuration
# in LLVM as we have for Codira.)
function(language_common_toolchain_config target)
  set(link_components ${ARGN})

  if((LANGUAGE_BUILT_STANDALONE OR SOURCEKIT_BUILT_STANDALONE) AND NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
    toolchain_map_components_to_libnames(libnames ${link_components})

    get_target_property(target_type "${target}" TYPE)
    if("${target_type}" STREQUAL "STATIC_LIBRARY")
      target_link_libraries("${target}" INTERFACE ${libnames})
    elseif("${target_type}" STREQUAL "SHARED_LIBRARY" OR
           "${target_type}" STREQUAL "MODULE_LIBRARY")
      target_link_libraries("${target}" PRIVATE ${libnames})
    else()
      # HACK: Otherwise (for example, for executables), use a plain signature,
      # because LLVM CMake does that already.
      target_link_libraries("${target}" PRIVATE ${libnames})
    endif()
  else()
    # If Codira was not built standalone, dispatch to 'toolchain_config()'.
    toolchain_config("${target}" ${ARGN})
  endif()
endfunction()

# Set sanitizer options to all Codira compiler flags. Similar options are added to C/CXX compiler in 'HandleLLVMOptions'
macro(language_common_sanitizer_config)
  if(TOOLCHAIN_USE_SANITIZER)
    if(TOOLCHAIN_USE_SANITIZER STREQUAL "Address")
      set(_Codira_SANITIZER_FLAGS "-sanitize=address -Xclang-linker -fsanitize=address")
    elseif(TOOLCHAIN_USE_SANITIZER STREQUAL "HWAddress")
      # Not supported?
    elseif(TOOLCHAIN_USE_SANITIZER MATCHES "Memory(WithOrigins)?")
      # Not supported
      if(TOOLCHAIN_USE_SANITIZER STREQUAL "MemoryWithOrigins")
        # Not supported
      endif()
    elseif(TOOLCHAIN_USE_SANITIZER STREQUAL "Undefined")
      set(_Codira_SANITIZER_FLAGS "-sanitize=undefined -Xclang-linker -fsanitize=undefined")
    elseif(TOOLCHAIN_USE_SANITIZER STREQUAL "Thread")
      set(_Codira_SANITIZER_FLAGS "-sanitize=thread -Xclang-linker -fsanitize=thread")
    elseif(TOOLCHAIN_USE_SANITIZER STREQUAL "DataFlow")
      # Not supported
    elseif(TOOLCHAIN_USE_SANITIZER STREQUAL "Address;Undefined" OR
           TOOLCHAIN_USE_SANITIZER STREQUAL "Undefined;Address")
      set(_Codira_SANITIZER_FLAGS "-sanitize=address -sanitize=undefined -Xclang-linker -fsanitize=address -Xclang-linker -fsanitize=undefined")
    elseif(TOOLCHAIN_USE_SANITIZER STREQUAL "Leaks")
      # Not supported
    else()
      message(SEND_ERROR "unsupported value for TOOLCHAIN_USE_SANITIZER: ${TOOLCHAIN_USE_SANITIZER}")
    endif()

    set(CMAKE_Codira_FLAGS "${CMAKE_Codira_FLAGS} ${_Codira_SANITIZER_FLAGS}")

  endif()
endmacro()
