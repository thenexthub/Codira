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

if (NOT PANDA_TARGET_ARM32)
    ets_native_test_helper(ets_native_exclusive_worker_tests
        CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/exclusive_worker_tests.cpp
        ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/exclusive_worker_tests.ets
        ETS_GTEST_ABC_PATH "ANI_GTEST_ABC_PATH"
        VERIFY_SOURCES true
    )
endif()
