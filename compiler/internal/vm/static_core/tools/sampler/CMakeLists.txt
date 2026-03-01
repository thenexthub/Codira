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

cmake_minimum_required(VERSION 3.14.1 FATAL_ERROR)
project(aspt_converter CXX)

panda_add_executable(aspt_converter main.cpp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
panda_add_library(aspt_converter_static STATIC
    aspt_converter.cpp
    trace_dumper.cpp
)
panda_target_link_libraries(aspt_converter_static arkfile arkbase)
panda_target_include_directories(aspt_converter_static PUBLIC ${PANDA_BINARY_ROOT})

panda_add_sanitizers(TARGET aspt_converter SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_add_sanitizers(TARGET aspt_converter_static SANITIZERS ${PANDA_SANITIZERS_LIST})

panda_gen_options(TARGET aspt_converter_static YAML_FILE options.yaml GENERATED_HEADER
                  aspt_converter_options.h)

panda_target_link_libraries(aspt_converter aspt_converter_static)
