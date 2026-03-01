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

enable_language(ASM)

set(ETS_INTEROP_ARCH_SOURCES)

if(PANDA_TARGET_ARM32_ABI_SOFT OR PANDA_TARGET_ARM32_ABI_SOFTFP)
    list(APPEND ETS_INTEROP_ARCH_SOURCES ${ETS_EXT_SOURCES}/interop_js/arch/arm32/call_bridge_arm32.S)
elseif(PANDA_TARGET_ARM32_ABI_HARD)
    list(APPEND ETS_INTEROP_ARCH_SOURCES ${ETS_EXT_SOURCES}/interop_js/arch/arm32/call_bridge_arm32hf.S)
elseif(PANDA_TARGET_ARM64)
    list(APPEND ETS_INTEROP_ARCH_SOURCES ${ETS_EXT_SOURCES}/interop_js/arch/arm64/call_bridge_aarch64.S)
elseif(PANDA_TARGET_AMD64)
    list(APPEND ETS_INTEROP_ARCH_SOURCES ${ETS_EXT_SOURCES}/interop_js/arch/amd64/call_bridge_amd64.S)
endif()

set(ETS_INTEROP_JS_NAPI_SOURCES
    ets_vm_plugin.cpp
)
set(ETS_INTEROP_JS_NAPI_LIBRARIES
    arkruntime
)
set(ETS_INTEROP_HYBRID_SOURCES)

add_runtime_options(${CMAKE_CURRENT_SOURCE_DIR}/runtime_options.yaml)

panda_ets_interop_js_plugin(ets_interop_js_napi
    SOURCES ${ETS_INTEROP_JS_NAPI_SOURCES}
    LIBRARIES ${ETS_INTEROP_JS_NAPI_LIBRARIES}
    OUTPUT_SUFFIX ".so"
)

if (PANDA_JS_ETS_HYBRID_MODE)
    list(APPEND ETS_INTEROP_HYBRID_SOURCES
        ${ETS_EXT_SOURCES}/interop_js/handshake.cpp
    )
    panda_target_compile_definitions(ets_interop_js_napi PRIVATE PANDA_JS_ETS_HYBRID_MODE)
endif(PANDA_JS_ETS_HYBRID_MODE)

panda_target_sources(arkruntime_obj PRIVATE
        ${ETS_EXT_SOURCES}/interop_js/intrinsics/std_js_jsruntime.cpp
        ${ETS_EXT_SOURCES}/interop_js/interop_context.cpp
        ${ETS_EXT_SOURCES}/interop_js/intrinsics_api_impl.cpp
        ${ETS_EXT_SOURCES}/interop_js/call/call_js.cpp
        ${ETS_EXT_SOURCES}/interop_js/call/call_ets.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_proxy/js_proxy.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/ets_class_wrapper.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/ets_field_wrapper.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/ets_method_wrapper.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/ets_method_set.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/shared_reference.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/shared_reference_storage.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/shared_reference_storage_verifier.cpp
        ${ETS_EXT_SOURCES}/interop_js/ets_proxy/ets_proxy.cpp
        ${ETS_EXT_SOURCES}/interop_js/event_loop_module.cpp
        ${ETS_EXT_SOURCES}/interop_js/st_value/ets_vm_STValue.cpp
        ${ETS_EXT_SOURCES}/interop_js/st_value/ets_vm_STValue_check.cpp
        ${ETS_EXT_SOURCES}/interop_js/st_value/ets_vm_STValue_instance.cpp
        ${ETS_EXT_SOURCES}/interop_js/st_value/ets_vm_STValue_invoke.cpp
        ${ETS_EXT_SOURCES}/interop_js/st_value/ets_vm_STValue_accessor.cpp
        ${ETS_EXT_SOURCES}/interop_js/st_value/ets_vm_STValue_unwrap.cpp
        ${ETS_EXT_SOURCES}/interop_js/st_value/ets_vm_STValue_wrap.cpp
        ${ETS_EXT_SOURCES}/interop_js/st_value/ets_vm_STValue_param_getter.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_value.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_refconvert.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_refconvert_builtin.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_refconvert_function.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_refconvert_record.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_refconvert_tuple.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_refconvert_union.cpp
        ${ETS_EXT_SOURCES}/interop_js/napi_impl/napi_impl.cpp
        ${ETS_EXT_SOURCES}/interop_js/native_api/arkts_interop_js_api_impl.cpp
        ${ETS_EXT_SOURCES}/interop_js/interop_common.cpp
        ${ETS_EXT_SOURCES}/interop_js/timer_module.cpp
        ${ETS_EXT_SOURCES}/interop_js/js_job_queue.cpp
        ${ETS_EXT_SOURCES}/interop_js/stack_info.cpp
        ${ETS_EXT_SOURCES}/interop_js/sts_vm_interface_impl.cpp
        ${ETS_EXT_SOURCES}/interop_js/timer_helper/timer.cpp
        ${ETS_EXT_SOURCES}/interop_js/timer_helper/interop_timer_helper.cpp
        ${ETS_EXT_SOURCES}/interop_js/xgc/xgc.cpp
        ${ETS_EXT_SOURCES}/interop_js/xgc/xgc_vm_adaptor.cpp
        ${ETS_EXT_SOURCES}/interop_js/xref_object_operator.cpp
        ${ETS_EXT_SOURCES}/interop_js/code_scopes.cpp
        ${ETS_INTEROP_ARCH_SOURCES}
        ${ETS_INTEROP_HYBRID_SOURCES}
)

if(PANDA_TARGET_OHOS)
    panda_target_link_libraries(arkruntime_obj ace_napi.z uv -Wl,--no-undefined)
else()
    panda_target_include_directories(arkruntime_obj SYSTEM INTERFACE ${ARKUI_NAPI_HEADERS_PATH})
    # In case of running using ArkJSVM but using ark_js_napi_cli tool it is enough to include directories,
    # because node and ark_js_napi_cli binaries are already linked with sources of uv and napi libraries
    panda_target_include_directories(arkruntime_obj SYSTEM INTERFACE ${NAPI_HEADERS_PATH})

    if (PANDA_JS_ETS_HYBRID_MODE)
        # NOTE(konstanting, #23205): A workaround for the hybrid cmake build. Will be removed soon
        # using a separate .cpp file with weak symbols.
        panda_target_compile_definitions(arkruntime_obj PRIVATE PANDA_JS_ETS_HYBRID_MODE_NEED_WEAK_SYMBOLS)
    endif()
    if (ARK_HYBRID)
        panda_target_compile_definitions(arkruntime_obj PRIVATE ARK_HYBRID)
    endif()
    if (USE_CMC_GC)
        panda_target_compile_definitions(arkruntime_obj PRIVATE USE_CMC_GC)
    endif()
    if (USE_READ_BARRIER)
        panda_target_compile_definitions(arkruntime_obj PRIVATE USE_READ_BARRIER)
    endif()
endif()
