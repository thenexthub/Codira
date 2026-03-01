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

macro(LISTPLUGINS result plugin_dir)
  file(GLOB children RELATIVE ${plugin_dir} ${plugin_dir}/*)
  set(plugin_list "")
  foreach(child ${children})
    if (IS_DIRECTORY ${plugin_dir}/${child})
        set(plugin_list ${plugin_list} ${child})
    endif()
  endforeach()
  set(${result} ${plugin_list})
endmacro()

function(register_plugin plugin_name)
    string(TOUPPER ${plugin_name} plugin_name_upper)

    string(CONCAT PANDA_WITH_PLUGIN "PANDA_WITH_" ${plugin_name_upper})
    string(CONCAT PANDA_WITH_PLUGIN "PANDA_WITH_" ${plugin_name_upper})
    
    set(PLUGIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/plugins/${plugin_name}")
    
    if (DEFINED ${PANDA_WITH_PLUGIN} AND NOT ${PANDA_WITH_PLUGIN})
        message(STATUS "${PANDA_WITH_PLUGIN} = ${${PANDA_WITH_PLUGIN}}")
        return()
    else()
        include("${PLUGIN_SOURCE_DIR}/RegisterPlugin.cmake")
        if ((PANDA_PRODUCT_BUILD AND NOT PLUGIN_IN_PRODUCT_BUILD) AND NOT ${PANDA_WITH_PLUGIN})
            option(${PANDA_WITH_PLUGIN} "Enable ${plugin_name} plugin" OFF)
        else()
            option(${PANDA_WITH_PLUGIN} "Enable ${plugin_name} plugin" ON)
        endif()
    endif()

    message(STATUS "${PANDA_WITH_PLUGIN} = ${${PANDA_WITH_PLUGIN}}")

    string(CONCAT PLUGIN_SOURCE "PANDA_" ${plugin_name_upper} "_PLUGIN_SOURCE")
    string(CONCAT PLUGIN_BINARY "PANDA_" ${plugin_name_upper} "_PLUGIN_BINARY")
    set(${PLUGIN_SOURCE} "${PLUGIN_SOURCE_DIR}" PARENT_SCOPE)
    set(${PLUGIN_BINARY} "${CMAKE_CURRENT_BINARY_DIR}/plugins/${plugin_name}" PARENT_SCOPE)

    panda_promote_to_definitions(${PANDA_WITH_PLUGIN})
endfunction()

LISTPLUGINS(PLUGINS ${CMAKE_CURRENT_SOURCE_DIR}/plugins)

foreach(plugin ${PLUGINS})
    register_plugin(${plugin})
endforeach()

# Merge Plugins machinery. Example below.
#
# ===== Code in core part =====
# [some_component/CMakeLists.txt]
#   declare_plugin_file("get_word_size.h")
#
# [some_core_file.cpp]
#   void GetWordSize(Language lang) {
#     switch (lang) {
#       #include "some_component/generated/get_word_size.h"
#     }
#   }
#
# ===== Code in some plugin =====
# [CMakeLists.txt]
#   add_merge_plugin(PLUGIN_NAME "get_word_size.h" INPUT_FILE "plugin_get_word_size.h")
#
# [plugin_get_word_size.h]
#   case (Language::PLUGIN_LANGUAGE)
#       return 4;
#   break;
#
# After that, content of `some_component/generated/get_word_size.h` file will be filled by all files passed via `add_merge_plugin`.
# In our case it will be just one file `plugin_get_word_size.h`.

add_custom_target(merge_plugins)
set_target_properties(merge_plugins PROPERTIES PLUGINS "")

# Declare plugin file for merge_plugins machinery.
function(declare_plugin_file NAME)
    string(REGEX REPLACE "[/\\.]" "_" plugin_files_target "${NAME}__files")
    add_custom_target(${plugin_files_target})
    set_target_properties(${plugin_files_target} PROPERTIES
        PLUGIN_FILES ""
        GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

    get_target_property(PLUGINS merge_plugins PLUGINS)
    list(APPEND PLUGINS ${NAME})
    set_target_properties(merge_plugins PROPERTIES PLUGINS "${PLUGINS}")

    add_dependencies(merge_plugins ${plugin_files_target})
endfunction()

# Add file to embedded plugin
# Arguments:
#   [in] PLUGIN_NAME - name of the plugin, where file should be added
#   [in] INPUT_FILE - path to file to be added to the plugin
function(add_merge_plugin)
    set(prefix ARG)
    set(singleValues PLUGIN_NAME INPUT_FILE)
    cmake_parse_arguments(${prefix} "" "${singleValues}" "${multiValues}" ${ARGN})

    string(REGEX REPLACE "[/\\.]" "_" plugin_files_target "${ARG_PLUGIN_NAME}__files")
    if (NOT TARGET ${plugin_files_target})
        message(FATAL_ERROR "Plugin file '${ARG_PLUGIN_NAME}' was not declared!")
    endif()

    get_target_property(FILES ${plugin_files_target} PLUGIN_FILES)
    list(APPEND FILES ${ARG_INPUT_FILE})
    set_target_properties(${plugin_files_target} PROPERTIES PLUGIN_FILES "${FILES}")
endfunction()
