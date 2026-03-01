#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# Generate files based on templates and YAML data provided.
# Adds targets for every template. Also adds a target for the whole function invocation
# with name ${data_name}_gen_${PROJECT_NAME} for ease of declaring dependencies on generated files.
#
# Mandatory arguments:
# * DATA -- a list of data sources, YAML files
# * API -- a list of Ruby scripts that provide data-querying API for templates
#   (Nth script from API should parse Nth YAML file from DATA)
# * TEMPLATES -- a list of templates to generate files
#
# Optional arguments:
# * SOURCE -- a directory with templates, default is ${PROJECT_SOURCE_DIR}/templates
# * DESTINATION -- a directory for output files, default is ${PANDA_BINARY_ROOT}
# * REQUIRES -- if defined, will require additional Ruby files for template generation
# * EXTRA_DEPENDENCIES -- a list of files that should be considered as dependencies
# * EXTRA_ARGV -- a list of positional arguments that could be accessed in '.erb' files via ARGV[]

function(panda_gen)
    set(singlevalues SOURCE DESTINATION TARGET_NAME)
    set(multivalues DATA API TEMPLATES REQUIRES EXTRA_DEPENDENCIES EXTRA_ARGV)
    cmake_parse_arguments(
        GEN_ARG
        ""
        "${singlevalues}"
        "${multivalues}"
        ${ARGN}
    )

    if (NOT DEFINED GEN_ARG_TEMPLATES)
        message(FATAL_ERROR "`TEMPLATES` were not passed to `panda_gen` function")
    endif()

    if (NOT DEFINED GEN_ARG_DATA)
        message(FATAL_ERROR "`DATA` was not passed to `panda_gen` function")
    endif()

    if (NOT DEFINED GEN_ARG_API)
        message(FATAL_ERROR "`API` was not passed to `panda_gen` function")
    endif()

    if (NOT DEFINED GEN_ARG_SOURCE)
        set(GEN_ARG_SOURCE "${PROJECT_SOURCE_DIR}/templates")
    endif()

    if (NOT DEFINED GEN_ARG_DESTINATION)
        set(GEN_ARG_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    if (NOT DEFINED GEN_ARG_TARGET_NAME)
        get_filename_component(DATA_NAME ${GEN_ARG_DATA} NAME_WE)
        set(GEN_ARG_TARGET_NAME "${DATA_NAME}_gen_${PROJECT_NAME}")
    endif()

    add_custom_target(${GEN_ARG_TARGET_NAME}) # Umbrella target for all generated files
    add_dependencies(panda_gen_files ${GEN_ARG_TARGET_NAME})

    foreach(t ${GEN_ARG_TEMPLATES})
        set(TEMPLATE "${GEN_ARG_SOURCE}/${t}")
        string(REGEX REPLACE "\.erb$" "" NAME ${t})
        string(REPLACE "\." "_" TARGET ${NAME})
        string(REPLACE "/" "_" TARGET ${TARGET})
        set(TARGET ${PROJECT_NAME}_${TARGET})
        set(OUTPUT_FILE "${GEN_ARG_DESTINATION}/${NAME}")

        panda_gen_file(DATA ${GEN_ARG_DATA}
            API ${GEN_ARG_API}
            TEMPLATE ${TEMPLATE}
            OUTPUTFILE ${OUTPUT_FILE}
            REQUIRES ${GEN_ARG_REQUIRES}
            EXTRA_DEPENDENCIES ${GEN_ARG_EXTRA_DEPENDENCIES}
            EXTRA_ARGV ${GEN_ARG_EXTRA_ARGV}
        )
        add_custom_target(${TARGET} DEPENDS ${OUTPUT_FILE})
        add_dependencies(${GEN_ARG_TARGET_NAME} ${TARGET})
    endforeach()
endfunction()

# Calls `panda_gen` for ISA YAML.
# Adds targets for every template. Also adds a target for the whole function invocation
# with name isa_gen_${PROJECT_NAME} for ease of declaring dependencies on generated files.
#
# Mandatory arguments:
# * TEMPLATES -- a list of templates to generate files
#
# Optional arguments:
# * SOURCE -- a directory with templates, default is ${PROJECT_SOURCE_DIR}/templates
# * DESTINATION -- a directory for output files, default is ${PANDA_BINARY_ROOT}
# * REQUIRES -- if defined, will require additional Ruby files for template generation
# * EXTRA_DEPENDENCIES -- a list of files that should be considered as dependencies

function(panda_isa_gen)
    set(singlevalues SOURCE DESTINATION TARGET_NAME)
    set(multivalues TEMPLATES REQUIRES EXTRA_DEPENDENCIES)
    cmake_parse_arguments(
        ISA_GEN_ARG
        ""
        "${singlevalues}"
        "${multivalues}"
        ${ARGN}
    )
    set(ISA_DATA "${CMAKE_BINARY_DIR}/isa/isa.yaml")
    set(ISAPI "${PANDA_ROOT}/isa/isapi.rb")
    list(APPEND ISA_GEN_ARG_EXTRA_DEPENDENCIES isa_assert)
    panda_gen(DATA ${ISA_DATA}
        API ${ISAPI}
        TEMPLATES ${ISA_GEN_ARG_TEMPLATES}
        SOURCE ${ISA_GEN_ARG_SOURCE}
        TARGET_NAME ${ISA_GEN_ARG_TARGET_NAME}
        DESTINATION ${ISA_GEN_ARG_DESTINATION}
        REQUIRES ${ISA_GEN_ARG_REQUIRES}
        EXTRA_DEPENDENCIES ${ISA_GEN_ARG_EXTRA_DEPENDENCIES}
    )
endfunction()

# Generate file for a template and YAML data provided.
#
# Mandatory arguments:
# * DATA -- a list of data sources, YAML files
# * API -- a list of Ruby scripts that provide data-querying API for templates
#   (Nth script from API should parse Nth YAML file from DATA)
# * TEMPLATE -- template full name
# * OUTPUTFILE -- output file full name
#
# Optional arguments:
# * REQUIRES -- if defined, will require additional Ruby files for template generation
# * EXTRA_DEPENDENCIES -- a list of files that should be considered as dependencies
# * EXTRA_ARGV -- a list of positional arguments that could be accessed in '.erb' files via ARGV[]

function(panda_gen_file)
    set(singlevalues TEMPLATE OUTPUTFILE)
    set(multivalues DATA API REQUIRES EXTRA_DEPENDENCIES EXTRA_ARGV)
    cmake_parse_arguments(
        ARG
        ""
        "${singlevalues}"
        "${multivalues}"
        ${ARGN}
    )
    if (NOT DEFINED ARG_TEMPLATE)
        message(FATAL_ERROR "`TEMPLATE` was not passed to `panda_gen_file` function")
    endif()
    if (NOT DEFINED ARG_DATA)
        message(FATAL_ERROR "`DATA` was not passed to `panda_gen_file` function")
    endif()
    if (NOT DEFINED ARG_API)
        message(FATAL_ERROR "`API` was not passed to `panda_gen_file` function")
    endif()
    set(GENERATOR "${PANDA_ROOT}/isa/gen.rb")
    string(REPLACE ";" "," DATA_STR "${ARG_DATA}")
    string(REPLACE ";" "," API_STR "${ARG_API}")
    if(DEFINED ARG_REQUIRES)
        string(REPLACE ";" "," REQUIRE_STR "${ARG_REQUIRES}")
        set(REQUIRE_OPTION --require ${REQUIRE_STR})
    endif()
    set(DEPENDS_LIST ${GENERATOR} ${ARG_TEMPLATE} ${ARG_DATA} ${ARG_API})
    

    add_custom_command(OUTPUT ${ARG_OUTPUTFILE}
        COMMENT "Generate file for ${ARG_TEMPLATE}"
        COMMAND ${GENERATOR} ${ARG_EXTRA_ARGV} --template ${ARG_TEMPLATE} --data ${DATA_STR} --api ${API_STR}  --target ${CMAKE_SYSTEM_PROCESSOR} --output ${ARG_OUTPUTFILE} ${REQUIRE_OPTION}
        DEPENDS ${DEPENDS_LIST} ${ARG_EXTRA_DEPENDENCIES}
    )
endfunction()

# Create an options header using a YAML file for the target
#
# Mandatory arguments:
# TARGET -- target
# YAML_FILE -- YAML file
# GENERATED_HEADER -- generated header
#
# Use "#include 'generated/GENERATED_HEADER"' to include the generated header

function(panda_gen_options)
    # Parsing function arguments
    set(singlevalues TARGET YAML_FILE GENERATED_HEADER)
    cmake_parse_arguments(GEN_OPTIONS "" "${singlevalues}" "" ${ARGN})

    # Generate a options header
    get_filename_component(YAML_FILE ${GEN_OPTIONS_YAML_FILE} ABSOLUTE)
    set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/panda_gen_options/generated)
    file(MAKE_DIRECTORY ${GENERATED_DIR})
    set(OPTIONS_H ${GENERATED_DIR}/${GEN_OPTIONS_GENERATED_HEADER})
    panda_gen_file(
        DATA ${YAML_FILE}
        TEMPLATE ${PANDA_ROOT}/templates/options/options.h.erb
        OUTPUTFILE ${OPTIONS_H}
        API ${PANDA_ROOT}/templates/common.rb
    )

    # Add dependencies for a target
    panda_target_include_directories(${GEN_OPTIONS_TARGET} PUBLIC ${GENERATED_DIR}/..)
    add_custom_target(${GEN_OPTIONS_TARGET}_options DEPENDS ${OPTIONS_H})
    add_dependencies(${GEN_OPTIONS_TARGET} ${GEN_OPTIONS_TARGET}_options)
    add_dependencies(panda_gen_files ${GEN_OPTIONS_TARGET}_options)
endfunction()

function(panda_gen_messages)
    set(singlevalues TARGET YAML_FILE GENERATED_HEADER)
    cmake_parse_arguments(ARG "" "${singlevalues}" "" ${ARGN})

    if(NOT DEFINED ARG_YAML_FILE)
        set(ARG_YAML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/messages.yaml)
    endif()

    if(NOT DEFINED ARG_GENERATED_HEADER)
        set(ARG_GENERATED_HEADER messages.h)
    endif()

    get_filename_component(YAML_FILE ${ARG_YAML_FILE} ABSOLUTE)

    if(IS_ABSOLUTE ${ARG_GENERATED_HEADER})
        get_filename_component(GENERATED_DIR ${ARG_GENERATED_HEADER} DIRECTORY)
        set(MESSAGES_H ${ARG_GENERATED_HEADER})
    else()
        set(INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/panda_gen_messages)
        set(GENERATED_DIR ${INCLUDE_DIR}/generated)
        set(MESSAGES_H ${GENERATED_DIR}/${ARG_GENERATED_HEADER})
    endif()

    file(MAKE_DIRECTORY ${GENERATED_DIR})
    panda_gen_file(
        DATA ${YAML_FILE}
        TEMPLATE ${PANDA_ROOT}/templates/messages/messages.h.erb
        OUTPUTFILE ${MESSAGES_H}
        API ${PANDA_ROOT}/templates/messages.rb
    )

    # Add dependencies for a target
    if (NOT DEFINED ARG_TARGET)
        set(ARG_TARGET messages_gen_${PROJECT_NAME})
        add_custom_target(${ARG_TARGET})
    endif()

    if (DEFINED INCLUDE_DIR)
        panda_target_include_directories(${ARG_TARGET} PUBLIC ${INCLUDE_DIR})
    endif()
    add_custom_target(${ARG_TARGET}_messages DEPENDS ${MESSAGES_H})
    add_dependencies(${ARG_TARGET} ${ARG_TARGET}_messages)
    add_dependencies(panda_gen_files ${ARG_TARGET}_messages)
endfunction()

add_custom_target(plugin_options_gen)
set_target_properties(plugin_options_gen PROPERTIES PLUGIN_OPTIONS_YAML_FILES "${PANDA_ROOT}/templates/plugin_options.yaml")

add_custom_target(entrypoints_yaml_gen)
set_target_properties(entrypoints_yaml_gen PROPERTIES ENTRYPOINT_YAML_FILES "${PANDA_ROOT}/runtime/entrypoints/entrypoints.yaml")

add_custom_target(runtime_options_gen)
set_target_properties(runtime_options_gen PROPERTIES RUNTIME_OPTIONS_YAML_FILES "${PANDA_ROOT}/runtime/options.yaml")

add_custom_target(compiler_options_gen)
set_target_properties(compiler_options_gen PROPERTIES COMPILER_OPTIONS_YAML_FILES "${PANDA_ROOT}/compiler/compiler.yaml")

function(add_plugin_options YAML_FILE_PATH)
    get_target_property(YAML_FILES plugin_options_gen PLUGIN_OPTIONS_YAML_FILES)
    list(APPEND YAML_FILES ${YAML_FILE_PATH})
    set_target_properties(plugin_options_gen PROPERTIES PLUGIN_OPTIONS_YAML_FILES "${YAML_FILES}")
endfunction()

function(add_entrypoints_yaml YAML_FILE_PATH)
    get_target_property(YAML_FILES entrypoints_yaml_gen ENTRYPOINT_YAML_FILES)
    list(APPEND YAML_FILES ${YAML_FILE_PATH})
    set_target_properties(entrypoints_yaml_gen PROPERTIES ENTRYPOINT_YAML_FILES "${YAML_FILES}")
endfunction()

function(add_runtime_options YAML_FILE_PATH)
    get_target_property(YAML_FILES runtime_options_gen RUNTIME_OPTIONS_YAML_FILES)
    list(APPEND YAML_FILES ${YAML_FILE_PATH})
    set_target_properties(runtime_options_gen PROPERTIES RUNTIME_OPTIONS_YAML_FILES "${YAML_FILES}")
endfunction()

function(add_compiler_options YAML_FILE_PATH)
    get_target_property(YAML_FILES compiler_options_gen COMPILER_OPTIONS_YAML_FILES)
    list(APPEND YAML_FILES ${YAML_FILE_PATH})
    set_target_properties(compiler_options_gen PROPERTIES COMPILER_OPTIONS_YAML_FILES "${YAML_FILES}")
endfunction()
