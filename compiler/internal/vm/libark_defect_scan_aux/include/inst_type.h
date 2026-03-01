/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBARK_DEFECT_SCAN_AUX_INCLUDE_INST_TYPE_H
#define LIBARK_DEFECT_SCAN_AUX_INCLUDE_INST_TYPE_H

#include "compiler/optimizer/ir/inst.h"
#include "intrinsic_map_table.h"
#include "opcode_map_table.h"

namespace panda::defect_scan_aux {
#define INSTTYPE_ENUM(x, y) y,
#define BUILD_OPCODE_MAP_TABLE(x, y) {Opcode::x, InstType::y},
#define BUILD_INTRINSIC_MAP_TABLE(x, y) {IntrinsicId::x, InstType::y},

using Opcode = compiler::Opcode;
using IntrinsicId = compiler::RuntimeInterface::IntrinsicId;
enum class InstType {
    OPCODE_INSTTYPE_MAP_TABLE(INSTTYPE_ENUM)
    INTRINSIC_INSTTYPE_MAP_TABLE(INSTTYPE_ENUM)
    COUNT,
    INVALID_TYPE,
};
}  // namespace panda::defect_scan_aux

#endif  // LIBARK_DEFECT_SCAN_AUX_INCLUDE_INST_TYPE_H