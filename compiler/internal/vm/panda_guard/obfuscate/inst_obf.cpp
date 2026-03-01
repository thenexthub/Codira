/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "inst_obf.h"

#include "configs/guard_context.h"
#include "graph_analyzer.h"
#include "program.h"

using namespace panda::guard;
namespace {
using FunctionProcessor = void(InstructionInfo &inst);
using ProcessorMap = std::map<panda::pandasm::Opcode, const std::function<FunctionProcessor>>;

constexpr auto PROCESSOR_DEFAULT = [](InstructionInfo &inst) { inst.UpdateInsName(false); };

constexpr auto PROCESSOR_DEFAULT_LDA_STR = [](InstructionInfo &inst) {
    InstructionInfo outInfo;
    GraphAnalyzer::GetLdaStr(inst, outInfo);
    if (!outInfo.IsValid()) {
        return;
    }
    outInfo.UpdateInsName(false);
};

constexpr auto PROCESSOR_FILE_PATH_LDA_STR = [](InstructionInfo &inst) {
    InstructionInfo outInfo;
    GraphAnalyzer::GetLdaStr(inst, outInfo);
    if (!outInfo.IsValid()) {
        return;
    }
    outInfo.UpdateInsFileName();
};

const ProcessorMap INST_PROCESSOR_MAP = {
    {panda::pandasm::Opcode::LDOBJBYNAME, PROCESSOR_DEFAULT},
    {panda::pandasm::Opcode::THROW_UNDEFINEDIFHOLEWITHNAME, PROCESSOR_DEFAULT},
    {panda::pandasm::Opcode::LDSUPERBYNAME, PROCESSOR_DEFAULT},
    {panda::pandasm::Opcode::LDOBJBYVALUE, PROCESSOR_DEFAULT_LDA_STR},
    {panda::pandasm::Opcode::ISIN, PROCESSOR_DEFAULT_LDA_STR},
    {panda::pandasm::Opcode::LDSUPERBYVALUE, PROCESSOR_DEFAULT_LDA_STR},
    {panda::pandasm::Opcode::DYNAMICIMPORT, PROCESSOR_FILE_PATH_LDA_STR},
};
}  // namespace

void InstObf::UpdateInst(InstructionInfo &inst)
{
    auto it = INST_PROCESSOR_MAP.find(inst.ins_->opcode);
    if (it == INST_PROCESSOR_MAP.end()) {
        return;
    }
    it->second(inst);
}
