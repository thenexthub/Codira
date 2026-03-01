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

add_custom_target(ir_builder_coverage DEPENDS compiler_unit_tests)
add_custom_target(pbc_2_ir_doc_gen)

add_custom_command(TARGET ir_builder_coverage POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    # CC-OFFNXT(bc-40032) false positive
    COMMAND bash ${PANDA_ROOT}/compiler/tools/ir_builder_coverage.sh --binary-dir=${PANDA_BINARY_ROOT} --root-dir=${PANDA_ROOT}
)

add_custom_command(TARGET pbc_2_ir_doc_gen POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    # CC-OFFNXT(bc-40032) false positive
    COMMAND bash ${PANDA_ROOT}/compiler/tools/pbc_2_ir_doc_gen.sh --binary-dir=${PANDA_BINARY_ROOT} --root-dir=${PANDA_ROOT}
)
