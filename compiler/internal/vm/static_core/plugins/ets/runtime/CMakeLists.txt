# Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

set(ETS_EXT_SOURCES ${PANDA_ETS_PLUGIN_SOURCE}/runtime)

set(ETS_RUNTIME_SOURCES
    ${PANDA_ETS_PLUGIN_SOURCE}/dfx/heap_helpers.cpp
    ${ETS_EXT_SOURCES}/ani/ani_interaction_api.cpp
    ${ETS_EXT_SOURCES}/ani/ani_mangle.cpp
    ${ETS_EXT_SOURCES}/ani/ani_options.cpp
    ${ETS_EXT_SOURCES}/ani/ani_options_parser.cpp
    ${ETS_EXT_SOURCES}/ani/ani_vm_api.cpp
    ${ETS_EXT_SOURCES}/ani/verify/ani_verifier.cpp
    ${ETS_EXT_SOURCES}/ani/verify/env_ani_verifier.cpp
    ${ETS_EXT_SOURCES}/ani/verify/types/internal_ref.cpp
    ${ETS_EXT_SOURCES}/ani/verify/types/venv.cpp
    ${ETS_EXT_SOURCES}/ani/verify/types/vmethod.cpp
    ${ETS_EXT_SOURCES}/ani/verify/types/vfield.cpp
    ${ETS_EXT_SOURCES}/ani/verify/types/vref.cpp
    ${ETS_EXT_SOURCES}/ani/verify/verify_ani_checker.cpp
    ${ETS_EXT_SOURCES}/ani/verify/verify_ani_interaction_api.cpp
    ${ETS_EXT_SOURCES}/ani/verify/verify_ani_vm_api.cpp
    ${ETS_EXT_SOURCES}/ets_annotation.cpp
    ${ETS_EXT_SOURCES}/ets_class_linker.cpp
    ${ETS_EXT_SOURCES}/ets_class_linker_context.cpp
    ${ETS_EXT_SOURCES}/ets_class_linker_extension.cpp
    ${ETS_EXT_SOURCES}/ets_exceptions.cpp
    ${ETS_EXT_SOURCES}/ets_itable_builder.cpp
    ${ETS_EXT_SOURCES}/ets_language_context.cpp
    ${ETS_EXT_SOURCES}/ets_napi_env.cpp
    ${ETS_EXT_SOURCES}/ets_native_library.cpp
    ${ETS_EXT_SOURCES}/ets_native_library_provider.cpp
    ${ETS_EXT_SOURCES}/ets_namespace_manager.cpp
    ${ETS_EXT_SOURCES}/ets_namespace_manager_impl.cpp
    ${ETS_EXT_SOURCES}/ets_platform_types.cpp
    ${ETS_EXT_SOURCES}/ets_runtime_interface.cpp
    ${ETS_EXT_SOURCES}/ets_stdlib_cache.cpp
    ${ETS_EXT_SOURCES}/ets_stubs.cpp
    ${ETS_EXT_SOURCES}/ets_vm.cpp
    ${ETS_EXT_SOURCES}/ets_vtable_builder.cpp
    ${ETS_EXT_SOURCES}/integrate/ark_vm_api.cpp
    ${ETS_EXT_SOURCES}/integrate/ets_ani_expo.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core.cpp
    ${ETS_EXT_SOURCES}/intrinsics/escompat_Array.cpp
    ${ETS_EXT_SOURCES}/intrinsics/escompat_ArrayBuffer.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_RegExp.cpp
    ${ETS_EXT_SOURCES}/intrinsics/escompat_TypedArrays.cpp
    ${ETS_EXT_SOURCES}/intrinsics/escompat_Date.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_JSON.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_concurrency_taskpool.cpp
    ${ETS_EXT_SOURCES}/intrinsics/escompat_Reflect.cpp
    ${ETS_EXT_SOURCES}/intrinsics/escompat_Map.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_containers_WeakMap.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Set.cpp
    ${ETS_EXT_SOURCES}/intrinsics/compiler_intrinsics.cpp
    ${ETS_EXT_SOURCES}/intrinsics/debugger_api.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_AniHelpers.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Runtime.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Class.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_AbcFile.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_AbcRuntimeLinker.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Char.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Console.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Byte.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Short.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Int.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Long.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Double.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Float.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_SyncPrimitives.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_ExclusiveLauncher.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_String.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_StringBuilder.cpp
    ${ETS_EXT_SOURCES}/intrinsics/gc_task_tracker.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_gc.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_finalization_registry.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Promise.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_StackTrace.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Type.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_TypeCreator.cpp
    ${ETS_EXT_SOURCES}/intrinsics/reflect_Proxy.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Method.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_reflect_Method.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_reflect_Field.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_reflect_internals.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Value.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Arrays.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Job.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_core_Timers.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_concurrency_Launch.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_concurrency_Atomics.cpp
    ${ETS_EXT_SOURCES}/intrinsics/std_math.cpp
    ${ETS_EXT_SOURCES}/intrinsics/unsafe_memory.cpp
    ${ETS_EXT_SOURCES}/intrinsics/helpers/dtoa_helper.cpp
    ${ETS_EXT_SOURCES}/intrinsics/helpers/array_buffer_helper.cpp
    ${ETS_EXT_SOURCES}/intrinsics/helpers/ets_intrinsics_helpers.cpp
    ${ETS_EXT_SOURCES}/intrinsics/helpers/reflection_helpers.cpp
    ${ETS_EXT_SOURCES}/intrinsics/helpers/ets_string_case_conversion.cpp
    ${ETS_EXT_SOURCES}/intrinsics/helpers/ets_string_helpers.cpp
    ${ETS_EXT_SOURCES}/intrinsics/helpers/ets_to_string_cache.cpp
    ${ETS_EXT_SOURCES}/intrinsics/helpers/json_helper.cpp
    ${ETS_EXT_SOURCES}/finalreg/finalization_registry_manager.cpp
    ${ETS_EXT_SOURCES}/unhandled_manager/unhandled_object_manager.cpp
    ${ETS_EXT_SOURCES}/main_worker_external_scheduler.cpp
    ${ETS_EXT_SOURCES}/mem/ets_gc_stat.cpp
    ${ETS_EXT_SOURCES}/mem/ets_reference_processor.cpp
    ${ETS_EXT_SOURCES}/ani/ani_helpers.cpp
    ${ETS_EXT_SOURCES}/ets_coroutine.cpp
    ${ETS_EXT_SOURCES}/ets_entrypoints.cpp
    ${ETS_EXT_SOURCES}/types/ets_class.cpp
    ${ETS_EXT_SOURCES}/types/ets_arraybuffer.cpp
    ${ETS_EXT_SOURCES}/types/ets_escompat_array.cpp
    ${ETS_EXT_SOURCES}/types/ets_field.cpp
    ${ETS_EXT_SOURCES}/types/ets_method.cpp
    ${ETS_EXT_SOURCES}/types/ets_promise.cpp
    ${ETS_EXT_SOURCES}/types/ets_promise_ref.cpp
    ${ETS_EXT_SOURCES}/types/ets_object.cpp
    ${ETS_EXT_SOURCES}/types/ets_sync_primitives.cpp
    ${ETS_EXT_SOURCES}/types/ets_string_builder.cpp
    ${ETS_EXT_SOURCES}/types/ets_string.cpp
    ${ETS_EXT_SOURCES}/types/ets_typeapi_create.cpp
    ${ETS_EXT_SOURCES}/types/ets_typeapi_field.cpp
    ${ETS_EXT_SOURCES}/types/ets_reflect_field.cpp
    ${ETS_EXT_SOURCES}/types/ets_typeapi_method.cpp
    ${ETS_EXT_SOURCES}/types/ets_reflect_method.cpp
    ${ETS_EXT_SOURCES}/types/ets_typeapi_parameter.cpp
    ${ETS_EXT_SOURCES}/types/ets_waiters_list.cpp
    ${ETS_EXT_SOURCES}/types/ets_finalizable_weak_ref.cpp
    ${ETS_EXT_SOURCES}/types/ets_job.cpp
    ${ETS_EXT_SOURCES}/types/ets_map.cpp
    ${ETS_EXT_SOURCES}/types/ets_finreg_node.cpp
    ${ETS_EXT_SOURCES}/types/ets_finalization_registry.cpp
    ${ETS_EXT_SOURCES}/ets_vm_api.cpp
    ${ETS_EXT_SOURCES}/ets_utils.cpp
    ${ETS_EXT_SOURCES}/ets_object_state_info.cpp
    ${ETS_EXT_SOURCES}/ets_object_state_table.cpp
    ${ETS_EXT_SOURCES}/static_object_accessor.cpp
    ${ETS_EXT_SOURCES}/static_type_converter.cpp
    ${ETS_EXT_SOURCES}/entrypoints/ets_proxy_entrypoints.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/init_native_methods.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/regexp/RegExp.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/regexp/regexp_executor.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/regexp/regexp_16.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/regexp/regexp_8.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/etsstdlib.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/Intl.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlCommon.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlState.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlNumberFormat.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlNumberFormatters.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlFormattersCache.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlLocaleMatch.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlCollator.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlCollatorCache.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlLocale.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlLocaleHelper.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlPluralRules.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlSegmenter.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/stdlib_ani_helpers.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlDateTimeFormat.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlDateTimeFormatCache.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlListFormat.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlDisplayNames.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlRelativeTimeFormat.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlRelativeTimeFormatCache.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/IntlLanguageTag.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/stdlib/native/core/Process.cpp
)

get_directory_property(CURRENT_DEFS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_DEFINITIONS)

if (PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)
    list (APPEND ETS_RUNTIME_SOURCES
        ${ETS_EXT_SOURCES}/ani/arch/arm32/ets_async_entry_point_arm32.S
        ${ETS_EXT_SOURCES}/ani/arch/arm32/ets_napi_entry_point_arm32.S
        ${ETS_EXT_SOURCES}/entrypoints/arch/arm32/ets_proxy_entry_point_arm32.S
    )
elseif(PANDA_TARGET_ARM32_ABI_HARD)
    list (APPEND ETS_RUNTIME_SOURCES
        ${ETS_EXT_SOURCES}/ani/arch/arm32/ets_async_entry_point_arm32hf.S
        ${ETS_EXT_SOURCES}/ani/arch/arm32/ets_napi_entry_point_arm32hf.S
        ${ETS_EXT_SOURCES}/entrypoints/arch/arm32/ets_proxy_entry_point_arm32hf.S
    )
elseif(PANDA_TARGET_ARM64)
    list (APPEND ETS_RUNTIME_SOURCES
        ${ETS_EXT_SOURCES}/ani/arch/arm64/ets_async_entry_point_aarch64.S
        ${ETS_EXT_SOURCES}/ani/arch/arm64/ets_napi_entry_point_aarch64.S
        ${ETS_EXT_SOURCES}/entrypoints/arch/arm64/ets_proxy_entry_point_aarch64.S
    )
elseif (PANDA_TARGET_AMD64)
    list (APPEND ETS_RUNTIME_SOURCES
        ${ETS_EXT_SOURCES}/ani/arch/amd64/ets_async_entry_point_amd64.S
        ${ETS_EXT_SOURCES}/ani/arch/amd64/ets_napi_entry_point_amd64.S
        ${ETS_EXT_SOURCES}/entrypoints/arch/amd64/ets_proxy_entry_point_amd64.S
    )
endif()

if (PANDA_TARGET_OHOS)
    list (APPEND ETS_RUNTIME_SOURCES
        ${ETS_EXT_SOURCES}/platform/ohos/logger.cpp)
endif()

panda_target_sources(arkruntime_obj PRIVATE ${ETS_RUNTIME_SOURCES})
panda_target_include_directories(arkruntime_obj PUBLIC
    ${PANDA_BINARY_ROOT}/cross_values
    ${PANDA_ETS_PLUGIN_SOURCE}/runtime/
    ${PANDA_BINARY_ROOT}/
    # The line below needed only for stdlib. It must be removed after stdlib separation from runtime is implemented(#18135).
    ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani/
    ${PANDA_ROOT}/../common_runtime
)
add_dependencies(arkruntime_obj es2panda-public)
panda_target_link_libraries(arkruntime_obj es2panda-public arkassembler)

panda_target_compile_definitions(arkruntime_obj PRIVATE ${CURRENT_DEFS})
add_dependencies(arkruntime_obj cross_values)

if(PANDA_TARGET_OHOS)
    panda_target_link_libraries(arkruntime_obj hilog_ndk.z)
endif()

if (PANDA_ETS_INTEROP_JS)
    add_subdirectory(${PANDA_ROOT}/../platform ${PANDA_BINARY_ROOT}/platform)
    if (PANDA_JS_ETS_HYBRID_MODE)
        panda_target_compile_definitions(arkruntime_obj PRIVATE PANDA_JS_ETS_HYBRID_MODE)
    endif(PANDA_JS_ETS_HYBRID_MODE)
    add_subdirectory(interop_js)
endif()

add_subdirectory(libani_helpers)

set(IRTOC_ETS_SCRIPTS ${PANDA_ROOT}/plugins/ets/irtoc_scripts)
set(IRTOC_ETS_SCRIPTS ${IRTOC_ETS_SCRIPTS} PARENT_SCOPE)
if(NOT (CMAKE_CROSSCOMPILING OR PANDA_TARGET_OHOS))
    irtoc_compile(TARGET_NAME irtoc_ets_fastpath
                  # Workaround for #30178: file order of arrays.irt and escompat_array.irt are changed
                  INPUT_FILES ${IRTOC_ETS_SCRIPTS}/escompat_array.irt
                              ${IRTOC_ETS_SCRIPTS}/to_string_cache.irt
                              ${IRTOC_ETS_SCRIPTS}/typed_arrays.irt
                              ${IRTOC_ETS_SCRIPTS}/arrays.irt
                              ${IRTOC_ETS_SCRIPTS}/runtime.irt
                  TARGET_VARIABLE IRTOC_ETS_FASTPATH_OBJ
                  TARGET_VARIABLE_LLVM IRTOC_ETS_FASTPATH_LLVM_OBJ)

    if (NOT EXISTS ${IRTOC_ETS_FASTPATH_OBJ})
        file(TOUCH ${IRTOC_ETS_FASTPATH_OBJ})
    endif()
    if (PANDA_LLVM_FASTPATH AND NOT EXISTS ${IRTOC_ETS_FASTPATH_LLVM_OBJ})
        file(TOUCH ${IRTOC_ETS_FASTPATH_LLVM_OBJ})
    endif()
    set(irtoc_ets_fastpath_target irtoc_ets_fastpath)
else()
    ExternalProject_Get_Property(panda_host_tools binary_dir)
    set(IRTOC_ETS_FASTPATH_OBJ "${binary_dir}/irtoc/irtoc_ets_fastpath/irtoc_ets_fastpath.o")
    set(IRTOC_ETS_FASTPATH_LLVM_OBJ "${binary_dir}/irtoc/irtoc_ets_fastpath/irtoc_ets_fastpath_llvm.o")

    SET_SOURCE_FILES_PROPERTIES(${IRTOC_ETS_FASTPATH_OBJ} PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )
    SET_SOURCE_FILES_PROPERTIES(${IRTOC_ETS_FASTPATH_LLVM_OBJ} PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
    )

    # Due to cmake complain about absence of the irtoc file, we just fake it until it will be generated
    file(MAKE_DIRECTORY ${binary_dir}/irtoc/irtoc_ets_fastpath)
    if (NOT EXISTS ${IRTOC_ETS_FASTPATH_OBJ})
        file(TOUCH ${IRTOC_ETS_FASTPATH_OBJ})
    endif()
    if (NOT EXISTS ${IRTOC_ETS_FASTPATH_LLVM_OBJ})
        file(TOUCH ${IRTOC_ETS_FASTPATH_LLVM_OBJ})
    endif()
    set(irtoc_ets_fastpath_target panda_host_tools-build)
endif()

add_dependencies(arkruntime_obj
    ${irtoc_ets_fastpath_target}
)

set(IRTOC_ETS_LLVM_OBJECTS "")
if (PANDA_LLVM_FASTPATH)
    list(APPEND IRTOC_ETS_LLVM_OBJECTS ${IRTOC_ETS_FASTPATH_LLVM_OBJ})
endif()

panda_target_sources(arkruntime_interpreter_static PRIVATE ${IRTOC_ETS_FASTPATH_OBJ} ${IRTOC_ETS_LLVM_OBJECTS})
panda_target_sources(arkruntime_static PRIVATE ${IRTOC_ETS_FASTPATH_OBJ} ${IRTOC_ETS_LLVM_OBJECTS})

panda_target_link_libraries(arkruntime_obj ani_interface)
