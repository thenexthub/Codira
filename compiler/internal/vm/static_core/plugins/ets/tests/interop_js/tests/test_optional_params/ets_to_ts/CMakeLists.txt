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

panda_ets_interop_js_gtest(ets_interop_js__optional_parameter_ets_to_ts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_optional_params_ets_to_ts.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/optional_params.ets
    TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test_optional_params_ets_to_ts.ts
)