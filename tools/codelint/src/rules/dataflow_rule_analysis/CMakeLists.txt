# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

include_directories(${CMAKE_SOURCE_DIR}/src)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
aux_source_directory(./ ALL_SRCS)
aux_source_directory(./dataflow_rule_G_CHK_01 G_CHK_01)
aux_source_directory(./dataflow_rule_G_SER_03 G_SER_03)
aux_source_directory(./dataflow_rule_G_FIO_01 G_FIO_01)
aux_source_directory(./dataflow_rule_P_02 P_02)
aux_source_directory(./dataflow_rule_G_OTH_01 G_OTH_01)
aux_source_directory(./dataflow_rule_G_VAR_01 G_VAR_01)
aux_source_directory(./dataflow_rule_G_CHK_03 G_CHK_03)
add_library(dataflowrule STATIC ${ALL_SRCS} ${G_CHK_01} ${G_SER_03} ${G_FIO_01} ${P_02} ${G_OTH_01} ${G_VAR_01} ${G_CHK_03})
if(COVERAGE_FLAG)
    target_compile_options(dataflowrule PUBLIC "-fno-exceptions")
endif()
