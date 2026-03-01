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


# Create ets_interop_js_gtest plugin
panda_add_library(ets_interop_js_gtest STATIC ets_interop_js_gtest.cpp)
panda_target_link_libraries(ets_interop_js_gtest gtest)
panda_target_link_libraries(ets_interop_js_gtest arkruntime)
panda_target_link_libraries(ets_interop_js_gtest ani_helpers)
set_target_properties(ets_interop_js_gtest PROPERTIES POSITION_INDEPENDENT_CODE ON)
panda_target_include_directories(ets_interop_js_gtest
    PUBLIC ${PANDA_ETS_PLUGIN_SOURCE}/tests/interop_js/gtest_plugin
    PUBLIC ${PANDA_ETS_PLUGIN_SOURCE}/tests/interop_js/test_helper
    PUBLIC ${PANDA_ROOT}
)

if(PANDA_TARGET_OHOS)
    panda_target_link_libraries(ets_interop_js_gtest ace_napi.z)
else()
    panda_target_include_directories(ets_interop_js_gtest SYSTEM PRIVATE ${NAPI_HEADERS_PATH})
endif()

compile_dynamic_file(ets_interop_js_gtest_launcher
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/gtest_launcher.js"
    OUTPUT_DIR "${INTEROP_TESTS_DIR}"
    COMPILE_OPTION --commonjs
)

# Make symbolic links to convinient work with requireNapiPreview
set(SO_FILES_OUTPUT "${INTEROP_TESTS_DIR}/module/lib/")
set(INTEROP_LIB_SOURCE "${PANDA_BINARY_ROOT}/lib/module/ets_interop_js_napi.so")
set(INTEROP_HELPER_LIB_SOURCE "${PANDA_BINARY_ROOT}/lib/interop_js/libinterop_test_helper.so")

add_custom_target(create_slinks_for_interop_gtest_launcher
    COMMAND mkdir -p ${SO_FILES_OUTPUT}
            && ln -sf ${INTEROP_LIB_SOURCE} ${INTEROP_HELPER_LIB_SOURCE} -t ${SO_FILES_OUTPUT}
    DEPENDS ets_interop_js_napi ${INTEROP_HELPER_LIB_SOURCE}
)

add_dependencies(ets_interop_js_gtest_launcher create_slinks_for_interop_gtest_launcher)
