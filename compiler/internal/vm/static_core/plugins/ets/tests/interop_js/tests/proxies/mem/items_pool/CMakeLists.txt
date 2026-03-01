# Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

panda_ets_add_gtest(
    NAME ets_interop_js__test_items_pool
    SOURCES test_items_pool.cpp
    INCLUDE_DIRS ${PANDA_ROOT}
    LIBRARIES arkbase
    SANITIZERS ${PANDA_SANITIZERS_LIST}
)
add_dependencies(ets_interop_js_gtests ets_interop_js__test_items_pool_gtests)
