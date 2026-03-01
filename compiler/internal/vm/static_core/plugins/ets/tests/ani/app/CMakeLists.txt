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

cmake_minimum_required(VERSION 3.10)

project(ani_app)


set(ANI_APP_SOURCES
    main.cpp
)

panda_add_executable(ani_app ${ANI_APP_SOURCES})

panda_target_link_libraries(ani_app arkruntime)

# Build rules for ani_app_lib native shared library
panda_add_library(ani_app_lib SHARED plugin/plugin.cpp)
panda_target_link_libraries(ani_app_lib ani_interface)
add_dependencies(ani_app ani_app_lib)

panda_add_sanitizers(TARGET ani_app SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_add_sanitizers(TARGET ani_app_lib SANITIZERS ${PANDA_SANITIZERS_LIST})

set(ANI_APP_ETS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/interface/ability.ets
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/plugin.ets
    ${CMAKE_CURRENT_SOURCE_DIR}/app/app.ets
)
compile_arktsconfig_unit(ani_app_managed_code APP_ABC_FILES ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.json
                         "${ANI_APP_ETS_SOURCES}"
)

add_dependencies(ani_app ani_app_managed_code)

list(JOIN APP_ABC_FILES ":" APP_ABC_FILES)

# NOTE(@srokashevich, #29189): enable with asan after fix
if (NOT PANDA_ENABLE_ADDRESS_SANITIZER)
    # Create a custom target to run ani_app after build
    add_custom_target(run_ani_app
        COMMAND ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
                APP_ABC_FILES=${APP_ABC_FILES}
                LD_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            ${PANDA_RUN_PREFIX} $<TARGET_FILE:ani_app>
        DEPENDS ani_app ani_app_lib ani_app_managed_code
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running ani_app with compiled ABC files"
    )

    # Add the run target as a dependency of the main test target
    add_dependencies(ani_tests run_ani_app)
endif()
