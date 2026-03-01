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

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/arktsconfig.in.json ${ETS_CONFIG})

panda_ets_interop_js_gtest(ets_interop_js__user_class_method_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/user_class/user_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/user_class/user_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__child_class_method_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/child_class/child_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/child_class/child_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__interface_class_method_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/interface_class/interface_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/interface_class/interface_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__put_another_instance_class_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/put_another_instance_class/put_another_instance_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/put_another_instance_class/put_another_instance_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__static_class_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/static_class/static_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/static_class/static_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__private_class_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/private_class/private_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/private_class/private_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__protected_class_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/protected_class/protected_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/protected_class/protected_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__child_protected_class_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/child_protected_class/child_protected_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/child_protected_class/child_protected_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__abstract_class_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/abstract_class/abstract_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/abstract_class/abstract_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)
