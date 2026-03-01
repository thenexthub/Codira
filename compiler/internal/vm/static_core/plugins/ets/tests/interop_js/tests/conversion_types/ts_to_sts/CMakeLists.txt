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

panda_ets_interop_js_gtest(ets_interop_js__conversion_int_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_int/conversion_int.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_int/conversion_int.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__conversion_array_int_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_array_int/conversion_array_int.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_array_int/conversion_array_int.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__conversion_binary_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_binary/conversion_binary.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_binary/conversion_binary.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__conversion_hexadecimal_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_hexadecimal/conversion_hexadecimal.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_hexadecimal/conversion_hexadecimal.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__conversion_string_to_int_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_string_to_int/conversion_string_to_int.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_string_to_int/conversion_string_to_int.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__conversion_byte_to_int_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_byte/conversion_byte.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_byte/conversion_byte.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__conversion_float_ts_to_ets
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_float/conversion_float.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_float/conversion_float.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__conversion_big_int_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_big_int/conversion_big_int.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_big_int/conversion_big_int.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)

panda_ets_interop_js_gtest(ets_interop_js__conversion_exponential_ts_to_sts
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_exponential/conversion_exponential.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/conversion_exponential/conversion_exponential.ets
    ETS_CONFIG ${ETS_CONFIG}
    JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.js
)
