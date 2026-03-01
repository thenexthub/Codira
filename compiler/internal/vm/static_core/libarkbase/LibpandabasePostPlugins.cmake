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

set(LOGGER_FILE "${CMAKE_BINARY_DIR}/libarkbase/logger.yaml")
set(LOGGER_CORE_FILE "${PROJECT_SOURCE_DIR}/libarkbase/templates/logger.yaml")
set(LOGGER_GENERATOR "${PROJECT_SOURCE_DIR}/libarkbase/templates/logger_gen.rb")

add_custom_command(OUTPUT ${LOGGER_FILE}
    COMMENT "Generate logger.yaml"
    COMMAND ${LOGGER_GENERATOR} -d "${LOGGER_CORE_FILE}" -p "${GEN_PLUGIN_OPTIONS_YAML}" -o ${LOGGER_FILE}
    DEPENDS ${LOGGER_GENERATOR} ${LOGGER_CORE_FILE} plugin_options_merge
)
add_custom_target(logger_yaml_gen DEPENDS ${LOGGER_FILE})

set(LOGGER_TEMPLATES
    logger_enum_gen.h.erb
    logger_impl_gen.inc.erb
)

add_custom_target(logger_gen)

foreach(TEMPLATE ${LOGGER_TEMPLATES})
    get_filename_component(OUTPUT_FILENAME ${TEMPLATE} NAME_WLE)
    set(OUTPUT ${CMAKE_BINARY_DIR}/libarkbase/generated/${OUTPUT_FILENAME})

    panda_gen_file(
        DATA ${LOGGER_FILE}
        TEMPLATE ${PROJECT_SOURCE_DIR}/libarkbase/templates/${TEMPLATE}
        OUTPUTFILE ${OUTPUT}
        API ${PROJECT_SOURCE_DIR}/libarkbase/templates/logger.rb
        EXTRA_DEPENDENCIES logger_yaml_gen
    )
    add_custom_target(logger_gen_${OUTPUT_FILENAME} DEPENDS ${OUTPUT})
    add_dependencies(logger_gen logger_gen_${OUTPUT_FILENAME})
endforeach()

add_dependencies(panda_gen_files logger_gen)

add_dependencies(arkassembler assembler_extensions)

if(PANDA_ENABLE_LTO AND PANDA_ARKBASE_LTO)
    set(ARKBASE_TARGETS arkbase_obj arkbase_lto)
else()
    set(ARKBASE_TARGETS arkbase_obj)
endif()

set(SOURCE_LANGUAGE_H ${CMAKE_BINARY_DIR}/libarkbase/generated/source_language.h)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PROJECT_SOURCE_DIR}/libarkbase/templates/source_language.h.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${SOURCE_LANGUAGE_H}
)
add_custom_target(source_language_gen DEPENDS ${SOURCE_LANGUAGE_H})
add_dependencies(panda_gen_files source_language_gen)

set(PLUGINS_REGMASKS_INL ${CMAKE_BINARY_DIR}/libarkbase/generated/plugins_regmasks.inl)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PROJECT_SOURCE_DIR}/libarkbase/templates/plugins_regmasks.inl.erb
    API ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${PLUGINS_REGMASKS_INL}
)
add_custom_target(plugins_regmasks_gen DEPENDS ${PLUGINS_REGMASKS_INL})
add_dependencies(panda_gen_files plugins_regmasks_gen)

foreach(TARGET ${ARKBASE_TARGETS})
    add_dependencies(${TARGET} logger_gen source_language_gen plugins_regmasks_gen)
endforeach()
