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

#include "graph_analyzer.h"

#include "compiler/optimizer/ir/basicblock.h"

#include "function.h"
#include "util/assert_util.h"

using namespace panda::guard;

namespace {
constexpr std::string_view TAG = "[Graph]";
constexpr std::string_view UI_COMPONENT_BASE_CLASS_VIEW_PU = "ViewPU";
constexpr std::string_view UI_COMPONENT_BASE_CLASS_VIEW_V2 = "ViewV2";
constexpr size_t PARAM_COUNT_THRESHOLD = 2;

using IntrinsicId = panda::compiler::RuntimeInterface::IntrinsicId;
using InstIdFilterList = std::vector<IntrinsicId>;
struct InstParam {
public:
    unsigned index;               // Target instruction: Index in the input instruction register list
    InstIdFilterList filterList;  // Input instruction: Type list
};

const InstIdFilterList INST_ID_LIST_DYNAMICIMPORT = {
    IntrinsicId::DYNAMICIMPORT,
};
const InstIdFilterList INST_ID_LIST_STLEXVAR = {IntrinsicId::STLEXVAR_IMM4_IMM4, IntrinsicId::STLEXVAR_IMM8_IMM8};
const InstIdFilterList INST_ID_LIST_LDOBJBYVALUE = {
    IntrinsicId::LDOBJBYVALUE_IMM8_V8,
    IntrinsicId::LDOBJBYVALUE_IMM16_V8,
};
const InstIdFilterList INST_ID_LIST_STOBJBYVALUE = {
    IntrinsicId::STOBJBYVALUE_IMM8_V8_V8,
    IntrinsicId::STOBJBYVALUE_IMM16_V8_V8,
};
const InstIdFilterList INST_ID_LIST_DEFINEGETTERSETTERBYVALUE = {
    IntrinsicId::DEFINEGETTERSETTERBYVALUE_V8_V8_V8_V8,
};
const InstIdFilterList INST_ID_LIST_ISIN = {
    IntrinsicId::ISIN_IMM8_V8,
};
const InstIdFilterList INST_ID_LIST_LDSUPERBYVALUE = {
    IntrinsicId::LDSUPERBYVALUE_IMM16_V8,
    IntrinsicId::LDSUPERBYVALUE_IMM8_V8,
};
const InstIdFilterList INST_ID_LIST_STSUPERBYVALUE = {
    IntrinsicId::STSUPERBYVALUE_IMM16_V8_V8,
    IntrinsicId::STSUPERBYVALUE_IMM8_V8_V8,
};
const InstIdFilterList INST_ID_LIST_STOWNBYNAME = {
    IntrinsicId::STOWNBYNAME_IMM16_ID16_V8,
    IntrinsicId::STOWNBYNAME_IMM8_ID16_V8,
};
const InstIdFilterList INST_ID_LIST_STOWNBYNAMEWITHNAMESET = {
    IntrinsicId::STOWNBYNAMEWITHNAMESET_IMM16_ID16_V8,
    IntrinsicId::STOWNBYNAMEWITHNAMESET_IMM8_ID16_V8,
};
const InstIdFilterList INST_ID_LIST_STOWNBYVALUE = {
    IntrinsicId::STOWNBYVALUE_IMM16_V8_V8,
    IntrinsicId::STOWNBYVALUE_IMM8_V8_V8,
};
const InstIdFilterList INST_ID_LIST_STOWNBYVALUEWITHNAMESET = {
    IntrinsicId::STOWNBYVALUEWITHNAMESET_IMM16_V8_V8,
    IntrinsicId::STOWNBYVALUEWITHNAMESET_IMM8_V8_V8,
};
const InstIdFilterList INST_ID_LIST_DEFINEFUNC = {
    IntrinsicId::DEFINEFUNC_IMM8_ID16_IMM8,
    IntrinsicId::DEFINEFUNC_IMM16_ID16_IMM8,
};
const InstIdFilterList INST_ID_LIST_DEFINEMETHOD = {
    IntrinsicId::DEFINEMETHOD_IMM16_ID16_IMM8,
    IntrinsicId::DEFINEMETHOD_IMM8_ID16_IMM8,
};
const InstIdFilterList INST_ID_LIST_CREATEOBJECTWITHBUFFER = {
    IntrinsicId::CREATEOBJECTWITHBUFFER_IMM16_ID16,
    IntrinsicId::CREATEOBJECTWITHBUFFER_IMM8_ID16,
};
const InstIdFilterList INST_ID_LIST_DEFINECLASSWITHBUFFER = {
    IntrinsicId::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8,
    IntrinsicId::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8,
};
const InstIdFilterList INST_ID_LIST_LDOBJBYNAME = {
    IntrinsicId::LDOBJBYNAME_IMM16_ID16,
    IntrinsicId::LDOBJBYNAME_IMM8_ID16,
};
const InstIdFilterList INST_ID_LIST_CALLRUNTIME_TOPROPERTYKEY = {
    IntrinsicId::CALLRUNTIME_TOPROPERTYKEY_PREF_NONE,
};
const InstIdFilterList INST_ID_LIST_TRYLDGLOBALBYNAME = {
    IntrinsicId::TRYLDGLOBALBYNAME_IMM8_ID16,
    IntrinsicId ::TRYLDGLOBALBYNAME_IMM16_ID16,
};
const InstIdFilterList INST_ID_LIST_DEFINEPROPERTYBYNAME = {
    IntrinsicId::DEFINEPROPERTYBYNAME_IMM8_ID16_V8,
};
const InstIdFilterList INST_ID_LIST_CALLRUNTIME_DEFINESENDABLECLASS = {
    IntrinsicId::CALLRUNTIME_DEFINESENDABLECLASS_PREF_IMM16_ID16_ID16_IMM16_V8,
};
const InstIdFilterList INST_ID_LIST_STMODULEVAR = {
    IntrinsicId::STMODULEVAR_IMM8,
    IntrinsicId::WIDE_STMODULEVAR_PREF_IMM16,
};

const InstIdFilterList INST_ID_LIST_STOBJBYNAME = {
    IntrinsicId::STOBJBYNAME_IMM8_ID16_V8,
    IntrinsicId::STOBJBYNAME_IMM16_ID16_V8,
};
const InstIdFilterList INST_ID_LIST_NEWOBJRANGE = {
    IntrinsicId::NEWOBJRANGE_IMM8_IMM8_V8,
    IntrinsicId::NEWOBJRANGE_IMM16_IMM8_V8,
};
const InstIdFilterList INST_ID_LIST_CALLTHIS1 = {
    IntrinsicId::CALLTHIS1_IMM8_V8_V8,
};
const InstIdFilterList INST_ID_LIST_CALLTHIS2 = {
    IntrinsicId::CALLTHIS2_IMM8_V8_V8_V8,
};
const InstIdFilterList INST_ID_LIST_CALLTHIS3 = {
    IntrinsicId::CALLTHIS3_IMM8_V8_V8_V8_V8,
};
const InstIdFilterList INST_ID_LIST_CALLARG0 = {
    IntrinsicId::CALLARG0_IMM8,
};
const InstIdFilterList INST_ID_LIST_CALLARG1 = {
    IntrinsicId::CALLARG1_IMM8_V8,
};
const InstIdFilterList INST_ID_LIST_CALLARGS2 = {
    IntrinsicId::CALLARGS2_IMM8_V8_V8,
};
const InstIdFilterList INST_ID_LIST_CALLARGS3 = {
    IntrinsicId::CALLARGS3_IMM8_V8_V8_V8,
};
const InstIdFilterList INST_ID_LIST_CALLRANGE = {
    IntrinsicId::CALLRANGE_IMM8_IMM8_V8,
};
const InstIdFilterList INST_ID_LIST_CALLRUNTIME_ISFALSE = {
    IntrinsicId::CALLRUNTIME_ISFALSE_PREF_IMM8,
};

const std::map<panda::pandasm::Opcode, InstIdFilterList> CALL_INST_MAP = {
    {panda::pandasm::Opcode::CALLTHIS1, INST_ID_LIST_CALLTHIS1},
    {panda::pandasm::Opcode::CALLTHIS2, INST_ID_LIST_CALLTHIS2},
    {panda::pandasm::Opcode::CALLTHIS3, INST_ID_LIST_CALLTHIS3},
    {panda::pandasm::Opcode::CALLARG1, INST_ID_LIST_CALLARG1},
    {panda::pandasm::Opcode::CALLARGS2, INST_ID_LIST_CALLARGS2},
    {panda::pandasm::Opcode::CALLARGS3, INST_ID_LIST_CALLARGS3},
    {panda::pandasm::Opcode::CALLRANGE, INST_ID_LIST_CALLRANGE},
};

const std::map<panda::pandasm::Opcode, InstParam> LDA_STA_INST_PARAM_MAP = {
    {panda::pandasm::Opcode::DYNAMICIMPORT, {0, INST_ID_LIST_DYNAMICIMPORT}},
    {panda::pandasm::Opcode::STLEXVAR, {0, INST_ID_LIST_STLEXVAR}},
    {panda::pandasm::Opcode::LDOBJBYVALUE, {1, INST_ID_LIST_LDOBJBYVALUE}},
    {panda::pandasm::Opcode::STOBJBYVALUE, {1, INST_ID_LIST_STOBJBYVALUE}},
    {panda::pandasm::Opcode::DEFINEGETTERSETTERBYVALUE, {1, INST_ID_LIST_DEFINEGETTERSETTERBYVALUE}},
    {panda::pandasm::Opcode::ISIN, {0, INST_ID_LIST_ISIN}},
    {panda::pandasm::Opcode::LDSUPERBYVALUE, {1, INST_ID_LIST_LDSUPERBYVALUE}},
    {panda::pandasm::Opcode::STSUPERBYVALUE, {1, INST_ID_LIST_STSUPERBYVALUE}},
    {panda::pandasm::Opcode::STOWNBYVALUE, {1, INST_ID_LIST_STOWNBYVALUE}},
    {panda::pandasm::Opcode::STOWNBYVALUEWITHNAMESET, {1, INST_ID_LIST_STOWNBYVALUEWITHNAMESET}}};

const std::vector<InstIdFilterList> DEFINE_INST_PASS_THROUGH_LIST = {
    INST_ID_LIST_DEFINEGETTERSETTERBYVALUE,
    INST_ID_LIST_DEFINEMETHOD,
};
const std::vector<InstIdFilterList> DEFINE_INST_ID_LIST = {
    INST_ID_LIST_DEFINEFUNC,
    INST_ID_LIST_DEFINECLASSWITHBUFFER,
    INST_ID_LIST_CREATEOBJECTWITHBUFFER,
    INST_ID_LIST_LDOBJBYNAME,  // XXX.prototype: get define ins by ldobjbyname of XXX object
    INST_ID_LIST_CALLRUNTIME_DEFINESENDABLECLASS,
};
const std::vector<InstIdFilterList> METHOD_NAME_INST_ID_LIST = {
    INST_ID_LIST_DEFINEGETTERSETTERBYVALUE, INST_ID_LIST_STOWNBYNAME,
    INST_ID_LIST_STOWNBYNAMEWITHNAMESET,    INST_ID_LIST_STOWNBYVALUE,
    INST_ID_LIST_STOWNBYVALUEWITHNAMESET,
};

/**
 * Is the instruction type in the list
 */
bool IsInstIdMatched(const panda::compiler::Inst *inst, const InstIdFilterList &list)
{
    if (!inst->IsIntrinsic()) {
        return false;
    }

    IntrinsicId instId = inst->CastToIntrinsic()->GetIntrinsicId();
    return std::any_of(list.begin(), list.end(), [&instId](IntrinsicId filter) -> bool { return instId == filter; });
}

bool IsInstIdMatched(const panda::compiler::Inst *inst, const std::vector<InstIdFilterList> &list)
{
    return std::any_of(list.begin(), list.end(),
                       [&](const InstIdFilterList &filter) -> bool { return IsInstIdMatched(inst, filter); });
}

/**
 * Does the current instruction match the requirement, Consistent with InstructionInfo
 * @param inst Graph instruction to be checked
 * @param list The instruction type that should be matched
 * @param targetIns The InstructionInfo that should be matched
 */
bool IsInstMatched(const panda::compiler::Inst *inst, const InstructionInfo &targetIns, const InstIdFilterList &list)
{
    if (!IsInstIdMatched(inst, list)) {
        return false;
    }

    const uint32_t pcVal = inst->GetPc();
    auto &pcInstMap = targetIns.function_->pcInstMap_;
    if (pcInstMap.find(pcVal) == pcInstMap.end() || pcInstMap.at(pcVal) != targetIns.index_) {
        return false;
    }

    return true;
}

/**
 * Traverse the Graph to find matching instructions, Converting InstructionInfo to Graph Instruction
 * @param inIns Instructions to be found
 * @param list instruction type
 * @param outInst Corresponding instructions in the graph
 */
void VisitGraph(const InstructionInfo &inIns, const InstIdFilterList &list, const panda::compiler::Inst *&outInst)
{
    Function *func = inIns.function_;
    panda::compiler::Graph *graph;
    func->GetGraph(graph);

    for (const panda::compiler::BasicBlock *bb : graph->GetBlocksLinearOrder()) {
        for (const panda::compiler::Inst *inst : bb->AllInsts()) {
            if (!IsInstMatched(inst, inIns, list)) {
                continue;
            }
            outInst = inst;
            return;
        }
    }

    PANDA_GUARD_ABORT_PRINT(TAG, ErrorCode::GENERIC_ERROR, "not find inst: " << inIns.index_ << " in " << func->idx_);
}

/**
 * Fill the graph instruction into InstructionInfo, Convert Graph instruction to InstructionInfo
 * @param inIns Used to obtain background information of InstructionInfo
 * @param target graph instruction
 * @param outIns InstructionInfo for filling
 */
void FillInstInfo(const InstructionInfo &inIns, const panda::compiler::Inst *target, InstructionInfo &outIns)
{
    const uint32_t pcVal = target->GetPc();
    Function *func = inIns.function_;
    PANDA_GUARD_ASSERT_PRINT(func->pcInstMap_.find(pcVal) == func->pcInstMap_.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "no valid target ins: " << func->idx_ << ", " << inIns.index_);
    func->FillInstInfo(func->pcInstMap_.at(pcVal), outIns);
}

/**
 * Print Graph instruction content
 */
std::string PrintInst(const panda::compiler::Inst *ins)
{
    std::stringstream stream;
    ins->Dump(&stream, false);
    return stream.str();
}

/**
 * Find the lda.str instruction corresponding to the input command
 */
void FindLdaStr(const panda::compiler::Inst *prevInst, const panda::compiler::Inst *&outIns)
{
    if (IsInstIdMatched(prevInst, INST_ID_LIST_CALLRUNTIME_TOPROPERTYKEY)) {
        prevInst = prevInst->GetInput(0).GetInst();
    }

    if (prevInst->GetOpcode() != panda::compiler::Opcode::CastValueToAnyType) {
        return;
    }
    const panda::compiler::Inst *targetInst = prevInst->GetInput(0).GetInst();
    if (targetInst->GetOpcode() != panda::compiler::Opcode::LoadString) {
        return;
    }

    outIns = targetInst;
}

void FillLdaStrInst(const InstructionInfo &inIns, const panda::compiler::Inst *curInst, uint32_t targetIndex,
                    InstructionInfo &out)
{
    const panda::compiler::Inst *prevInst = curInst->GetInput(targetIndex).GetInst();
    const panda::compiler::Inst *targetInst = nullptr;
    FindLdaStr(prevInst, targetInst);
    if (targetInst == nullptr) {
        LOG(INFO, PANDAGUARD) << TAG << "FillLdaStrInst failed: " << inIns.function_->idx_ << ", " << inIns.index_
                              << ";" << PrintInst(prevInst);
        return;
    }

    FillInstInfo(inIns, targetInst, out);
}

/**
 * Find the object definition instruction corresponding to the input instruction
 */
void FindDefineInst(const panda::compiler::Inst *prevInst, const panda::compiler::Inst *&outIns)
{
    if (IsInstIdMatched(prevInst, {INST_ID_LIST_CREATEOBJECTWITHBUFFER, INST_ID_LIST_DEFINECLASSWITHBUFFER,
                                   INST_ID_LIST_CALLRUNTIME_DEFINESENDABLECLASS})) {
        outIns = prevInst;
        return;
    }

    // Try to find class define in prototype instance
    if (!IsInstIdMatched(prevInst, INST_ID_LIST_LDOBJBYNAME)) {
        return;
    }

    const panda::compiler::Inst *targetIns = prevInst->GetInput(0).GetInst();
    if (!IsInstIdMatched(targetIns, INST_ID_LIST_DEFINECLASSWITHBUFFER)) {
        return;
    }
    outIns = targetIns;
}
}  // namespace

void GraphAnalyzer::GetLdaStr(const InstructionInfo &inIns, InstructionInfo &outIns, int index /* = -1 */)
{
    const auto it = LDA_STA_INST_PARAM_MAP.find(inIns.ins_->opcode);
    PANDA_GUARD_ASSERT_PRINT(it == LDA_STA_INST_PARAM_MAP.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "unsupported lda.str scene: " << inIns.ins_->ToString());

    const compiler::Inst *inst;
    VisitGraph(inIns, it->second.filterList, inst);

    const unsigned targetIndex = index >= 0 ? index : it->second.index;
    FillLdaStrInst(inIns, inst, targetIndex, outIns);
}

void GraphAnalyzer::HandleDefineMethod(const InstructionInfo &inIns, InstructionInfo &defineIns,
                                       InstructionInfo &nameIns)
{
    const compiler::Inst *inst;
    VisitGraph(inIns, INST_ID_LIST_DEFINEMETHOD, inst);

    for (const auto &userIns : inst->GetUsers()) {
        if (IsInstIdMatched(userIns.GetInst(), METHOD_NAME_INST_ID_LIST)) {
            FillInstInfo(inIns, userIns.GetInst(), nameIns);
            break;
        }
    }

    do {
        inst = inst->GetInput(0).GetInst();
    } while (IsInstIdMatched(inst, DEFINE_INST_PASS_THROUGH_LIST));
    PANDA_GUARD_ASSERT_PRINT(!IsInstIdMatched(inst, DEFINE_INST_ID_LIST), TAG, ErrorCode::GENERIC_ERROR,
                             "unexpect defineMethod scene: " << inIns.function_->idx_ << ", " << inIns.index_ << ";"
                                                             << PrintInst(inst));

    const compiler::Inst *targetInst = nullptr;
    FindDefineInst(inst, targetInst);
    PANDA_GUARD_ASSERT_PRINT(targetInst == nullptr, TAG, ErrorCode::GENERIC_ERROR,
                             "HandleDefineMethod failed: " << inIns.function_->idx_ << ", " << inIns.index_ << ";"
                                                           << PrintInst(inst));

    FillInstInfo(inIns, targetInst, defineIns);
}

void GraphAnalyzer::HandleDefineProperty(const InstructionInfo &inIns, InstructionInfo &defineIns)
{
    const compiler::Inst *inst;
    VisitGraph(inIns, INST_ID_LIST_DEFINEPROPERTYBYNAME, inst);

    const compiler::Inst *target = nullptr;
    FindDefineInst(inst->GetInput(0).GetInst(), target);
    if (target == nullptr) {
        return;
    }

    FillInstInfo(inIns, target, defineIns);
}

bool GraphAnalyzer::IsComponentClass(const InstructionInfo &inIns)
{
    if (inIns.ins_->opcode != pandasm::Opcode::DEFINECLASSWITHBUFFER) {
        return false;
    }

    const compiler::Inst *inst;
    VisitGraph(inIns, INST_ID_LIST_DEFINECLASSWITHBUFFER, inst);

    const compiler::Inst *tryldglobalbyname = inst->GetInput(0).GetInst();
    if (!IsInstIdMatched(tryldglobalbyname, INST_ID_LIST_TRYLDGLOBALBYNAME)) {
        return false;
    }

    InstructionInfo out;
    FillInstInfo(inIns, tryldglobalbyname, out);

    const std::string baseClassName = out.ins_->GetId(0);
    if (baseClassName != UI_COMPONENT_BASE_CLASS_VIEW_PU && baseClassName != UI_COMPONENT_BASE_CLASS_VIEW_V2) {
        return false;
    }

    LOG(INFO, PANDAGUARD) << TAG << "found UI component:" << inIns.ins_->GetId(0);

    return true;
}

void GraphAnalyzer::GetStModuleVarDefineIns(const InstructionInfo &inIns, InstructionInfo &defineIns)
{
    if (inIns.ins_->opcode != pandasm::Opcode::STMODULEVAR && inIns.ins_->opcode != pandasm::Opcode::WIDE_STMODULEVAR) {
        return;
    }

    const compiler::Inst *inst;
    VisitGraph(inIns, INST_ID_LIST_STMODULEVAR, inst);

    const compiler::Inst *target = inst->GetInput(0).GetInst();
    PANDA_GUARD_ASSERT_PRINT(target == nullptr, TAG, ErrorCode::GENERIC_ERROR, "find define inst failed");
    if (!IsInstIdMatched(target, {INST_ID_LIST_CREATEOBJECTWITHBUFFER, INST_ID_LIST_DEFINECLASSWITHBUFFER,
                                  INST_ID_LIST_CALLRUNTIME_DEFINESENDABLECLASS})) {
        return;
    }

    FillInstInfo(inIns, target, defineIns);
}

void GraphAnalyzer::GetStObjByNameDefineIns(const InstructionInfo &inIns, InstructionInfo &defineIns)
{
    if (inIns.notEqualToOpcode(pandasm::Opcode::STOBJBYNAME)) {
        return;
    }

    const compiler::Inst *inst;
    VisitGraph(inIns, INST_ID_LIST_STOBJBYNAME, inst);
    const compiler::Inst *target = inst->GetInput(1).GetInst();
    if (!IsInstIdMatched(target, DEFINE_INST_ID_LIST)) {
        return;
    }

    FillInstInfo(inIns, target, defineIns);
}

void GraphAnalyzer::GetStObjByNameInput(const InstructionInfo &inIns, InstructionInfo &out)
{
    const compiler::Inst *inst;
    VisitGraph(inIns, INST_ID_LIST_STOBJBYNAME, inst);

    const compiler::Inst *target = inst->GetInput(INDEX_1).GetInst();
    if (!IsInstIdMatched(target, {INST_ID_LIST_NEWOBJRANGE, INST_ID_LIST_CALLTHIS2, INST_ID_LIST_CALLTHIS3})) {
        return;
    }
    FillInstInfo(inIns, target, out);
}

void GraphAnalyzer::GetNewObjRangeInfo(const InstructionInfo &inIns, std::string &name, InstructionInfo &out)
{
    const compiler::Inst *inst;
    VisitGraph(inIns, INST_ID_LIST_NEWOBJRANGE, inst);

    const compiler::Inst *tryInst = inst->GetInput(INDEX_0).GetInst();
    if (!IsInstIdMatched(tryInst, INST_ID_LIST_TRYLDGLOBALBYNAME)) {
        return;
    }
    InstructionInfo tryInstInfo;
    FillInstInfo(inIns, tryInst, tryInstInfo);
    name = tryInstInfo.ins_->GetId(INDEX_0);
    if (name.empty()) {
        return;
    }

    FillLdaStrInst(inIns, inst, INDEX_3, out);
}

std::string GraphAnalyzer::GetCallName(const InstructionInfo &inIns)
{
    const auto it = CALL_INST_MAP.find(inIns.ins_->opcode);
    PANDA_GUARD_ASSERT_PRINT(it == CALL_INST_MAP.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "unsupported call name scene: " << inIns.ins_->ToString());

    const compiler::Inst *callInst;
    VisitGraph(inIns, it->second, callInst);

    uint32_t inputCnt = callInst->GetInputsCount();
    PANDA_GUARD_ASSERT_PRINT(inputCnt < PARAM_COUNT_THRESHOLD, TAG, ErrorCode::GENERIC_ERROR,
                             "unexpected call param count scene: " << inIns.ins_->ToString());

    const compiler::Inst *callNameInst = callInst->GetInput(inputCnt - 2).GetInst();
    if (IsInstIdMatched(callNameInst, {INST_ID_LIST_CALLARG0, INST_ID_LIST_CALLARG1})) {
        inputCnt = callNameInst->GetInputsCount();
        PANDA_GUARD_ASSERT_PRINT(inputCnt < PARAM_COUNT_THRESHOLD, TAG, ErrorCode::GENERIC_ERROR,
                                 "unexpected call args param count scene: " << inIns.ins_->ToString());
        callNameInst = callNameInst->GetInput(inputCnt - PARAM_COUNT_THRESHOLD).GetInst();
    }
    if (!IsInstIdMatched(callNameInst, {INST_ID_LIST_LDOBJBYNAME, INST_ID_LIST_TRYLDGLOBALBYNAME})) {
        return "";
    }

    InstructionInfo nameIns;
    FillInstInfo(inIns, callNameInst, nameIns);
    return nameIns.ins_->GetId(INDEX_0);
}

void GraphAnalyzer::GetCallLdaStrParam(const InstructionInfo &inIns, uint32_t paramIndex, InstructionInfo &out)
{
    const auto it = CALL_INST_MAP.find(inIns.ins_->opcode);
    PANDA_GUARD_ASSERT_PRINT(it == CALL_INST_MAP.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "unsupported get call lda str param scene: " << inIns.ins_->ToString());

    const compiler::Inst *callInst;
    VisitGraph(inIns, it->second, callInst);

    FillLdaStrInst(inIns, callInst, paramIndex, out);
}
void GraphAnalyzer::GetCallTryLdGlobalByNameParam(const InstructionInfo &inIns, uint32_t paramIndex,
                                                  InstructionInfo &out)
{
    const auto it = CALL_INST_MAP.find(inIns.ins_->opcode);
    PANDA_GUARD_ASSERT_PRINT(it == CALL_INST_MAP.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "unsupported get call tryldblobalbyname param scene: " << inIns.ins_->ToString());

    const compiler::Inst *callInst;
    VisitGraph(inIns, it->second, callInst);

    const compiler::Inst *target = callInst->GetInput(paramIndex).GetInst();
    if (!IsInstIdMatched(target, INST_ID_LIST_TRYLDGLOBALBYNAME)) {
        return;
    }
    FillInstInfo(inIns, target, out);
}

void GraphAnalyzer::GetCallLdObjByNameParam(const InstructionInfo &inIns, uint32_t paramIndex, InstructionInfo &out)
{
    const auto it = CALL_INST_MAP.find(inIns.ins_->opcode);
    PANDA_GUARD_ASSERT_PRINT(it == CALL_INST_MAP.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "unsupported get call ldobjbyname param scene: " << inIns.ins_->ToString());

    const compiler::Inst *callInst;
    VisitGraph(inIns, it->second, callInst);

    const compiler::Inst *target = callInst->GetInput(paramIndex).GetInst();
    if (!IsInstIdMatched(target, INST_ID_LIST_LDOBJBYNAME)) {
        return;
    }
    FillInstInfo(inIns, target, out);
}

void GraphAnalyzer::GetIsInInfo(const InstructionInfo &inIns, std::vector<InstructionInfo> &out)
{
    const compiler::Inst *isinInst;
    VisitGraph(inIns, INST_ID_LIST_ISIN, isinInst);

    InstructionInfo ldStrInstInfo;
    FillLdaStrInst(inIns, isinInst, INDEX_0, ldStrInstInfo);
    if (!ldStrInstInfo.IsValid()) {
        return;
    }

    const compiler::Inst *isFalseInst = isinInst->GetUsers().Front().GetInst();
    if (!IsInstIdMatched(isFalseInst, INST_ID_LIST_CALLRUNTIME_ISFALSE)) {
        return;
    }

    const compiler::Inst *ldInst = nullptr;
    auto falseBlock = isinInst->GetBasicBlock()->GetFalseSuccessor();
    for (const compiler::Inst *inst : falseBlock->AllInsts()) {
        if (!IsInstIdMatched(inst, INST_ID_LIST_LDOBJBYNAME)) {
            ldInst = inst;
            break;
        }
    }
    if (!ldInst) {
        return;
    }
    InstructionInfo ldInstInfo;
    FillInstInfo(inIns, ldInst, ldInstInfo);
    if (!ldInstInfo.IsValid()) {
        return;
    }

    const compiler::Inst *phiInst = ldInst->GetUsers().Front().GetInst();
    if (!phiInst->IsPhi()) {
        return;
    }

    const compiler::Inst *stInst = phiInst->GetUsers().Front().GetInst();
    if (!IsInstIdMatched(stInst, INST_ID_LIST_STOBJBYNAME)) {
        return;
    }
    InstructionInfo stInstInfo;
    FillInstInfo(inIns, stInst, stInstInfo);
    if (!stInstInfo.IsValid()) {
        return;
    }

    if ((ldStrInstInfo.ins_->GetId(INDEX_0) == ldInstInfo.ins_->GetId(INDEX_0)) &&
        (ldInstInfo.ins_->GetId(INDEX_0) == stInstInfo.ins_->GetId(INDEX_0))) {
        out.emplace_back(ldStrInstInfo);
        out.emplace_back(ldInstInfo);
        out.emplace_back(stInstInfo);
    }
}
void GraphAnalyzer::GetCallCreateObjectWithBufferParam(const InstructionInfo &inIns, uint32_t paramIndex,
                                                       InstructionInfo &out)
{
    const auto it = CALL_INST_MAP.find(inIns.ins_->opcode);
    PANDA_GUARD_ASSERT_PRINT(it == CALL_INST_MAP.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "unsupported get call createobjectwithbuffer param scene: " << inIns.ins_->ToString());

    const compiler::Inst *callInst;
    VisitGraph(inIns, it->second, callInst);

    const compiler::Inst *target = callInst->GetInput(paramIndex).GetInst();
    if (!IsInstIdMatched(target, INST_ID_LIST_CREATEOBJECTWITHBUFFER)) {
        return;
    }
    FillInstInfo(inIns, target, out);
}

void GraphAnalyzer::GetDefinePropertyByNameFunction(const InstructionInfo &inIns, InstructionInfo &out)
{
    if (inIns.notEqualToOpcode(pandasm::Opcode::DEFINEPROPERTYBYNAME)) {
        return;
    }

    const compiler::Inst *inst;
    VisitGraph(inIns, INST_ID_LIST_DEFINEPROPERTYBYNAME, inst);

    const compiler::Inst *target = inst->GetInput(INDEX_1).GetInst();
    if (!IsInstIdMatched(target, INST_ID_LIST_DEFINEFUNC)) {
        return;
    }
    FillInstInfo(inIns, target, out);
}
