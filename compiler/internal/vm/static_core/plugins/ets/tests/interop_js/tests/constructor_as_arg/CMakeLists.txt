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

panda_ets_interop_js_gtest(ets_interop_js_parent_class_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/parent_class/parent_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/parent_class/parent_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js_child_class_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/child_class/child_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/child_class/child_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js_anonymous_class_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/anonymous_class/anonymous_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/anonymous_class/anonymous_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)
# IIFE --> iife in the name of target because requireNapiPreview cant work with capital letters
panda_ets_interop_js_gtest(ets_interop_js_iife_class_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/IIFE_class/IIFE_class.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/IIFE_class/IIFE_class.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js_method_constructor_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/method_constructor/method_constructor.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/method_constructor/method_constructor.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js_function_arg_constructor_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function/function.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/function/function.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js_constructor_as_arg_error_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/error/error.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/error/error.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)
