cmake_minimum_required(VERSION 3.14.1)
project(cangjieCodeCheck)

if (MSVC AND ${CMAKE_VERSION} VERSION_GREATER "3.14")
    cmake_policy(SET CMP0091 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CODELINT_VERSION_STR "1.1.0-alpha.70")
set(CMAKE_CXX_FLAGS_RELEASE "-D_FORTIFY_SOURCE=2 -O2")

if (CODIRA_HOME)
    message(STATUS "CODIRA_HOME (from -D): ${CODIRA_HOME}")
elseif (DEFINED ENV{CODIRA_HOME})
    set(CODIRA_HOME $ENV{CODIRA_HOME})
    message(STATUS "CODIRA_HOME: ${CODIRA_HOME}")
else()
    message("Please specify the path to the Codira SDK via the environment variable CODIRA_HOME or by using the -DCODIRA_HOME option.")
endif()

if(WIN32)
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
endif()
string(TOLOWER ${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}_codenative output_lib_dir)


if (NOT APPLE AND NOT WIN32 AND NOT CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -ftrapv -fPIE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s -Wl,-z,relro,-z,now,-z,noexecstack -Wl,-Bsymbolic -rdynamic -Wl,--no-undefined")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s -pie -Wl,-z,relro,-z,now,-z,noexecstack -Wl,-Bsymbolic -rdynamic -Wl,--no-undefined")
    SET(CMAKE_SKIP_RPATH TRUE)
    SET(CMAKE_SKIP_BUILD_RPATH TRUE)
endif ()

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -ftrapv -fPIE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-insert-timestamp")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-insert-timestamp")
endif()

if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -ftrapv -fPIE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s -pie")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s -pie")
    SET(CMAKE_SKIP_RPATH TRUE)
    SET(CMAKE_SKIP_BUILD_RPATH TRUE)
endif ()

if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DNDEBUG)
endif ()

if (CODIRA_WIN_NATIVE_DEV)
    add_compile_definitions(CODIRA_WIN_NATIVE_DEV)
    message(STATUS "Building for MinGW Windows (Developing)")
endif()

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
    add_definitions(-D__ARM__)
endif()

if(NOT CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
    set(CXX_SYSTEM_INCLUDE_CONFIGURATION_FLAG /experimental:external /external:W0)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX /external:I)
endif()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif ()

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE AND
        NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE)$")
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif ()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(COVERAGE_FLAG "Do coverage" OFF)

add_compile_definitions(CODIRA_AST2CHIR)
add_compile_definitions(CODIRA_CODEGEN_CODENATIVE_BACKEND)

if(GCC_TOOLCHAIN_FLAG)
    add_definitions(--gcc-toolchain="${GCC_TOOLCHAIN_FLAG}")
endif()

foreach(libpath ${CODIRA_TARGET_LIB})
    add_link_options("-L${libpath}")
endforeach()

add_compile_options(${CXX_SYSTEM_INCLUDE_CONFIGURATION_FLAG})

if(WIN32)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
    set(LIB_SEC ssp)
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

if(NOT WIN32)
    set(LIB_DL dl)
endif()

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
set(CODIRA_ROOT ${THIRD_PARTY_DIR}/cangjie)
set(CODEC_DIR ${CODIRA_ROOT}/bin)
set(JSON_ROOT ${THIRD_PARTY_DIR}/json-v3.11.3)

LIST(APPEND labList
        pthread
        ${LIB_SSP}
        ${LIB_DL}
        )

if(COVERAGE_FLAG)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    add_compile_definitions(CODIRA_ENABLE_GCOV)
endif()

add_compile_definitions(CODELINT_VERSION="${CODELINT_VERSION_STR}")
message(STATUS "Codira Lint: ${CODELINT_VERSION_STR}")

# Obtain CODEC version
message(STATUS "Obtain codec version")
execute_process(COMMAND bash -c "./codec -v" OUTPUT_VARIABLE CODEC_VERSION ERROR_VARIABLE ERROR_MSG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY "${CODEC_DIR}")
if (ERROR_MSG)
    message(STATUS "Obtain codec version failed in directory ${CODEC_DIR}: ${ERROR_MSG}")
else()
    message(STATUS "${CODEC_VERSION}") # CODEC_VERSION comes with "Codira version"
    add_compile_definitions(CODEC_VERSION="${CODEC_VERSION}")
endif ()

if (BUILD_SHARED_LIBS)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif ()
if (CHAR_VERSION)
    add_compile_definitions(VERSION_TAIL="${CHAR_VERSION}")
endif ()

set(JSON_HEADERS ${JSON_ROOT}/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${JSON_HEADERS})
include_directories(${CODIRA_HOME}/include)

if(WIN32 AND CMAKE_BUILD_TYPE MATCHES Debug)
    set(WIN_DEBUG_USE_STATIC_LIB ON)
endif()
if(WIN_DEBUG_USE_STATIC_LIB)
    add_library(cangjie-lsp STATIC IMPORTED)
    set_target_properties(cangjie-lsp PROPERTIES
        IMPORTED_LOCATION ${CODIRA_HOME}/tools/lib/libcangjie-lsp.a
    )
else()
    add_library(cangjie-lsp SHARED IMPORTED)
    set_target_properties(cangjie-lsp PROPERTIES
        IMPORTED_LOCATION ${CODIRA_HOME}/tools/lib/libcangjie-lsp.so
    )
    set_target_properties(cangjie-lsp PROPERTIES
        IMPORTED_IMPLIB ${CODIRA_HOME}/tools/lib/libcangjie-lsp.dll.a
    )
endif()

if(APPLE)
    set_target_properties(cangjie-lsp PROPERTIES
    IMPORTED_LOCATION ${CODIRA_HOME}/tools/lib/libcangjie-lsp.dylib
    )
endif()

add_subdirectory(config)
add_subdirectory(src)
