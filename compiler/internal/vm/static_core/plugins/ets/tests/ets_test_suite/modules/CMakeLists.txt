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

if (PANDA_TARGET_ARM32)
    return()
endif()

function(compile_pandasm_source ABC_TARGET ABC_FILES WORK_DIR PA_SOURCES)
    add_custom_target(${ABC_TARGET})
    set(RESULT "")
    foreach(PA_SOURCE ${PA_SOURCES})
        get_filename_component(SOURCE_FNAME ${PA_SOURCE} NAME_WE)
        set(SOURCE_ABC ${WORK_DIR}/${ABC_TARGET}-${SOURCE_FNAME}.abc)
        set(SOURCE_ABC_TARGET ${ABC_TARGET}-${SOURCE_FNAME}-ets-asm)
        add_custom_command(OUTPUT "${SOURCE_ABC}"
                COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark_asm> ${PA_SOURCE} ${SOURCE_ABC}
                DEPENDS ${assembler} "${PA_SOURCE}"
                WORKING_DIRECTORY "${ARG_WORKING_DIR}")
        set(RESULT ${RESULT} ${SOURCE_ABC})
        add_custom_target(${SOURCE_ABC_TARGET} DEPENDS ${SOURCE_ABC})
        add_dependencies(${ABC_TARGET} ${SOURCE_ABC_TARGET})
    endforeach()
    set(${ABC_FILES} ${RESULT} PARENT_SCOPE)
endfunction()

function(build_spawn_native_module TARGET OUTPUT_DIR)
    panda_add_library(${TARGET} SHARED spawn/spawn.cpp)
    set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
    panda_target_link_libraries(${TARGET} arkruntime)
    panda_target_include_directories(${TARGET} PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani)
endfunction()

function(build_spawn_managed_code TARGET ABC_FILES WORK_DIR)
    set(oneValueArgs OPT_LEVEL)
    set(multiValueArgs )
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(SPAWN_ETS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/spawn/spawn.ets;
        ${CMAKE_CURRENT_SOURCE_DIR}/spawn/ability.ets;
    )
    compile_arktsconfig_unit(${TARGET} RESULT ${WORK_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/spawn/arktsconfig.json
                             "${SPAWN_ETS_SOURCES}"
                             OPT_LEVEL ${ARG_OPT_LEVEL}
    )
    set(${ABC_FILES} "${RESULT}" PARENT_SCOPE)
endfunction()

function(prepare_spawn TARGET ABC_FILES WORK_DIR)
    # Build Spawn native module
    set(TARGET_LIB ${TARGET}-lib)
    set(ETSNAPI_LIB_DIR "${WORK_DIR}/lib")
    file(MAKE_DIRECTORY ${ETSNAPI_LIB_DIR})
    build_spawn_native_module(${TARGET_LIB} ${ETSNAPI_LIB_DIR})

    # Build Spawn managed code. Produced abc files will be loaded into boot context
    set(TARGET_ABC ${TARGET}-abc)
    build_spawn_managed_code(${TARGET_ABC} RESULT ${WORK_DIR})
    set(${ABC_FILES} "${RESULT}" PARENT_SCOPE)

    # Add env variables required for spawn native module
    set(PANDA_RUN_PREFIX
        LD_LIBRARY_PATH=${ETSNAPI_LIB_DIR}
        NATIVE_LIB_NAME=${TARGET_LIB}
        ${PANDA_RUN_PREFIX}
        PARENT_SCOPE)

    add_custom_target(${TARGET} DEPENDS ${TARGET_LIB} ${TARGET_ABC})
endfunction()

function(run_sts_app_mode_with_special_option TARGET WORK_DIR APP_MAIN_ABILITY_CLASS APP_ABC_FILES)
    set(oneValueArgs STDOUT_FILE SPAWN_ENTRYPOINT_FUNCTION)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    prepare_spawn(${TARGET}-spawn SPAWN_ABC_FILES ${WORK_DIR})
    list(GET SPAWN_ABC_FILES 0 SPAWN_ENTRY_ABC)
    list(JOIN SPAWN_ABC_FILES ":" SPAWN_ABC_FILES)

    list(JOIN APP_ABC_FILES ":" APP_ABC_FILES)
    set(PANDA_RUN_PREFIX
        APP_ABC_FILES=${APP_ABC_FILES}
        APP_MAIN_ABILITY_CLASS=${APP_MAIN_ABILITY_CLASS}
        ${PANDA_RUN_PREFIX})

    if (NOT DEFINED ARG_SPAWN_ENTRYPOINT_FUNCTION)
        set(ARG_SPAWN_ENTRYPOINT_FUNCTION "main")
    endif()

    set(BOOT_PANDA_FILES ${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc:${SPAWN_ABC_FILES})

    set(RUNTIME_ARGUMENTS
        --boot-panda-files=${BOOT_PANDA_FILES}
        ${ARG_RUNTIME_EXTRA_OPTIONS}
        ${SPAWN_ENTRY_ABC}
        "@spawn.spawn.ETSGLOBAL::${ARG_SPAWN_ENTRYPOINT_FUNCTION}"
    )

    if (DEFINED ARG_STDOUT_FILE)
        set(RUNTIME_ARGUMENTS ${RUNTIME_ARGUMENTS} 1> ${ARG_STDOUT_FILE})
    endif()

    add_custom_target(${TARGET}
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS}
        DEPENDS ${TARGET}-spawn
    )
endfunction()


function(run_sts_app_mode TARGET WORK_DIR APP_MAIN_ABILITY_CLASS APP_ABC_FILES)
    set(oneValueArgs STDOUT_FILE SPAWN_ENTRYPOINT_FUNCTION)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "" ${ARGN})
    set(RUNTIME_EXTRA_OPTIONS
        "--load-runtimes=ets"
        "--verification-mode=ahead-of-time"
        "--gc-type=g1-gc"
    )
    run_sts_app_mode_with_special_option(${TARGET} ${WORK_DIR} ${APP_MAIN_ABILITY_CLASS} "${ABC_FILES}"
                STDOUT_FILE ${ARG_STDOUT_FILE}
                SPAWN_ENTRYPOINT_FUNCTION ${ARG_SPAWN_ENTRYPOINT_FUNCTION}
                RUNTIME_EXTRA_OPTIONS ${RUNTIME_EXTRA_OPTIONS})
endfunction()

function(prepare_app TARGET ABC_FILES WORK_DIR SOURCE_DIR)
    # All application files
    set(RESULT "")

    # `app` package
    set(SOURCES_APP_DIR ${SOURCE_DIR}/app)
    set(SOURCES_APP
        ${SOURCES_APP_DIR}/module1.ets
        ${SOURCES_APP_DIR}/module2.ets
        ${SOURCES_APP_DIR}/module3.ets
    )
    set(TARGET_APP ${TARGET}-app-abc)
    compile_arktsconfig_unit(${TARGET_APP} ABCS_MODULES ${WORK_DIR} ${SOURCES_APP_DIR}/arktsconfig.json
                             "${SOURCES_APP}"
    )
    set(RESULT ${RESULT} ${ABCS_MODULES})

    # `foo` package
    set(TARGET_FOO ${TARGET}-foo-abc)
    set(SOURCES_FOO
        ${SOURCE_DIR}/foo/base.ets
        ${SOURCE_DIR}/foo/foo.ets
    )
    compile_arktsconfig_unit(${TARGET_FOO} ABCS_FOO ${WORK_DIR} ${SOURCE_DIR}/foo/arktsconfig.json
                             "${SOURCES_FOO}"
    )
    set(RESULT ${RESULT} ${ABCS_FOO})

    # `bar` package
    set(TARGET_BAR ${TARGET}-bar-abc)
    compile_arktsconfig_unit(${TARGET_BAR} ABCS_BAR ${WORK_DIR} ${SOURCE_DIR}/bar/arktsconfig.json
                             ${SOURCE_DIR}/bar/bar.ets
    )
    set(RESULT ${RESULT} ${ABCS_BAR})
    set(${ABC_FILES} "${RESULT}" PARENT_SCOPE)
    add_custom_target(${TARGET} DEPENDS ${TARGET_APP} ${TARGET_FOO} ${TARGET_BAR})
endfunction()

# prepare_incompatible_app compile the specific case and replace the original abc files with incompatiable
# the src dir contains original sts file
# the incompatible dir contains incompatiable sts file
# the app dir contains main entry to run the case
function(prepare_incompatible_app TARGET ABC_FILES WORK_DIR SOURCE_DIR MODULE_NAME)
    set(RESULT "")
    set(oneValueArgs ASM_SOURCE)
    set(multiValueArgs IGNORE_REPLACE_FILES)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    # src package
    set(TARGET_SRC ${TARGET}-${MODULE_NAME}-src-abc)
    file(GLOB SOURCES_SRC "${SOURCE_DIR}/${MODULE_NAME}/src/*.ets")
    compile_arktsconfig_unit(${TARGET_SRC} ABCS_SRC ${WORK_DIR} ${SOURCE_DIR}/${MODULE_NAME}/src/arktsconfig.json
                             "${SOURCES_SRC}"
    )
    # incompatible package
    set(TARGET_INCOMPATIABLE ${TARGET}-${MODULE_NAME}-incompatible-abc)
    if (ARG_ASM_SOURCE)
        file(GLOB SOURCES_INCOMPATIABLE "${SOURCE_DIR}/${MODULE_NAME}/incompatible/*.pa")
        compile_pandasm_source(${TARGET_INCOMPATIABLE} ABCS_INCOMPATIABLE ${WORK_DIR} "${SOURCES_INCOMPATIABLE}")
    else()
        file(GLOB SOURCES_INCOMPATIABLE "${SOURCE_DIR}/${MODULE_NAME}/incompatible/*.ets")
        compile_arktsconfig_unit(${TARGET_INCOMPATIABLE} ABCS_INCOMPATIABLE ${WORK_DIR} ${SOURCE_DIR}/${MODULE_NAME}/incompatible/arktsconfig.json
                                "${SOURCES_INCOMPATIABLE}"
        )
    endif()

    # replace the same name with incompatiable file, but do not replace files set by the IGNORE_REPLACE_FILES parameter
    foreach(SRC ${ABCS_SRC})
        get_filename_component(SRC_NAME ${SRC} NAME)
        string(REPLACE "${TARGET_SRC}-" "" SRC_NAME ${SRC_NAME})
        set(REPLACEMENT ${SRC})
        foreach(INCOMPATIABLE ${ABCS_INCOMPATIABLE})
            get_filename_component(INCOMPATIABLE_NAME ${INCOMPATIABLE} NAME)
            string(REPLACE "${TARGET_INCOMPATIABLE}-" "" INCOMPATIABLE_NAME ${INCOMPATIABLE_NAME})
            if(SRC_NAME STREQUAL INCOMPATIABLE_NAME)
                if (DEFINED ARG_IGNORE_REPLACE_FILES)
                    list_find_by_regex("${ARG_IGNORE_REPLACE_FILES}" "${SRC_NAME}" BASE_ABC_IDX)
                    if (BASE_ABC_IDX EQUAL -1)
                        set(REPLACEMENT ${INCOMPATIABLE})
                    endif()
                else()
                    set(REPLACEMENT ${INCOMPATIABLE})
                endif()
                break()
            endif()
        endforeach()
        set(RESULT ${RESULT} ${REPLACEMENT})
    endforeach()

    # `app` package
    set(TARGET_APP ${TARGET}-${MODULE_NAME}-app-abc)
    file(GLOB SOURCES_APP "${SOURCE_DIR}/${MODULE_NAME}/app/*.ets")
    compile_arktsconfig_unit(${TARGET_APP} ABCS_APP ${WORK_DIR} ${SOURCE_DIR}/${MODULE_NAME}/app/arktsconfig.json
                             "${SOURCES_APP}"
    )
    set(RESULT ${RESULT} ${ABCS_APP})
    set(${ABC_FILES} "${RESULT}" PARENT_SCOPE)


    add_custom_target(${TARGET} DEPENDS ${TARGET_SRC} ${TARGET_APP} ${TARGET_INCOMPATIABLE})
endfunction()


function(create_modules_test TARGET WORK_DIR SOURCE_DIR)
    prepare_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR})

    run_sts_app_mode(${TARGET} ${WORK_DIR} "@app.module1.MainAbility" "${ABC_FILES}")

    add_dependencies(${TARGET} ${TARGET}-app)
endfunction()

create_modules_test(ets_modules_app_mode_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

function(list_find_by_regex ELEMENTS REGEX FOUND_IDX)
    set(RESULT -1)
    set(IDX 0)
    foreach(ITER ${ELEMENTS})
        if(ITER MATCHES ${REGEX})
            set(RESULT ${IDX})
            break()
        endif()
        math(EXPR IDX "${IDX}+1")
    endforeach()
    set(${FOUND_IDX} ${RESULT} PARENT_SCOPE)
endfunction()

function(create_no_class_def_found_test TARGET WORK_DIR SOURCE_DIR)
    prepare_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR})
    list_find_by_regex("${ABC_FILES}" "(.*)-base.abc" BASE_ABC_IDX)
    # As a result, `@foo.foo.Dummy` class from `foo.ets` must be not loaded
    list(REMOVE_AT ABC_FILES ${BASE_ABC_IDX})

    set(STDOUT_FILE ${WORK_DIR}/${TARGET}-output.txt)
    run_sts_app_mode(${TARGET}-run-sts-app-mode ${WORK_DIR} "@app.module1.MainAbility" "${ABC_FILES}"
                     STDOUT_FILE ${STDOUT_FILE}
                     SPAWN_ENTRYPOINT_FUNCTION "mainCatchLinkerUnresolvedClassError"
    )
    add_dependencies(${TARGET}-run-sts-app-mode ${TARGET}-app)

    add_custom_target(${TARGET}
        COMMENT "Check class not loaded due to LinkerUnresolvedClassError"
        COMMAND ${CMAKE_COMMAND} -E compare_files ${STDOUT_FILE} ${SOURCE_DIR}/incompatible_foo/not_verified_method.txt
        DEPENDS ${TARGET}-run-sts-app-mode
    )
endfunction()

create_no_class_def_found_test(ets_modules_no_class_def_found_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

function(create_incompatible_class_change_test TARGET WORK_DIR SOURCE_DIR)
    prepare_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR})
    list_find_by_regex("${ABC_FILES}" "(.*)-base.abc" BASE_ABC_IDX)
    list(REMOVE_AT ABC_FILES ${BASE_ABC_IDX})

    # Take `Base` class from `incompatible-foo` package's file
    set(TARGET_INCOMPATIBLE_FOO ${TARGET}-incompatible-foo-abc)
    compile_arktsconfig_unit(${TARGET_INCOMPATIBLE_FOO} ABCS_FOO ${WORK_DIR} ${SOURCE_DIR}/incompatible_foo/arktsconfig.json
                             ${SOURCE_DIR}/incompatible_foo/base.ets
    )
    set(ABC_FILES ${ABC_FILES} ${ABCS_FOO})

    set(STDOUT_FILE ${WORK_DIR}/${TARGET}-output.txt)
    run_sts_app_mode(${TARGET}-run-sts-app-mode ${WORK_DIR} "@app.module1.MainAbility" "${ABC_FILES}"
                     STDOUT_FILE ${STDOUT_FILE}
                     SPAWN_ENTRYPOINT_FUNCTION "mainCatchLinkerBadSupertypeError")
    add_dependencies(${TARGET}-run-sts-app-mode ${TARGET}-app ${TARGET_INCOMPATIBLE_FOO})

    add_custom_target(${TARGET}
        COMMENT "Check class not loaded due to LinkerBadSupertypeError"
        COMMAND ${CMAKE_COMMAND} -E compare_files ${STDOUT_FILE} ${SOURCE_DIR}/incompatible_foo/not_loaded_class.txt
        DEPENDS ${TARGET}-run-sts-app-mode
    )
endfunction()

create_incompatible_class_change_test(ets_modules_incompatible_class_change_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

function(create_mismatched_test TARGET WORK_DIR SOURCE_DIR)
    prepare_incompatible_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR} "mismatched" )

    # test extends final class
    run_sts_app_mode(${TARGET}-extends-final-run-sts-app-mode ${WORK_DIR} "@app.module4.MainAbility" "${ABC_FILES}"
                     SPAWN_ENTRYPOINT_FUNCTION "mainMisatchedExtendsFinal")
    add_dependencies(${TARGET}-extends-final-run-sts-app-mode ${TARGET}-app)
    add_custom_target(${TARGET}-extends
        COMMENT "Check class extends final class mainMisatchedExtendsFinal"
        DEPENDS ${TARGET}-extends-final-run-sts-app-mode
    )

    # test override final method
    run_sts_app_mode(${TARGET}-override-final-run-sts-app-mode ${WORK_DIR} "@app.module5.MainAbility" "${ABC_FILES}"
                     SPAWN_ENTRYPOINT_FUNCTION "mainMisatchedOverrideFinal")
    add_dependencies(${TARGET}-override-final-run-sts-app-mode ${TARGET}-app)
    add_custom_target(${TARGET}-override
        COMMENT "Check class override final method mainMisatchedOverrideFinal"
        DEPENDS ${TARGET}-override-final-run-sts-app-mode
    )

    add_custom_target(${TARGET}
            DEPENDS ${TARGET}-extends ${TARGET}-override
    )

endfunction()

create_mismatched_test(ets_modules_mismatched_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

function(create_loop_inheritance_class_test TARGET WORK_DIR SOURCE_DIR)
    prepare_incompatible_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR} "loop_inheritance"
                             IGNORE_REPLACE_FILES "level_two.abc")

    # test extends final class
    run_sts_app_mode(${TARGET}-run-sts-app-mode ${WORK_DIR} "@app.module6.MainAbility" "${ABC_FILES}"
                     SPAWN_ENTRYPOINT_FUNCTION "mainCircularityExtends")
    add_dependencies(${TARGET}-run-sts-app-mode ${TARGET}-app)

    add_custom_target(${TARGET}
        COMMENT "Check class loop inheritance class mainCircularityExtends"
        DEPENDS ${TARGET}-run-sts-app-mode
    )

endfunction()

create_loop_inheritance_class_test(ets_modules_loop_inheritance_class_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

function(create_multiple_override_test TARGET WORK_DIR SOURCE_DIR)
    prepare_incompatible_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR} "multiple_override")

    # test override_base.ets multiple override method
    run_sts_app_mode(${TARGET}-multiple_override-run-sts-app-mode ${WORK_DIR} "@app.module.MainAbility" "${ABC_FILES}"
                     SPAWN_ENTRYPOINT_FUNCTION "mainMultipleOverrideBase")
    add_dependencies(${TARGET}-multiple_override-run-sts-app-mode ${TARGET}-app)
    add_custom_target(${TARGET}-multiple_override
        COMMENT "Check class multiple override method mainMultipleOverrideBase"
        DEPENDS ${TARGET}-multiple_override-run-sts-app-mode
    )

    # test override_base1.ets multiple override method
    run_sts_app_mode(${TARGET}-multiple_override1-run-sts-app-mode ${WORK_DIR} "@app.module1.MainAbility" "${ABC_FILES}"
                     SPAWN_ENTRYPOINT_FUNCTION "mainMultipleOverrideBase1")
    add_dependencies(${TARGET}-multiple_override1-run-sts-app-mode ${TARGET}-app)
    add_custom_target(${TARGET}-multiple_override1
        COMMENT "Check class multiple override method mainMultipleOverrideBase1"
        DEPENDS ${TARGET}-multiple_override1-run-sts-app-mode
    )

    add_custom_target(${TARGET}
            DEPENDS ${TARGET}-multiple_override ${TARGET}-multiple_override1
    )

endfunction()

create_multiple_override_test(ets_modules_multiple_override_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

function(create_multiple_implement_test TARGET WORK_DIR SOURCE_DIR)
    prepare_incompatible_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR} "multiple_implement")
    # test a2_extends_a1_i1.ets multiple implement method
    run_sts_app_mode(${TARGET}-multiple_implement-run-sts-app-mode ${WORK_DIR} "@app.module.MainAbility" "${ABC_FILES}"
                     SPAWN_ENTRYPOINT_FUNCTION "mainMultipleImplement")
    add_dependencies(${TARGET}-multiple_implement-run-sts-app-mode ${TARGET}-app)
    add_custom_target(${TARGET}-multiple_implement
        COMMENT "Check class multiple implement method mainMultipleImplement"
        DEPENDS ${TARGET}-multiple_implement-run-sts-app-mode
    )

    # test a2_extends_a1_i2.ets multiple implement method
    run_sts_app_mode(${TARGET}-multiple_implement1-run-sts-app-mode ${WORK_DIR} "@app.module1.MainAbility" "${ABC_FILES}"
                     SPAWN_ENTRYPOINT_FUNCTION "mainMultipleImplement1")
    add_dependencies(${TARGET}-multiple_implement1-run-sts-app-mode ${TARGET}-app)
    add_custom_target(${TARGET}-multiple_implement1
        COMMENT "Check class multiple implement method mainMultipleImplement1"
        DEPENDS ${TARGET}-multiple_implement1-run-sts-app-mode
    )

    add_custom_target(${TARGET}
            DEPENDS ${TARGET}-multiple_implement ${TARGET}-multiple_implement1
    )

endfunction()

create_multiple_implement_test(ets_modules_multiple_implement_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

function(create_verification_mode_test TARGET WORK_DIR SOURCE_DIR TEST_NAME ASM_SOURCE)
    prepare_incompatible_app(${TARGET}-app ABC_FILES ${WORK_DIR} ${SOURCE_DIR} ${TEST_NAME} ASM_SOURCE ${ASM_SOURCE})
    # test --verification-mode on-the-fly mode
    set(RUNTIME_EXTRA_OPTIONS
        "--load-runtimes=ets"
        "--verification-mode=on-the-fly"
        "--gc-type=g1-gc"
    )
    run_sts_app_mode_with_special_option(${TARGET}-on-the-fly-run-sts-app-mode ${WORK_DIR} "@app.module.MainAbility" "${ABC_FILES}"
                     SPAWN_ENTRYPOINT_FUNCTION "mainVerificationModeOnTheFly" RUNTIME_EXTRA_OPTIONS ${RUNTIME_EXTRA_OPTIONS})
    add_dependencies(${TARGET}-on-the-fly-run-sts-app-mode ${TARGET}-app)
    add_custom_target(${TARGET}-on-the-fly
        COMMENT "Check --verification-mode on-the-fly mode mainVerificationModeOnTheFly"
        DEPENDS ${TARGET}-on-the-fly-run-sts-app-mode
    )

    # test --verification-mode ahead-of-time mode
    set(RUNTIME_EXTRA_OPTIONS
        "--load-runtimes=ets"
        "--verification-mode=ahead-of-time"
        "--gc-type=g1-gc"
    )
    run_sts_app_mode_with_special_option(${TARGET}-ahead-of-time-run-sts-app-mode ${WORK_DIR} "@app.module.MainAbility" "${ABC_FILES}"
                     SPAWN_ENTRYPOINT_FUNCTION "mainVerificationModeAheadOfTime" RUNTIME_EXTRA_OPTIONS ${RUNTIME_EXTRA_OPTIONS})
    add_dependencies(${TARGET}-ahead-of-time-run-sts-app-mode ${TARGET}-app)
    add_custom_target(${TARGET}-ahead-of-time
        COMMENT "Check --verification-mode ahead-of-time mode mainVerificationModeAheadOfTime"
        DEPENDS ${TARGET}-ahead-of-time-run-sts-app-mode
    )
    add_custom_target(${TARGET}
            DEPENDS ${TARGET}-on-the-fly ${TARGET}-ahead-of-time
    )
endfunction()

create_verification_mode_test(ets_modules_verification_mode_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} "verification_mode" FALSE)
create_verification_mode_test(ets_modules_verification_i32f64misuse_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} "verification_i32f64misuse" TRUE)
create_verification_mode_test(ets_modules_verification_wrong_this_test ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} "verification_wrong_this" TRUE)

add_custom_target(ets_modules_tests
                  DEPENDS
                    ets_modules_app_mode_test
                    ets_modules_no_class_def_found_test
                    ets_modules_incompatible_class_change_test
                    ets_modules_mismatched_test
                    ets_modules_loop_inheritance_class_test
                    ets_modules_multiple_override_test
                    ets_modules_multiple_implement_test
                    ets_modules_verification_mode_test)
add_dependencies(ets_tests ets_modules_tests)
