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

if (NOT PANDA_JS_ETS_HYBRID_MODE)
   message(FATAL_ERROR, "Interop tests supported only with PANDA_JS_ETS_HYBRID_MODE flag")
endif() 

include(cmake/import.cmake)

set(INTEROP_TESTS_DIR "${PANDA_BINARY_ROOT}/tests/ets_interop_js")
set(ES2ABC ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/interop_js/es2abc)
set(ARK_JS_NAPI_CLI ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/interop_js/ark_js_napi_cli)
set(ARK_JS_STUB_FILE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/interop_js/stub.an)

add_subdirectory(gtest_plugin)
add_subdirectory(tests)
