# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

function(gen_intrinsics_yaml)
    set(prefix ARG)
    set(noValues)
    set(singleValues OUTPUTFILE TARGET)
    set(multiValues DATAFILES DEPENDS)
    cmake_parse_arguments(${prefix}
            "${noValues}"
            "${singleValues}"
            "${multiValues}"
            ${ARGN})
    set(GENERATOR ${CMAKE_CURRENT_SOURCE_DIR}/templates/gen_intrinsics_data.rb)
    set(TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/templates/intrinsics.yaml.erb)
    set(REQUIRES ${CMAKE_CURRENT_SOURCE_DIR}/templates/runtime.rb ${PANDA_ROOT}/libarkbase/utils.rb)
    set(DEPENDS_LIST ${GENERATOR} ${TEMPLATE} ${ARG_DEPENDS})
    string(REPLACE ";" "," DATAFILES_STR "${ARG_DATAFILES}")
    foreach(d ${ARG_DATAFILES})
        list(APPEND DEPENDS_LIST ${d})
    endforeach()
    add_custom_command(OUTPUT ${ARG_OUTPUTFILE}
            COMMENT "Generate intrinsics.yaml"
            COMMAND ${GENERATOR} -d ${DATAFILES_STR} -t ${TEMPLATE} -o ${ARG_OUTPUTFILE} -r ${REQUIRES}
            DEPENDS ${DEPENDS_LIST})
    add_custom_target(${ARG_TARGET} ALL DEPENDS ${ARG_OUTPUTFILE})
endfunction()

set(DATAFILES_TARGET ${CMAKE_CURRENT_SOURCE_DIR}/runtime.yaml ${IRTOC_INTRINSICS_YAML})

foreach(plugin ${PLUGINS})
    string(TOUPPER ${plugin} plugin_name_upper)
    string(CONCAT PLUGIN_RUNTIME_YAML ${plugin_name_upper} "_RUNTIME_YAML")
    set(${PLUGIN_RUNTIME_YAML} "")

    string(CONCAT PANDA_WITH_PLUGIN "PANDA_WITH_" ${plugin_name_upper})
    if (${${PANDA_WITH_PLUGIN}})
        string(CONCAT PLUGIN_SOURCE "PANDA_" ${plugin_name_upper} "_PLUGIN_SOURCE")
        if(EXISTS ${${PLUGIN_SOURCE}}/intrinsics.cmake)
            include(${${PLUGIN_SOURCE}}/intrinsics.cmake)
        endif()
    endif()
    
    list(APPEND DATAFILES_TARGET ${${PLUGIN_RUNTIME_YAML}})
endforeach()

set(INTRINSICS_YAML ${CMAKE_CURRENT_BINARY_DIR}/intrinsics.yaml)
set(INTRINSICS_TARGET arkruntime_gen_intrinsics_yaml)

gen_intrinsics_yaml(
    TARGET ${INTRINSICS_TARGET}
    DATAFILES ${DATAFILES_TARGET}
    OUTPUTFILE ${INTRINSICS_YAML}
)

set(RUNTIME_TEMPLATES
    intrinsics_gen.h.erb
    intrinsics.h.erb
    intrinsics_enum.h.erb
    unimplemented_intrinsics-inl.cpp.erb
    )

panda_gen(
    DATA ${INTRINSICS_YAML}
    TARGET_NAME intrinsics_gen_arkruntime
    TEMPLATES ${RUNTIME_TEMPLATES}
    API ${CMAKE_CURRENT_SOURCE_DIR}/templates/intrinsics.rb
    REQUIRES ${PANDA_ROOT}/libarkbase/utils.rb
    SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/templates
    DESTINATION ${GEN_INCLUDE_DIR}
    EXTRA_DEPENDENCIES ${INTRINSICS_TARGET}
)
