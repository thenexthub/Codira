# Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

set(ETS_EXT_SRCS ${PANDA_ETS_PLUGIN_SOURCE}/verification)
set(ETS_SOURCES
  ${ETS_EXT_SRCS}/ets_plugin.cpp
)

panda_target_sources(arkruntime_obj PRIVATE ${ETS_SOURCES})
panda_target_include_directories(arkruntime_obj PUBLIC
  ${PANDA_ETS_PLUGIN_SOURCE}
)
