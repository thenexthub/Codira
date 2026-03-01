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

ani_add_gtest(ani_test_promise_ops_new
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/promise_new_test.cpp
)

ani_add_gtest(ani_test_promise_ops_resolve
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/promise_resolve_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/promise_resolve_test.ets
)

ani_add_gtest(ani_test_promise_ops_reject
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/promise_reject_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/promise_reject_test.ets
)
