# Copyright (c) 2025-2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http:/www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specif(ic language governing permissions and
# limitations under the License.

project(pcre2)

option(PANDA_ETS_PCRE2_NEWLINE_PATCH "Enable ETS PCRE2 newline patch" ON)

set(PCRE2_DIR
    ${PANDA_THIRD_PARTY_SOURCES_DIR}/pcre2/pcre2/
)

set(PCRE2_SRC_DIR
    ${PCRE2_DIR}/src/
)

add_definitions(-DHAVE_CONFIG_H)

set(PCRE2_EBCDIC OFF CACHE BOOL
    "Use EBCDIC coding instead of ASCII. (This is rarely used outside of mainframe systems.)")

set(PCRE2_EBCDIC_NL25 OFF CACHE BOOL
    "Use 0x25 as EBCDIC NL character instead of 0x15; implies EBCDIC.")

set(PCRE2_LINK_SIZE "2" CACHE STRING
    "Internal link size (2, 3 or 4 allowed). See LINK_SIZE in config.h.in for details.")

set(PCRE2_PARENS_NEST_LIMIT "250" CACHE STRING
    "Default nested parentheses limit. See PARENS_NEST_LIMIT in config.h.in for details.")

set(PCRE2_HEAP_LIMIT "20000000" CACHE STRING
    "Default limit on heap memory (kibibytes). See HEAP_LIMIT in config.h.in for details.")

set(PCRE2_MATCH_LIMIT "10000000" CACHE STRING
    "Default limit on internal looping. See MATCH_LIMIT in config.h.in for details.")

set(PCRE2_MATCH_LIMIT_DEPTH "MATCH_LIMIT" CACHE STRING
    "Default limit on internal depth of search. See MATCH_LIMIT_DEPTH in config.h.in for details.")

set(PCRE2GREP_BUFSIZE "20480" CACHE STRING
    "Buffer starting size parameter for pcre2grep. See PCRE2GREP_BUFSIZE in config.h.in for details.")

set(PCRE2GREP_MAX_BUFSIZE "1048576" CACHE STRING
    "Buffer maximum size parameter for pcre2grep. See PCRE2GREP_MAX_BUFSIZE in config.h.in for details.")

set(PCRE2_NEWLINE "LF" CACHE STRING
    "What to recognize as a newline (one of CR, LF, CRLF, ANY, ANYCRLF, NUL).")

set(PCRE2_HEAP_MATCH_RECURSE OFF CACHE BOOL
    "Obsolete option: do not use")

set(PCRE2_SUPPORT_JIT OFF CACHE BOOL
    "Enable support for Just-in-time compiling.")

set(NEWLINE_DEFAULT "")

if(PCRE2_NEWLINE STREQUAL "CR")
    set(NEWLINE_DEFAULT "1")
endif(PCRE2_NEWLINE STREQUAL "CR")
if(PCRE2_NEWLINE STREQUAL "LF")
    set(NEWLINE_DEFAULT "2")
endif(PCRE2_NEWLINE STREQUAL "LF")
if(PCRE2_NEWLINE STREQUAL "CRLF")
    set(NEWLINE_DEFAULT "3")
endif(PCRE2_NEWLINE STREQUAL "CRLF")
if(PCRE2_NEWLINE STREQUAL "ANY")
    set(NEWLINE_DEFAULT "4")
endif(PCRE2_NEWLINE STREQUAL "ANY")
if(PCRE2_NEWLINE STREQUAL "ANYCRLF")
    set(NEWLINE_DEFAULT "5")
endif(PCRE2_NEWLINE STREQUAL "ANYCRLF")
if(PCRE2_NEWLINE STREQUAL "NUL")
    set(NEWLINE_DEFAULT "6")
endif(PCRE2_NEWLINE STREQUAL "NUL")

if(NEWLINE_DEFAULT STREQUAL "")
    message(FATAL_ERROR "The PCRE2_NEWLINE variable must be set to one of the following values: \"LF\", \"CR\", \"CRLF\", \"ANY\", \"ANYCRLF\".")
endif(NEWLINE_DEFAULT STREQUAL "")

if(PCRE2_EBCDIC)
    set(EBCDIC 1)
endif(PCRE2_EBCDIC)

if(PCRE2_EBCDIC_NL25)
    set(EBCDIC 1)
    set(EBCDIC_NL25 1)
endif(PCRE2_EBCDIC_NL25)

set(PCRE2_CFLAGS_CC
    -fPIC
)

if (PANDA_ETS_PCRE2_NEWLINE_PATCH)
    set(PCRE2_ORIG_NEWLINE ${PCRE2_SRC_DIR}/pcre2_newline.c)
    set(PCRE2_NEWLINE_PATCH
        ${PANDA_ROOT}/plugins/ets/runtime/patches/pcre2_newline.patch)

    add_custom_target(pcre2_newline_patch
        COMMAND ${CMAKE_COMMAND} -E echo "Applying patch to ${PCRE2_ORIG_NEWLINE}"
        COMMAND patch -N ${PCRE2_ORIG_NEWLINE} ${PCRE2_NEWLINE_PATCH} || true
        DEPENDS ${PCRE2_ORIG_NEWLINE} ${PCRE2_NEWLINE_PATCH}
        COMMENT "Applying patch to pcre2_newline.c"
    )
endif()

set(PCRE2_SOURCES
  ${PCRE2_SRC_DIR}/pcre2_auto_possess.c
  ${PCRE2_SRC_DIR}/pcre2_chartables.c
  ${PCRE2_SRC_DIR}/pcre2_chkdint.c
  ${PCRE2_SRC_DIR}/pcre2_compile.c
  ${PCRE2_SRC_DIR}/pcre2_config.c
  ${PCRE2_SRC_DIR}/pcre2_context.c
  ${PCRE2_SRC_DIR}/pcre2_convert.c
  ${PCRE2_SRC_DIR}/pcre2_dfa_match.c
  ${PCRE2_SRC_DIR}/pcre2_error.c
  ${PCRE2_SRC_DIR}/pcre2_extuni.c
  ${PCRE2_SRC_DIR}/pcre2_find_bracket.c
  ${PCRE2_SRC_DIR}/pcre2_jit_compile.c
  ${PCRE2_SRC_DIR}/pcre2_maketables.c
  ${PCRE2_SRC_DIR}/pcre2_match.c
  ${PCRE2_SRC_DIR}/pcre2_match_data.c
  ${PCRE2_SRC_DIR}/pcre2_newline.c
  ${PCRE2_SRC_DIR}/pcre2_ord2utf.c
  ${PCRE2_SRC_DIR}/pcre2_pattern_info.c
  ${PCRE2_SRC_DIR}/pcre2_script_run.c
  ${PCRE2_SRC_DIR}/pcre2_serialize.c
  ${PCRE2_SRC_DIR}/pcre2_string_utils.c
  ${PCRE2_SRC_DIR}/pcre2_study.c
  ${PCRE2_SRC_DIR}/pcre2_substitute.c
  ${PCRE2_SRC_DIR}/pcre2_substring.c
  ${PCRE2_SRC_DIR}/pcre2_tables.c
  ${PCRE2_SRC_DIR}/pcre2_ucd.c
  ${PCRE2_SRC_DIR}/pcre2_valid_utf.c
  ${PCRE2_SRC_DIR}/pcre2_xclass.c
)

set(PCRE2_HEADERS ${PROJECT_BINARY_DIR}/pcre2.h)

panda_target_include_directories(arkruntime_obj SYSTEM PUBLIC
    ${PCRE2_SRC_DIR}/
    ${PROJECT_BINARY_DIR}/
)

file(STRINGS ${PCRE2_DIR}/configure.ac
  configure_lines
  LIMIT_COUNT 50 # Read only the first 50 lines of the file
)

set(SEARCHED_VARIABLES "pcre2_major" "pcre2_minor" "pcre2_prerelease" "pcre2_date"
  "libpcre2_posix_version" "libpcre2_8_version" "libpcre2_16_version" "libpcre2_32_version")
foreach(configure_line ${configure_lines})
    foreach(_substitution_variable ${SEARCHED_VARIABLES})
        string(TOUPPER ${_substitution_variable} _substitution_variable_upper)
        if(NOT ${_substitution_variable_upper})
            string(REGEX MATCH "m4_define\\(${_substitution_variable}, *\\[(.*)\\]" MATCHED_STRING ${configure_line})
            if(CMAKE_MATCH_1)
                set(${_substitution_variable_upper} ${CMAKE_MATCH_1})
            endif()
        endif()
    endforeach()
endforeach()

macro(PARSE_LIB_VERSION VARIABLE_PREFIX)
  string(REPLACE ":" ";" ${VARIABLE_PREFIX}_VERSION_LIST ${${VARIABLE_PREFIX}_VERSION})
  list(GET ${VARIABLE_PREFIX}_VERSION_LIST 0 ${VARIABLE_PREFIX}_VERSION_CURRENT)
  list(GET ${VARIABLE_PREFIX}_VERSION_LIST 1 ${VARIABLE_PREFIX}_VERSION_REVISION)
  list(GET ${VARIABLE_PREFIX}_VERSION_LIST 2 ${VARIABLE_PREFIX}_VERSION_AGE)

  math(EXPR ${VARIABLE_PREFIX}_SOVERSION "${${VARIABLE_PREFIX}_VERSION_CURRENT} - ${${VARIABLE_PREFIX}_VERSION_AGE}")
  math(EXPR ${VARIABLE_PREFIX}_MACHO_COMPATIBILITY_VERSION "${${VARIABLE_PREFIX}_VERSION_CURRENT} + 1")
  math(EXPR ${VARIABLE_PREFIX}_MACHO_CURRENT_VERSION "${${VARIABLE_PREFIX}_VERSION_CURRENT} + 1")
  set(${VARIABLE_PREFIX}_MACHO_CURRENT_VERSION "${${VARIABLE_PREFIX}_MACHO_CURRENT_VERSION}.${${VARIABLE_PREFIX}_VERSION_REVISION}}")
  set(${VARIABLE_PREFIX}_VERSION "${${VARIABLE_PREFIX}_SOVERSION}.${${VARIABLE_PREFIX}_VERSION_AGE}.${${VARIABLE_PREFIX}_VERSION_REVISION}")
endmacro()

PARSE_LIB_VERSION(LIBPCRE2_POSIX)
PARSE_LIB_VERSION(LIBPCRE2_8)
PARSE_LIB_VERSION(LIBPCRE2_16)
PARSE_LIB_VERSION(LIBPCRE2_32)

add_dependencies(arkruntime_obj pcre2-8-static)
add_dependencies(arkruntime_obj pcre2-16-static)

SET(SUPPORT_UNICODE 1)

configure_file(${PCRE2_SRC_DIR}/pcre2.h.in ${PROJECT_BINARY_DIR}/pcre2.h @ONLY)
configure_file(${PCRE2_SRC_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)

configure_file(${PCRE2_DIR}/config-cmake.h.in
               ${PROJECT_BINARY_DIR}/config.h
               @ONLY)

configure_file(${PCRE2_DIR}/libpcre2-8.pc.in libpcre2-8.pc @ONLY)
configure_file(${PCRE2_DIR}/libpcre2-16.pc.in libpcre2-16.pc @ONLY)
configure_file(${PCRE2_DIR}/pcre2-config.in pcre2-config @ONLY)

panda_add_library(pcre2-8-static STATIC ${PCRE2_HEADERS} ${PCRE2_SOURCES} ${PROJECT_BINARY_DIR}/config.h)
set_target_properties(pcre2-8-static PROPERTIES
  COMPILE_DEFINITIONS PCRE2_CODE_UNIT_WIDTH=8
  MACHO_COMPATIBILITY_VERSION "${LIBPCRE2_8_MACHO_COMPATIBILITY_VERSION}"
  MACHO_CURRENT_VERSION "${LIBPCRE2_8_MACHO_CURRENT_VERSION}"
  VERSION ${LIBPCRE2_8_VERSION}
  SOVERSION ${LIBPCRE2_8_SOVERSION}
)
panda_target_compile_definitions(pcre2-8-static PUBLIC PCRE2_STATIC)
set(targets ${targets} pcre2-8-static)

if(MSVC)
  set_target_properties(pcre2-8-static PROPERTIES OUTPUT_NAME pcre2-8-static)
else(MSVC)
  set_target_properties(pcre2-8-static PROPERTIES OUTPUT_NAME pcre2-8)
endif(MSVC)

panda_add_library(pcre2-16-static STATIC ${PCRE2_HEADERS} ${PCRE2_SOURCES} ${PROJECT_BINARY_DIR}/config.h)
set_target_properties(pcre2-16-static PROPERTIES
  COMPILE_DEFINITIONS PCRE2_CODE_UNIT_WIDTH=16
  MACHO_COMPATIBILITY_VERSION "${LIBPCRE2_32_MACHO_COMPATIBILITY_VERSION}"
  MACHO_CURRENT_VERSION "${LIBPCRE2_32_MACHO_CURRENT_VERSION}"
  VERSION ${LIBPCRE2_16_VERSION}
  SOVERSION ${LIBPCRE2_16_SOVERSION})
panda_target_compile_definitions(pcre2-16-static PUBLIC PCRE2_STATIC)
set(targets ${targets} pcre2-16-static)
if (PANDA_ETS_PCRE2_NEWLINE_PATCH)
    add_dependencies(pcre2-8-static pcre2_newline_patch)
    add_dependencies(pcre2-16-static pcre2_newline_patch)
    panda_target_compile_definitions(pcre2-8-static PUBLIC ARK_PCRE2_NEWLINE_PATCH)
    panda_target_compile_definitions(pcre2-16-static PUBLIC ARK_PCRE2_NEWLINE_PATCH)
endif()
if(MSVC)
  set_target_properties(pcre2-16-static PROPERTIES OUTPUT_NAME pcre2-16-static)
else(MSVC)
  set_target_properties(pcre2-16-static PROPERTIES OUTPUT_NAME pcre2-16)
endif(MSVC)

panda_target_include_directories(pcre2-8-static SYSTEM PUBLIC
    ${PCRE2_SRC_DIR}/
    ${PROJECT_BINARY_DIR}/
)

panda_target_include_directories(pcre2-16-static SYSTEM PUBLIC
    ${PCRE2_SRC_DIR}/
    ${PROJECT_BINARY_DIR}/
)

panda_target_compile_options(pcre2-8-static
    PRIVATE
    ${PCRE2_CFLAGS_CC}
)

panda_target_compile_options(pcre2-16-static
    PRIVATE
    ${PCRE2_CFLAGS_CC}
)

panda_target_link_libraries(arkruntime_obj pcre2-8-static)
panda_target_link_libraries(arkruntime_obj pcre2-16-static)
