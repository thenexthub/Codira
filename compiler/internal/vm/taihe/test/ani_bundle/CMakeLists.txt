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

include(../../cmake/TaiheWorkflows.cmake)
setup_taihe_cmake_test_env()
cmake_minimum_required(VERSION 3.18)
project(ani_bundle)

set(IDL_FILES
    "${CMAKE_CURRENT_LIST_DIR}/idl/abilityInfo.taihe"
    "${CMAKE_CURRENT_LIST_DIR}/idl/applicationInfo.taihe"
    "${CMAKE_CURRENT_LIST_DIR}/idl/bundleInfo.taihe"
    "${CMAKE_CURRENT_LIST_DIR}/idl/customizeData.taihe"
    "${CMAKE_CURRENT_LIST_DIR}/idl/elementName.taihe"
    "${CMAKE_CURRENT_LIST_DIR}/idl/hapModuleInfo.taihe"
    "${CMAKE_CURRENT_LIST_DIR}/idl/moduleInfo.taihe"
    "${CMAKE_CURRENT_LIST_DIR}/idl/ohos_app_ability_want.taihe"
    "${CMAKE_CURRENT_LIST_DIR}/idl/ohos_bundle.taihe"
    "${CMAKE_CURRENT_LIST_DIR}/idl/shortcutInfo.taihe"
)

set(GEN_ETS_FILES
    "abilityInfo.ets"
    "applicationInfo.ets"
    "bundleInfo.ets"
    "customizeData.ets"
    "elementName.ets"
    "hapModuleInfo.ets"
    "moduleInfo.ets"
    "ohos_app_ability_want.ets"
    "ohos_bundle.ets"
    "shortcutInfo.ets"
)

set(USER_ETS_FILES
    "${CMAKE_CURRENT_LIST_DIR}/user/main.ets"
)

file(GLOB AUTHOR_CPP_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/author/src/*.cpp")

add_ani_demo(ani_bundle "${IDL_FILES}" "" "${GEN_ETS_FILES}" "${USER_ETS_FILES}" "" "${AUTHOR_CPP_FILES}")
