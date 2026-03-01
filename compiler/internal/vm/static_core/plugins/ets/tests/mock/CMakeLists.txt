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

add_custom_target(ets_test_napi_mock_run)

panda_ets_napi_add_gtest(
    NAME stack_trace_test
    CPP_SOURCE stack_trace_test.cpp
    ETS_SOURCE StackTraceTest.ets
)

panda_ets_napi_add_gtest(
    NAME js_mode_launch
    CPP_SOURCE js_mode_launch.cpp
    ETS_SOURCE js_mode_launch.ets
)


# NOTE(@srokashevich, #29097): enable with asan after fix
if (NOT PANDA_ENABLE_ADDRESS_SANITIZER)
ani_add_gtest(ani_test_unhandled_rejection_recursive
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/unhandled_rejection_recursive_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/UnhandledRejectionRecursiveTest.ets
)
endif()

panda_ets_napi_add_gtest(
    NAME uncaught_error_handler
    CPP_SOURCE uncaught_error_handler_test.cpp
    ETS_SOURCE UncaughtErrorHandlerTest.ets
)

if (NOT ENABLE_UNIT_TESTS_FULL_COVERAGE)
    panda_ets_napi_add_gtest(
        NAME register_native_allocation_test
        CPP_SOURCE register_native_allocation_test.cpp
        ETS_SOURCE RegisterNativeAllocationTest.ets
    )
    panda_ets_napi_add_gtest(
        NAME coroswitch_in_cctor
        CPP_SOURCE coroswitch_in_cctor.cpp
        ETS_SOURCE coroswitch_in_cctor.ets
    )
    panda_ets_napi_add_gtest(
        NAME abort_flag_launch
        CPP_SOURCE abort_flag_launch_test.cpp
        ETS_SOURCE AbortFlagLaunchTest.ets
    )
endif()
