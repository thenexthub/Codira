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

add_custom_target(ets_interop_tests COMMENT "Common target to run ETS interop tests")

include(cmake/interop_js_tests_arkjsvm.cmake)
include(cmake/interop_js_checked_tests.cmake)

function(gen_file)
    cmake_parse_arguments(
        ARG
        ""
        "TARGET;TEMPLATE;OUTPUT;GENERATOR"
        "REQUIRES"
        ${ARGN}
    )

    if (NOT DEFINED ARG_GENERATOR)
        set(ARG_GENERATOR "${PANDA_ETS_PLUGIN_SOURCE}/tests/interop_js/gen_file.rb")
    endif()

    string(REPLACE ";" "," REQUIRE_STR "${ARG_REQUIRES}")

    add_custom_command(OUTPUT ${ARG_OUTPUT}
        COMMAND ${ARG_GENERATOR} ${ARG_TEMPLATE} ${ARG_OUTPUT} ${REQUIRE_STR}
        DEPENDS ${ARG_GENERATOR} ${ARG_TEMPLATE} ${ARG_REQUIRES}
    )

    add_custom_target(${ARG_TARGET}
        DEPENDS ${ARG_OUTPUT}
    )

    add_dependencies(panda_gen_files ${ARG_TARGET})
endfunction(gen_file)
