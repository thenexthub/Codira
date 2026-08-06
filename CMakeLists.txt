cmake_minimum_required(VERSION 3.10)
project(CodiraRuntime VERSION 0.4.0 LANGUAGES C CXX)

# Determine platform (32/64)
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(X64 ON)
else ()
    set(X86 ON)
endif ()

if (X64)
    if (WIN32)
        set(codira_os windows-x86_64)
        set(archive_ext zip)
    elseif (UNIX AND NOT APPLE)
        set(codira_os linux-x86_64)
        set(archive_ext tar.xz)
    elseif (UNIX AND APPLE)
        set(codira_os macos-x86_64)
        set(archive_ext tar.xz)
    else ()
        message(ERROR "Unsupported operating system.")
    endif ()
else ()
    message(ERROR "Only 64-bit operating systems are supported at present.")
endif ()

set(codira_library_url_latest "https://github.com/theomnira/codira/releases/download/v${CMAKE_PROJECT_VERSION}/codira_runtime-${codira_os}-${CMAKE_PROJECT_VERSION}.${archive_ext}")

option(codira_build_examples "Build all of Codira's own examples." OFF)
option(codira_build_tests "Build all of Codira's own tests." OFF)
set(codira_binaries_path "" CACHE FILEPATH "Location of a local directory containing the Codira libraries and executable.")
set(codira_library_url ${codira_library_url_latest} CACHE STRING "URL to an archive on the web containing the Codira libraries.")

set(cpp_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/cpp)

add_library(CodiraRuntime SHARED IMPORTED GLOBAL)

set_target_properties(CodiraRuntime PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES
        "${cpp_source_dir}/include"
)

include(ExternalProject)

if (codira_binaries_path)
    if (IS_DIRECTORY ${codira_binaries_path})
        set(codira_library_dir ${codira_binaries_path})
    else ()
        message(FATAL_ERROR "The provided location of a local build directory is invalid.")
    endif ()
elseif (codira_library_url)
    # Download the codira libraries
    ExternalProject_Add(
        codira_library_download
        PREFIX ${CMAKE_BINARY_DIR}/codira
        URL ${codira_library_url}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
    )

    ExternalProject_Get_Property(codira_library_download source_dir)
    set(codira_library_dir ${source})
    add_dependencies(CodiraRuntime codira_library_download)
else ()
    message(FATAL_ERROR "You must specify the `codira_binaries_path` or `codira_library_url` to be able to use the Codira Runtime")
endif ()

set_target_properties(CodiraRuntime PROPERTIES
    IMPORTED_LOCATION ${codira_library_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}codira_runtime${CMAKE_SHARED_LIBRARY_SUFFIX}
)

if (WIN32)
    set_target_properties(CodiraRuntime PROPERTIES
        IMPORTED_IMPLIB ${codira_library_dir}/codira_runtime${CMAKE_SHARED_LIBRARY_SUFFIX}.lib
    )
endif ()

if (codira_build_examples)
    add_subdirectory(examples)
endif ()

include(CTest)
if (codira_build_tests)
    add_subdirectory(${cpp_source_dir}/tests)
endif ()
