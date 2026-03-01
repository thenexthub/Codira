/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "libabckit/src/adapter_static/helpers_static.h"
#include "libabckit/src/adapter_static/string_util.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/src/macros.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/wrappers/pandasm_wrapper.h"

#include "static_core/assembler/assembly-program.h"
#include "static_core/assembler/mangling.h"
#include "static_core/bytecode_optimizer/reg_acc_alloc.h"
#include "static_core/compiler/optimizer/ir/graph.h"
#include "static_core/compiler/optimizer/ir/basicblock.h"
#include "static_core/compiler/optimizer/ir/marker.h"
#include "static_core/compiler/optimizer/optimizations/regalloc/reg_alloc_graph_coloring.h"
#include "static_core/compiler/optimizer/optimizations/regalloc/reg_alloc_resolver.h"
#include "static_core/compiler/optimizer/analysis/loop_analyzer.h"
#include "static_core/compiler/optimizer/analysis/dominators_tree.h"
#include "static_core/compiler/optimizer/analysis/linear_order.h"
#include "static_core/compiler/optimizer/analysis/rpo.h"

#include "abckit_intrinsics_opcodes.inc"
#include "name_util.h"

static constexpr uint32_t IC_SLOT_VALUE = 0xF;

namespace libabckit {

std::tuple<std::string, std::string> ClassGetNames(const std::string &fullName)
{
    static const std::string GLOBAL_CLASS_NAME = "ETSGLOBAL";
    {
        const std::string kSep = ".@ohos.";
        auto sepPos = fullName.rfind(kSep);
        if (sepPos != std::string::npos) {
            return {fullName.substr(0, sepPos), fullName.substr(sepPos + 1)};
        }
    }
    std::string::size_type pos = fullName.rfind('.');
    if (pos == std::string::npos) {
        return {".", fullName};
    }
    if (pos < GLOBAL_CLASS_NAME.size()) {
        return {fullName.substr(0, pos), fullName.substr(pos + 1)};
    }
    auto rawModuleName = fullName.substr(pos - GLOBAL_CLASS_NAME.size(), GLOBAL_CLASS_NAME.size());
    if (rawModuleName == GLOBAL_CLASS_NAME) {
        std::string::size_type pos2 = fullName.substr(0, pos).rfind('.');
        return {fullName.substr(0, pos2), fullName.substr(pos2 + 1)};
    }
    return {fullName.substr(0, pos), fullName.substr(pos + 1)};
}

std::tuple<std::string, std::string> FuncGetNames(const std::string &fullSig)
{
    auto fullName = fullSig.substr(0, fullSig.find(':'));
    std::string::size_type pos = fullName.rfind('.');
    if (pos == std::string::npos) {
        return {".", "ETSGLOBAL"};
    }
    return ClassGetNames(fullName.substr(0, pos));
}

std::string FuncNameCropModule(const std::string &fullSig)
{
    auto fullName = fullSig.substr(0, fullSig.find(':'));
    std::string::size_type dotPos = fullName.rfind('.');
    if (dotPos != std::string::npos) {
        return fullSig.substr(dotPos + 1);
    }
    return fullSig;
}

void CheckInvalidOpcodes([[maybe_unused]] ark::compiler::Graph *graph, [[maybe_unused]] bool isDynamic)
{
#ifndef NDEBUG
    for (auto *bb : graph->GetBlocksRPO()) {
        for (auto *inst : bb->AllInsts()) {
            bool isInvalid = isDynamic ? (GetDynamicOpcode(inst) == ABCKIT_ISA_API_DYNAMIC_OPCODE_INVALID)
                                       : (GetStaticOpcode(inst) == ABCKIT_ISA_API_STATIC_OPCODE_INVALID);
            if (isInvalid) {
                std::ostringstream out;
                LIBABCKIT_LOG_DUMP(inst->DumpOpcode(&out), DEBUG);
                LIBABCKIT_LOG(DEBUG) << "ASSERTION FAILED: Invalid opcode encountered: " << out.str() << std::endl;
                ASSERT(false);
            }
        }
    }
#endif
}

// CC-OFFNXT(G.FUD.05) huge function, big switch-case
// CC-OFFNXT(G.FUN.01-CPP) huge function, big switch-case
AbckitIsaApiStaticOpcode GetStaticOpcode(ark::compiler::Inst *inst)
{
    auto opcode = inst->GetOpcode();
    switch (opcode) {
        case ark::compiler::Opcode::CallStatic:
            return ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC;
        case ark::compiler::Opcode::CallVirtual:
            return ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL;
        case ark::compiler::Opcode::LoadStatic:
            return ABCKIT_ISA_API_STATIC_OPCODE_LOADSTATIC;
        case ark::compiler::Opcode::LoadString:
            return ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING;
        case ark::compiler::Opcode::LoadObject:
            return ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT;
        case ark::compiler::Opcode::Sub:
            return ABCKIT_ISA_API_STATIC_OPCODE_SUB;
        case ark::compiler::Opcode::ReturnVoid:
            return ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID;
        case ark::compiler::Opcode::Parameter:
            return ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER;
        case ark::compiler::Opcode::Constant:
            return ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT;
        case ark::compiler::Opcode::Cmp:
            return ABCKIT_ISA_API_STATIC_OPCODE_CMP;
        case ark::compiler::Opcode::Cast:
            return ABCKIT_ISA_API_STATIC_OPCODE_CAST;
        case ark::compiler::Opcode::Return:
            return ABCKIT_ISA_API_STATIC_OPCODE_RETURN;
        case ark::compiler::Opcode::Add:
            return ABCKIT_ISA_API_STATIC_OPCODE_ADD;
        case ark::compiler::Opcode::Mul:
            return ABCKIT_ISA_API_STATIC_OPCODE_MUL;
        case ark::compiler::Opcode::Mod:
            return ABCKIT_ISA_API_STATIC_OPCODE_MOD;
        case ark::compiler::Opcode::Div:
            return ABCKIT_ISA_API_STATIC_OPCODE_DIV;
        case ark::compiler::Opcode::Neg:
            return ABCKIT_ISA_API_STATIC_OPCODE_NEG;
        case ark::compiler::Opcode::AddI:
            return ABCKIT_ISA_API_STATIC_OPCODE_ADDI;
        case ark::compiler::Opcode::DivI:
            return ABCKIT_ISA_API_STATIC_OPCODE_DIVI;
        case ark::compiler::Opcode::SubI:
            return ABCKIT_ISA_API_STATIC_OPCODE_SUBI;
        case ark::compiler::Opcode::MulI:
            return ABCKIT_ISA_API_STATIC_OPCODE_MULI;
        case ark::compiler::Opcode::ModI:
            return ABCKIT_ISA_API_STATIC_OPCODE_MODI;
        case ark::compiler::Opcode::Shl:
            return ABCKIT_ISA_API_STATIC_OPCODE_SHL;
        case ark::compiler::Opcode::Shr:
            return ABCKIT_ISA_API_STATIC_OPCODE_SHR;
        case ark::compiler::Opcode::AShr:
            return ABCKIT_ISA_API_STATIC_OPCODE_ASHR;
        case ark::compiler::Opcode::ShlI:
            return ABCKIT_ISA_API_STATIC_OPCODE_SHLI;
        case ark::compiler::Opcode::ShrI:
            return ABCKIT_ISA_API_STATIC_OPCODE_SHRI;
        case ark::compiler::Opcode::AShrI:
            return ABCKIT_ISA_API_STATIC_OPCODE_ASHRI;
        case ark::compiler::Opcode::And:
            return ABCKIT_ISA_API_STATIC_OPCODE_AND;
        case ark::compiler::Opcode::Or:
            return ABCKIT_ISA_API_STATIC_OPCODE_OR;
        case ark::compiler::Opcode::Xor:
            return ABCKIT_ISA_API_STATIC_OPCODE_XOR;
        case ark::compiler::Opcode::AndI:
            return ABCKIT_ISA_API_STATIC_OPCODE_ANDI;
        case ark::compiler::Opcode::OrI:
            return ABCKIT_ISA_API_STATIC_OPCODE_ORI;
        case ark::compiler::Opcode::XorI:
            return ABCKIT_ISA_API_STATIC_OPCODE_XORI;
        case ark::compiler::Opcode::Not:
            return ABCKIT_ISA_API_STATIC_OPCODE_NOT;
        case ark::compiler::Opcode::LenArray:
            return ABCKIT_ISA_API_STATIC_OPCODE_LENARRAY;
        case ark::compiler::Opcode::If:
            return ABCKIT_ISA_API_STATIC_OPCODE_IF;
        case ark::compiler::Opcode::NullPtr:
            return ABCKIT_ISA_API_STATIC_OPCODE_NULLPTR;
        case ark::compiler::Opcode::Phi:
            return ABCKIT_ISA_API_STATIC_OPCODE_PHI;
        case ark::compiler::Opcode::LoadUniqueObject:
            return ABCKIT_ISA_API_STATIC_OPCODE_LOADNULLVALUE;
        case ark::compiler::Opcode::Try:
            return ABCKIT_ISA_API_STATIC_OPCODE_TRY;
        case ark::compiler::Opcode::CatchPhi:
            return ABCKIT_ISA_API_STATIC_OPCODE_CATCHPHI;
        case ark::compiler::Opcode::Intrinsic:
            return GetStaticIntrinsicOpcode(inst->CastToIntrinsic());
        default:
            LIBABCKIT_LOG(DEBUG) << "compiler->abckit INVALID\n";
            return ABCKIT_ISA_API_STATIC_OPCODE_INVALID;
    }
    LIBABCKIT_UNREACHABLE;
}

AbckitIsaApiDynamicOpcode GetDynamicOpcode(ark::compiler::Inst *inst)
{
    auto opcode = inst->GetOpcode();
    switch (opcode) {
        case ark::compiler::Opcode::LoadString:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING;
        case ark::compiler::Opcode::Parameter:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER;
        case ark::compiler::Opcode::Constant:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT;
        case ark::compiler::Opcode::If:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_IF;
        case ark::compiler::Opcode::Phi:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_PHI;
        case ark::compiler::Opcode::Try:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_TRY;
        case ark::compiler::Opcode::CatchPhi:
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_CATCHPHI;
        case ark::compiler::Opcode::Intrinsic:
            switch (inst->CastToIntrinsic()->GetIntrinsicId()) {
                case ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_STRING:
                    return ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING;
                default:
                    break;
            }
            return GetDynamicIntrinsicOpcode(inst->CastToIntrinsic());
        default:
            LIBABCKIT_LOG(DEBUG) << "compiler->abckit INVALID\n";
            return ABCKIT_ISA_API_DYNAMIC_OPCODE_INVALID;
    }
    LIBABCKIT_UNREACHABLE;
}

size_t GetIntrinicMaxInputsCount(AbckitInst *inst)
{
    ASSERT(inst->impl->IsIntrinsic());
    if (IsDynamic(inst->graph->function->owningModule->target)) {
        return GetIntrinicMaxInputsCountDyn(inst->impl->CastToIntrinsic());
    }
    return GetIntrinicMaxInputsCountStatic(inst->impl->CastToIntrinsic());
}

bool IsCallInst(AbckitInst *inst)
{
    if (IsDynamic(inst->graph->function->owningModule->target)) {
        return IsCallInstDyn(inst->impl);
    }
    return IsCallInstStatic(inst->impl);
}

AbckitTypeId TypeToTypeId(ark::compiler::DataType::Type type)
{
    LIBABCKIT_LOG(DEBUG) << "compiler->abckit\n";
    switch (type) {
        case ark::compiler::DataType::Type::REFERENCE:
            return AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE;
        case ark::compiler::DataType::Type::BOOL:
            return AbckitTypeId::ABCKIT_TYPE_ID_U1;
        case ark::compiler::DataType::Type::UINT8:
            return AbckitTypeId::ABCKIT_TYPE_ID_U8;
        case ark::compiler::DataType::Type::INT8:
            return AbckitTypeId::ABCKIT_TYPE_ID_I8;
        case ark::compiler::DataType::Type::UINT16:
            return AbckitTypeId::ABCKIT_TYPE_ID_U16;
        case ark::compiler::DataType::Type::INT16:
            return AbckitTypeId::ABCKIT_TYPE_ID_I16;
        case ark::compiler::DataType::Type::UINT32:
            return AbckitTypeId::ABCKIT_TYPE_ID_U32;
        case ark::compiler::DataType::Type::INT32:
            return AbckitTypeId::ABCKIT_TYPE_ID_I32;
        case ark::compiler::DataType::Type::UINT64:
            return AbckitTypeId::ABCKIT_TYPE_ID_U64;
        case ark::compiler::DataType::Type::INT64:
            return AbckitTypeId::ABCKIT_TYPE_ID_I64;
        case ark::compiler::DataType::Type::FLOAT32:
            return AbckitTypeId::ABCKIT_TYPE_ID_F32;
        case ark::compiler::DataType::Type::FLOAT64:
            return AbckitTypeId::ABCKIT_TYPE_ID_F64;
        case ark::compiler::DataType::Type::ANY:
            return AbckitTypeId::ABCKIT_TYPE_ID_ANY;
        case ark::compiler::DataType::Type::VOID:
            return AbckitTypeId::ABCKIT_TYPE_ID_VOID;
        case ark::compiler::DataType::Type::POINTER:
        case ark::compiler::DataType::Type::NO_TYPE:
        default:
            LIBABCKIT_LOG(DEBUG) << "compiler->abckit INVALID\n";
            return AbckitTypeId::ABCKIT_TYPE_ID_INVALID;
    }
    LIBABCKIT_UNREACHABLE;
}

ark::compiler::DataType::Type TypeIdToType(AbckitTypeId typeId)
{
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler\n";
    switch (typeId) {
        case AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE:
            return ark::compiler::DataType::Type::REFERENCE;
        case AbckitTypeId::ABCKIT_TYPE_ID_U1:
            return ark::compiler::DataType::Type::BOOL;
        case AbckitTypeId::ABCKIT_TYPE_ID_U8:
            return ark::compiler::DataType::Type::UINT8;
        case AbckitTypeId::ABCKIT_TYPE_ID_I8:
            return ark::compiler::DataType::Type::INT8;
        case AbckitTypeId::ABCKIT_TYPE_ID_U16:
            return ark::compiler::DataType::Type::UINT16;
        case AbckitTypeId::ABCKIT_TYPE_ID_I16:
            return ark::compiler::DataType::Type::INT16;
        case AbckitTypeId::ABCKIT_TYPE_ID_U32:
            return ark::compiler::DataType::Type::UINT32;
        case AbckitTypeId::ABCKIT_TYPE_ID_I32:
            return ark::compiler::DataType::Type::INT32;
        case AbckitTypeId::ABCKIT_TYPE_ID_U64:
            return ark::compiler::DataType::Type::UINT64;
        case AbckitTypeId::ABCKIT_TYPE_ID_I64:
            return ark::compiler::DataType::Type::INT64;
        case AbckitTypeId::ABCKIT_TYPE_ID_F32:
            return ark::compiler::DataType::Type::FLOAT32;
        case AbckitTypeId::ABCKIT_TYPE_ID_F64:
            return ark::compiler::DataType::Type::FLOAT64;
        case AbckitTypeId::ABCKIT_TYPE_ID_ANY:
            return ark::compiler::DataType::Type::ANY;
        case AbckitTypeId::ABCKIT_TYPE_ID_VOID:
            return ark::compiler::DataType::Type::VOID;
        case AbckitTypeId::ABCKIT_TYPE_ID_INVALID:
        default:
            LIBABCKIT_LOG(DEBUG) << "abckit->compiler INVALID\n";
            return ark::compiler::DataType::Type::NO_TYPE;
    }
    LIBABCKIT_UNREACHABLE;
}

ark::compiler::ConditionCode CcToCc(AbckitIsaApiDynamicConditionCode cc)
{
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler\n";
    switch (cc) {
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ:
            return ark::compiler::ConditionCode::CC_EQ;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE:
            return ark::compiler::ConditionCode::CC_NE;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LT:
            return ark::compiler::ConditionCode::CC_LT;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LE:
            return ark::compiler::ConditionCode::CC_LE;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GT:
            return ark::compiler::ConditionCode::CC_GT;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GE:
            return ark::compiler::ConditionCode::CC_GE;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_B:
            return ark::compiler::ConditionCode::CC_B;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_BE:
            return ark::compiler::ConditionCode::CC_BE;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_A:
            return ark::compiler::ConditionCode::CC_A;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_AE:
            return ark::compiler::ConditionCode::CC_AE;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_EQ:
            return ark::compiler::ConditionCode::CC_TST_EQ;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_NE:
            return ark::compiler::ConditionCode::CC_TST_NE;
        case AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE:
            break;
    }
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler CcToCc INVALID\n";
    LIBABCKIT_UNREACHABLE;
}

ark::compiler::ConditionCode CcToCc(AbckitIsaApiStaticConditionCode cc)
{
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler\n";
    switch (cc) {
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ:
            return ark::compiler::ConditionCode::CC_EQ;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE:
            return ark::compiler::ConditionCode::CC_NE;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LT:
            return ark::compiler::ConditionCode::CC_LT;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LE:
            return ark::compiler::ConditionCode::CC_LE;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GT:
            return ark::compiler::ConditionCode::CC_GT;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GE:
            return ark::compiler::ConditionCode::CC_GE;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_B:
            return ark::compiler::ConditionCode::CC_B;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_BE:
            return ark::compiler::ConditionCode::CC_BE;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_A:
            return ark::compiler::ConditionCode::CC_A;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_AE:
            return ark::compiler::ConditionCode::CC_AE;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_EQ:
            return ark::compiler::ConditionCode::CC_TST_EQ;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_NE:
            return ark::compiler::ConditionCode::CC_TST_NE;
        case AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE:
            break;
    }
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler CcToCc INVALID\n";
    LIBABCKIT_UNREACHABLE;
}

AbckitIsaApiDynamicConditionCode CcToDynamicCc(ark::compiler::ConditionCode cc)
{
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler\n";
    switch (cc) {
        case ark::compiler::ConditionCode::CC_EQ:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ;
        case ark::compiler::ConditionCode::CC_NE:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE;
        case ark::compiler::ConditionCode::CC_LT:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LT;
        case ark::compiler::ConditionCode::CC_LE:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LE;
        case ark::compiler::ConditionCode::CC_GT:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GT;
        case ark::compiler::ConditionCode::CC_GE:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GE;
        case ark::compiler::ConditionCode::CC_B:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_B;
        case ark::compiler::ConditionCode::CC_BE:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_BE;
        case ark::compiler::ConditionCode::CC_A:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_A;
        case ark::compiler::ConditionCode::CC_AE:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_AE;
        case ark::compiler::ConditionCode::CC_TST_EQ:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_EQ;
        case ark::compiler::ConditionCode::CC_TST_NE:
            return AbckitIsaApiDynamicConditionCode::ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_NE;
        default:
            break;
    }
    LIBABCKIT_LOG(DEBUG) << "compiler->abckit CcToDynamicCc INVALID\n";
    LIBABCKIT_UNREACHABLE;
}

AbckitIsaApiStaticConditionCode CcToStaticCc(ark::compiler::ConditionCode cc)
{
    LIBABCKIT_LOG(DEBUG) << "abckit->compiler\n";
    switch (cc) {
        case ark::compiler::ConditionCode::CC_EQ:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ;
        case ark::compiler::ConditionCode::CC_NE:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE;
        case ark::compiler::ConditionCode::CC_LT:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LT;
        case ark::compiler::ConditionCode::CC_LE:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LE;
        case ark::compiler::ConditionCode::CC_GT:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GT;
        case ark::compiler::ConditionCode::CC_GE:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GE;
        case ark::compiler::ConditionCode::CC_B:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_B;
        case ark::compiler::ConditionCode::CC_BE:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_BE;
        case ark::compiler::ConditionCode::CC_A:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_A;
        case ark::compiler::ConditionCode::CC_AE:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_AE;
        case ark::compiler::ConditionCode::CC_TST_EQ:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_EQ;
        case ark::compiler::ConditionCode::CC_TST_NE:
            return AbckitIsaApiStaticConditionCode::ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_NE;
        default:
            break;
    }
    LIBABCKIT_LOG(DEBUG) << "compiler->abckit CcToStaticCc INVALID\n";
    LIBABCKIT_UNREACHABLE;
}

void SetLastError(AbckitStatus err)
{
    LIBABCKIT_LOG(DEBUG) << "error code: " << err << std::endl;
    statuses::SetLastError(err);
}

uint32_t GetClassOffset(AbckitGraph *graph, AbckitCoreClass *klass)
{
    ASSERT(!IsDynamic(graph->function->owningModule->target));

    auto *rec = klass->GetArkTSImpl()->impl.GetStaticClass();
    LIBABCKIT_LOG(DEBUG) << "className: " << rec->name << "\n";

    uint32_t classOffset = 0;
    for (auto &[id, s] : graph->irInterface->classes) {
        if (s == rec->name) {
            classOffset = id;
        }
    }
    if (classOffset == 0) {
        LIBABCKIT_LOG(DEBUG) << "classOffset == 0\n";
        LIBABCKIT_UNREACHABLE;
    }
    LIBABCKIT_LOG(DEBUG) << "classOffset: " << classOffset << "\n";
    return classOffset;
}

std::string GetFuncName(AbckitCoreFunction *function)
{
    std::string funcName = "__ABCKIT_INVALID__";

    if (IsDynamic(function->owningModule->target)) {
        auto *func = PandasmWrapper::GetWrappedFunction(GetDynFunction(function));
        funcName = func->name;
        delete func;
    } else {
        auto *func = reinterpret_cast<const ark::pandasm::Function *>(function->GetArkTSImpl()->GetStaticImpl());
        funcName = func->name;
    }

    return funcName;
}

std::string GetMangleFuncName(AbckitCoreFunction *function)
{
    std::string funcName = "__ABCKIT_INVALID__";

    if (IsDynamic(function->owningModule->target)) {
        auto *func = PandasmWrapper::GetWrappedFunction(GetDynFunction(function));
        funcName = func->name;
        delete func;
    } else {
        auto *func = reinterpret_cast<const ark::pandasm::Function *>(function->GetArkTSImpl()->GetStaticImpl());
        funcName = MangleFunctionName(ark::pandasm::DeMangleName(func->name), func->params, func->returnType);
    }
    return funcName;
}

uint32_t GetMethodOffset(AbckitGraph *graph, AbckitCoreFunction *function)
{
    std::string funcName = GetMangleFuncName(function);
    ASSERT(funcName != "__ABCKIT_INVALID__");
    LIBABCKIT_LOG(DEBUG) << "functionName: " << funcName << "\n";

    uint32_t methodOffset = 0;
    for (auto &[id, s] : graph->irInterface->methods) {
        if (s == funcName) {
            methodOffset = id;
            break;
        }
    }
    if (methodOffset == 0) {
        LIBABCKIT_LOG(DEBUG) << "methodOffset == 0\n";
        LIBABCKIT_UNREACHABLE;
    }
    LIBABCKIT_LOG(DEBUG) << "methodOffset: " << methodOffset << "\n";
    return methodOffset;
}

uint32_t GetStringOffset(AbckitGraph *graph, AbckitString *string)
{
    uint32_t stringOffset = 0;
    for (auto &[id, s] : graph->irInterface->strings) {
        if (s == string->impl) {
            stringOffset = id;
        }
    }

    if (stringOffset == 0) {
        // Newly created string
        // NOLINTNEXTLINE(cert-msc51-cpp)
        do {
            LIBABCKIT_LOG(DEBUG) << "generating new stringOffset\n";
            // NOLINTNEXTLINE(cert-msc50-cpp)
            stringOffset++;
        } while (graph->irInterface->strings.find(stringOffset) != graph->irInterface->strings.end());
        // insert new string id
        graph->irInterface->strings.insert({stringOffset, string->impl.data()});
    }

    LIBABCKIT_LOG(DEBUG) << "stringOffset: " << stringOffset << "\n";
    return stringOffset;
}

uint32_t GetFieldOffset(AbckitGraph *graph, AbckitString *string)
{
    uint32_t fieldOffset = 0;
    for (auto &[id, s] : graph->irInterface->fields) {
        if (s == string->impl) {
            fieldOffset = id;
        }
    }

    if (fieldOffset == 0) {
        // Newly created string
        // NOLINTNEXTLINE(cert-msc51-cpp)
        do {
            LIBABCKIT_LOG(DEBUG) << "generating new fieldOffset\n";
            // NOLINTNEXTLINE(cert-msc50-cpp)
            fieldOffset++;
        } while (graph->irInterface->fields.find(fieldOffset) != graph->irInterface->fields.end());
        // insert new string id
        graph->irInterface->fields.insert({fieldOffset, string->impl.data()});
    }

    LIBABCKIT_LOG(DEBUG) << "fieldOffset: " << fieldOffset << "\n";
    return fieldOffset;
}

uint32_t GetLiteralArrayOffset(AbckitGraph *graph, AbckitLiteralArray *arr)
{
    std::string arrName = "__ABCKIT_INVALID__";
    if (IsDynamic(graph->function->owningModule->target)) {
        auto litarrTable = PandasmWrapper::GetUnwrappedLiteralArrayTable(graph->file->GetDynamicProgram());
        for (auto &[id, s] : litarrTable) {
            if (s == arr->GetDynamicImpl()) {
                arrName = id;
            }
        }
    } else {
        auto *prog = graph->file->GetStaticProgram();
        for (auto &[id, s] : prog->literalarrayTable) {
            if (&s == arr->GetStaticImpl()) {
                arrName = id;
            }
        }
    }
    ASSERT(arrName != "__ABCKIT_INVALID__");

    uint32_t arrayOffset = 0;
    for (auto &[id, s] : graph->irInterface->literalarrays) {
        if (s == arrName) {
            arrayOffset = id;
        }
    }

    if (arrayOffset == 0) {
        // Newly created literal array
        // NOLINTNEXTLINE(cert-msc51-cpp)
        do {
            LIBABCKIT_LOG(DEBUG) << "generating new arrayOffset\n";
            // NOLINTNEXTLINE(cert-msc50-cpp)
            arrayOffset++;
        } while (graph->irInterface->literalarrays.find(arrayOffset) != graph->irInterface->literalarrays.end());
        // insert new literal array
        graph->irInterface->literalarrays.insert({arrayOffset, arrName});
    }
    return arrayOffset;
}

AbckitInst *FindOrCreateInstFromImpl(AbckitGraph *graph, ark::compiler::Inst *impl)
{
    if (graph->implToInst.find(impl) != graph->implToInst.end()) {
        return graph->implToInst.at(impl);
    }

    return CreateInstFromImpl(graph, impl);
}

AbckitInst *CreateInstFromImpl(AbckitGraph *graph, ark::compiler::Inst *impl)
{
    AbckitInst *newInst = graph->impl->GetLocalAllocator()->New<AbckitInst>(graph, impl);
    graph->implToInst.insert({impl, newInst});
    return newInst;
}

AbckitInst *CreateDynInst(AbckitGraph *graph, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId,
                          bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, ark::compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {2U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, ark::compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, AbckitInst *input2,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {3U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input2->impl, ark::compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {2U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, ark::compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, ark::compiler::DataType::Type retType,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(retType, 0, intrinsicId);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, AbckitInst *inputSize,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::REFERENCE, 0, intrinsicId);

    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(inputSize->impl, ark::compiler::DataType::INT32);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId,
                          bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, ark::compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0, uint64_t imm1,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, ark::compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    intrImpl->AddImm(graph->impl->GetAllocator(), imm1);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, uint64_t imm1,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    intrImpl->AddImm(graph->impl->GetAllocator(), imm1);
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0, uint64_t imm1,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {2U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, ark::compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    intrImpl->AddImm(graph->impl->GetAllocator(), imm1);
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01-CPP) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                          AbckitInst *input2, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {4U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(acc->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input0->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input2->impl, ark::compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, uint64_t imm1, uint64_t imm2, AbckitInst *input0,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, ark::compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    intrImpl->AddImm(graph->impl->GetAllocator(), imm1);
    intrImpl->AddImm(graph->impl->GetAllocator(), imm2);
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                          AbckitInst *input2, AbckitInst *input3, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId,
                          bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {5U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(acc->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input0->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input2->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input3->impl, ark::compiler::DataType::ANY);
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0, std::va_list args,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {imm0 + 2U};
    intrImpl->ReserveInputs(imm0 + 2U);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, ark::compiler::DataType::ANY);
    intrImpl->AppendInput(input1->impl, ark::compiler::DataType::ANY);
    for (size_t index = 0; index < imm0; index++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        AbckitInst *input = va_arg(args, AbckitInst *);
        intrImpl->AppendInput(input->impl, ark::compiler::DataType::ANY);
    }
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), imm0);
    return CreateInstFromImpl(graph, intrImpl);
}

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {argCount + 1};
    intrImpl->ReserveInputs(argCount + 1);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(acc->impl, ark::compiler::DataType::ANY);
    for (size_t index = 0; index < argCount; ++index) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        AbckitInst *input = va_arg(args, AbckitInst *);
        intrImpl->AppendInput(input->impl, ark::compiler::DataType::ANY);
    }
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), argCount);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *CreateDynInst(AbckitGraph *graph, size_t argCount, std::va_list args,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC)
{
    auto intrImpl = graph->impl->CreateInstIntrinsic(ark::compiler::DataType::ANY, 0, intrinsicId);
    size_t argsCount {argCount};
    intrImpl->ReserveInputs(argCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    for (size_t index = 0; index < argCount; ++index) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        AbckitInst *input = va_arg(args, AbckitInst *);
        intrImpl->AppendInput(input->impl, ark::compiler::DataType::ANY);
    }
    if (hasIC) {
        intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), argCount);
    return CreateInstFromImpl(graph, intrImpl);
}

template <class T>
static AbckitLiteral *FindOrCreateLiteral(AbckitFile *file,
                                          std::unordered_map<T, std::unique_ptr<AbckitLiteral>> &cache, T value,
                                          ark::panda_file::LiteralTag tagImpl)
{
    if (cache.count(value) == 1) {
        return cache[value].get();
    }

    auto literalDeleter = [](pandasm_Literal *p) -> void {
        delete reinterpret_cast<ark::pandasm::LiteralArray::Literal *>(p);
    };

    auto *literal = new ark::pandasm::LiteralArray::Literal();
    literal->tag = tagImpl;
    literal->value = value;
    auto abcLit = std::make_unique<AbckitLiteral>(
        file, AbckitLiteralImplT(reinterpret_cast<pandasm_Literal *>(literal), literalDeleter));
    cache.insert({value, std::move(abcLit)});
    return cache[value].get();
}

AbckitLiteral *FindOrCreateLiteralBoolStaticImpl(AbckitFile *file, bool value)
{
    return FindOrCreateLiteral(file, file->literals.boolLits, value, ark::panda_file::LiteralTag::ARRAY_U1);
}

AbckitLiteral *FindOrCreateLiteralU8StaticImpl(AbckitFile *file, uint8_t value)
{
    return FindOrCreateLiteral(file, file->literals.u8Lits, value, ark::panda_file::LiteralTag::ARRAY_U8);
}

AbckitLiteral *FindOrCreateLiteralU16StaticImpl(AbckitFile *file, uint16_t value)
{
    return FindOrCreateLiteral(file, file->literals.u16Lits, value, ark::panda_file::LiteralTag::ARRAY_U16);
}

AbckitLiteral *FindOrCreateLiteralMethodAffiliateStaticImpl(AbckitFile *file, uint16_t value)
{
    return FindOrCreateLiteral(file, file->literals.methodAffilateLits, value,
                               ark::panda_file::LiteralTag::METHODAFFILIATE);
}

AbckitLiteral *FindOrCreateLiteralU32StaticImpl(AbckitFile *file, uint32_t value)
{
    return FindOrCreateLiteral(file, file->literals.u32Lits, value, ark::panda_file::LiteralTag::ARRAY_U32);
}

AbckitLiteral *FindOrCreateLiteralU64StaticImpl(AbckitFile *file, uint64_t value)
{
    return FindOrCreateLiteral(file, file->literals.u64Lits, value, ark::panda_file::LiteralTag::ARRAY_U64);
}

AbckitLiteral *FindOrCreateLiteralFloatStaticImpl(AbckitFile *file, float value)
{
    return FindOrCreateLiteral(file, file->literals.floatLits, value, ark::panda_file::LiteralTag::FLOAT);
}

AbckitLiteral *FindOrCreateLiteralDoubleStaticImpl(AbckitFile *file, double value)
{
    return FindOrCreateLiteral(file, file->literals.doubleLits, value, ark::panda_file::LiteralTag::DOUBLE);
}

AbckitLiteral *FindOrCreateLiteralStringStaticImpl(AbckitFile *file, const std::string &value)
{
    return FindOrCreateLiteral(file, file->literals.stringLits, value, ark::panda_file::LiteralTag::STRING);
}

AbckitLiteral *FindOrCreateLiteralMethodStaticImpl(AbckitFile *file, const std::string &value)
{
    return FindOrCreateLiteral(file, file->literals.methodLits, value, ark::panda_file::LiteralTag::METHOD);
}

template <class T, ark::pandasm::Value::Type PANDASM_VALUE_TYPE>
static AbckitValue *FindOrCreateScalarValue(AbckitFile *file,
                                            std::unordered_map<T, std::unique_ptr<AbckitValue>> &cache, T value)
{
    if (cache.count(value) == 1) {
        return cache[value].get();
    }

    auto valueDeleter = [](pandasm_Value *val) -> void { delete reinterpret_cast<ark::pandasm::ScalarValue *>(val); };

    auto *pval = new ark::pandasm::ScalarValue(ark::pandasm::ScalarValue::Create<PANDASM_VALUE_TYPE>(value));
    auto abcVal =
        std::make_unique<AbckitValue>(file, AbckitValueImplT(reinterpret_cast<pandasm_Value *>(pval), valueDeleter));
    cache.insert({value, std::move(abcVal)});
    return cache[value].get();
}

AbckitValue *FindOrCreateValueU1StaticImpl(AbckitFile *file, bool value)
{
    return FindOrCreateScalarValue<bool, ark::pandasm::Value::Type::U1>(file, file->values.boolVals, value);
}

AbckitValue *FindOrCreateValueDoubleStaticImpl(AbckitFile *file, double value)
{
    return FindOrCreateScalarValue<double, ark::pandasm::Value::Type::F64>(file, file->values.doubleVals, value);
}

AbckitValue *FindOrCreateValueIntStaticImpl(AbckitFile *file, int value)
{
    return FindOrCreateScalarValue<int, ark::pandasm::Value::Type::I32>(file, file->values.intVals, value);
}

AbckitValue *FindOrCreateValueLongStaticImpl(AbckitFile *file, int64_t value)
{
    return FindOrCreateScalarValue<int64_t, ark::pandasm::Value::Type::I64>(file, file->values.longVals, value);
}

AbckitValue *FindOrCreateValueStringNullptrStaticImpl(AbckitFile *file, int32_t value)
{
    return FindOrCreateScalarValue<int32_t, ark::pandasm::Value::Type::STRING_NULLPTR>(
        file, file->values.stringNullptrVals, value);
}

AbckitValue *FindOrCreateValueStringStaticImpl(AbckitFile *file, const std::string &value)
{
    return FindOrCreateScalarValue<std::string, ark::pandasm::Value::Type::STRING>(file, file->values.stringVals,
                                                                                   value);
}

AbckitValue *FindOrCreateLiteralArrayValueStaticImpl(AbckitFile *file, const std::string &value)
{
    return FindOrCreateScalarValue<std::string, ark::pandasm::Value::Type::LITERALARRAY>(file, file->values.litarrVals,
                                                                                         value);
}

AbckitValue *FindOrCreateRecordValueStaticImpl(AbckitFile *file, const ark::pandasm::Type &value)
{
    std::string key = value.GetName();
    if (file->values.recordVals.count(key) == 1) {
        return file->values.recordVals[key].get();
    }

    auto valueDeleter = [](pandasm_Value *val) -> void { delete reinterpret_cast<ark::pandasm::ScalarValue *>(val); };

    auto *pval =
        new ark::pandasm::ScalarValue(ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::RECORD>(value));
    auto abcVal =
        std::make_unique<AbckitValue>(file, AbckitValueImplT(reinterpret_cast<pandasm_Value *>(pval), valueDeleter));
    file->values.recordVals.insert({key, std::move(abcVal)});
    return file->values.recordVals[key].get();
}

AbckitValue *FindOrCreateValueMethodStaticImpl(AbckitFile *file, const std::string &value, bool isStatic)
{
    std::string key = value;
    if (file->values.methodVals.count(key) == 1) {
        return file->values.methodVals[key].get();
    }

    auto valueDeleter = [](pandasm_Value *val) -> void { delete reinterpret_cast<ark::pandasm::ScalarValue *>(val); };

    // Create ScalarValue with std::string and correct isStatic flag
    auto *pval = new ark::pandasm::ScalarValue(
        ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::METHOD>(value, isStatic));

    auto abcVal =
        std::make_unique<AbckitValue>(file, AbckitValueImplT(reinterpret_cast<pandasm_Value *>(pval), valueDeleter));
    file->values.methodVals.insert({key, std::move(abcVal)});
    return file->values.methodVals[key].get();
}

AbckitValue *FindOrCreateValueEnumStaticImpl(AbckitFile *file, const std::string &value)
{
    return FindOrCreateScalarValue<std::string, ark::pandasm::Value::Type::ENUM>(file, file->values.enumVals, value);
}

AbckitValue *FindOrCreateAnnotationValueStaticImpl(AbckitFile *file, const ark::pandasm::AnnotationData &value)
{
    const std::string key = value.GetName();
    if (file->values.annotationVals.find(key) != file->values.annotationVals.end()) {
        return file->values.annotationVals[key].get();
    }

    auto valueDeleter = [](pandasm_Value *val) -> void { delete reinterpret_cast<ark::pandasm::ScalarValue *>(val); };
    auto *pval =
        new ark::pandasm::ScalarValue(ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::ANNOTATION>(value));
    auto abcVal =
        std::make_unique<AbckitValue>(file, AbckitValueImplT(reinterpret_cast<pandasm_Value *>(pval), valueDeleter));
    file->values.annotationVals.insert({key, std::move(abcVal)});
    return file->values.annotationVals[key].get();
}

AbckitValue *FindOrCreateArrayValueStaticImpl(AbckitFile *file, const ark::pandasm::ArrayValue &value)
{
    // Generate a unique key based on component type and all values
    std::string key = std::to_string(static_cast<int>(value.GetComponentType())) + ":";
    const auto &values = value.GetValues();

    for (size_t i = 0; i < values.size(); ++i) {
        key += (i > 0 ? "," : "");

        const auto &val = values[i];
        auto *scalarValue = val.GetAsScalar();
        auto valType = val.GetType();

        if (valType == ark::pandasm::Value::Type::METHOD && scalarValue != nullptr) {
            key += scalarValue->GetValue<std::string>();
            key += (scalarValue->IsStatic() ? ":static" : "");
            continue;
        }

        if ((valType == ark::pandasm::Value::Type::STRING || valType == ark::pandasm::Value::Type::ENUM) &&
            scalarValue != nullptr) {
            key += scalarValue->GetValue<std::string>();
            continue;
        }

        // For other types, use a placeholder
        key += "?";
    }

    if (file->values.arrayVals.find(key) != file->values.arrayVals.end()) {
        return file->values.arrayVals[key].get();
    }

    auto valueDeleter = [](pandasm_Value *val) -> void { delete reinterpret_cast<ark::pandasm::ArrayValue *>(val); };
    // Create a copy of the ArrayValue
    auto *pval = new ark::pandasm::ArrayValue(value.GetComponentType(), value.GetValues());
    auto abcVal =
        std::make_unique<AbckitValue>(file, AbckitValueImplT(reinterpret_cast<pandasm_Value *>(pval), valueDeleter));
    file->values.arrayVals.insert({key, std::move(abcVal)});
    return file->values.arrayVals[key].get();
}

AbckitValue *FindOrCreateValueStatic(AbckitFile *file, const ark::pandasm::Value &value)
{
    switch (value.GetType()) {
        case ark::pandasm::Value::Type::U1:
            return FindOrCreateValueU1StaticImpl(file, value.GetAsScalar()->GetValue<bool>());
        case ark::pandasm::Value::Type::I8:
        case ark::pandasm::Value::Type::U8:
        case ark::pandasm::Value::Type::I16:
        case ark::pandasm::Value::Type::U16:
        case ark::pandasm::Value::Type::I32:
        case ark::pandasm::Value::Type::U32:
            return FindOrCreateValueIntStaticImpl(file, value.GetAsScalar()->GetValue<int32_t>());
        case ark::pandasm::Value::Type::I64:
        case ark::pandasm::Value::Type::U64:
            return FindOrCreateValueLongStaticImpl(file, value.GetAsScalar()->GetValue<int64_t>());
        case ark::pandasm::Value::Type::F32:
            return FindOrCreateValueDoubleStaticImpl(file, static_cast<double>(value.GetAsScalar()->GetValue<float>()));
        case ark::pandasm::Value::Type::F64:
            return FindOrCreateValueDoubleStaticImpl(file, value.GetAsScalar()->GetValue<double>());
        case ark::pandasm::Value::Type::STRING:
            return FindOrCreateValueStringStaticImpl(file, value.GetAsScalar()->GetValue<std::string>());
        case ark::pandasm::Value::Type::METHOD: {
            auto *scalarValue = value.GetAsScalar();
            bool isStatic = scalarValue->IsStatic();
            return FindOrCreateValueMethodStaticImpl(file, scalarValue->GetValue<std::string>(), isStatic);
        }
        case ark::pandasm::Value::Type::ENUM:
            return FindOrCreateValueEnumStaticImpl(file, value.GetAsScalar()->GetValue<std::string>());
        case ark::pandasm::Value::Type::STRING_NULLPTR:
            return FindOrCreateValueStringNullptrStaticImpl(file, value.GetAsScalar()->GetValue<int32_t>());
        case ark::pandasm::Value::Type::LITERALARRAY:
            return FindOrCreateLiteralArrayValueStaticImpl(file, value.GetAsScalar()->GetValue<std::string>());
        case ark::pandasm::Value::Type::RECORD:
            return FindOrCreateRecordValueStaticImpl(file, value.GetAsScalar()->GetValue<ark::pandasm::Type>());
        case ark::pandasm::Value::Type::ANNOTATION:
            return FindOrCreateAnnotationValueStaticImpl(file,
                                                         value.GetAsScalar()->GetValue<ark::pandasm::AnnotationData>());
        case ark::pandasm::Value::Type::ARRAY: {
            // Value is a reference, need to get pointer for casting
            auto *arrayValue = const_cast<ark::pandasm::Value *>(&value);
            if (arrayValue == nullptr || arrayValue->GetType() != ark::pandasm::Value::Type::ARRAY) {
                LIBABCKIT_LOG(ERROR) << "Failed to get ArrayValue from Value" << std::endl;
                LIBABCKIT_UNREACHABLE;
            }
            auto *castedArrayValue = static_cast<ark::pandasm::ArrayValue *>(arrayValue);
            return FindOrCreateArrayValueStaticImpl(file, *castedArrayValue);
        }
        case ark::pandasm::Value::Type::VOID:
        case ark::pandasm::Value::Type::METHOD_HANDLE:
        case ark::pandasm::Value::Type::UNKNOWN:
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

void FixPreassignedRegisters(ark::compiler::Inst *inst, ark::compiler::Register reg, ark::compiler::Register regLarge)
{
    for (size_t idx = 0; idx < inst->GetInputsCount(); idx++) {
        if (inst->GetSrcReg(idx) == reg) {
            inst->SetSrcReg(idx, regLarge);
        }
    }
    if (inst->GetDstReg() == reg) {
        inst->SetDstReg(regLarge);
    }
}

void FixPreassignedRegisters(ark::compiler::Graph *graph)
{
    for (auto bb : graph->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            FixPreassignedRegisters(inst, ark::compiler::INVALID_REG, ark::compiler::INVALID_REG_LARGE);
            FixPreassignedRegisters(inst, ark::compiler::INVALID_REG - 1U,
                                    ark::compiler::INVALID_REG_LARGE - 1U);  // ACC_REG
        }
    }
}

bool AllocateRegisters(ark::compiler::Graph *graph, uint8_t reservedReg)
{
    graph->RunPass<ark::bytecodeopt::RegAccAlloc>();
    ark::compiler::RegAllocResolver(graph).ResolveCatchPhis();
    if (!ark::compiler::IsFrameSizeLarge()) {
        return graph->RunPass<ark::compiler::RegAllocGraphColoring>(ark::compiler::GetFrameSize());
    }
    ark::compiler::LocationMask regMask(graph->GetLocalAllocator());
    regMask.Resize(ark::compiler::GetFrameSize());
    regMask.Set(reservedReg);
    FixPreassignedRegisters(graph);
    return graph->RunPass<ark::compiler::RegAllocGraphColoring>(regMask);
}

void GraphInvalidateAnalyses(ark::compiler::Graph *graph)
{
    graph->InvalidateAnalysis<ark::compiler::LoopAnalyzer>();
    graph->InvalidateAnalysis<ark::compiler::Rpo>();
    graph->InvalidateAnalysis<ark::compiler::DominatorsTree>();
    graph->InvalidateAnalysis<ark::compiler::LinearOrder>();
}

static void GraphMarkBlocksRec(ark::compiler::Graph *graph, ark::compiler::Marker mrk, ark::compiler::BasicBlock *block)
{
    if (block->SetMarker(mrk)) {
        return;
    }
    for (auto succ : block->GetSuccsBlocks()) {
        GraphMarkBlocksRec(graph, mrk, succ);
    }
}

bool GraphHasUnreachableBlocks(ark::compiler::Graph *graph)
{
    bool ret = false;
    ark::compiler::Marker mrk = graph->NewMarker();
    GraphMarkBlocksRec(graph, mrk, graph->GetStartBlock());
    auto bbs = graph->GetVectorBlocks();
    // Remove unreachable blocks
    for (auto &bb : bbs) {
        if (bb != nullptr && !bb->IsMarked(mrk)) {
            LIBABCKIT_LOG(DEBUG) << "BB " << bb->GetId() << " is unreachable\n";
            ret = true;
            break;
        }
    }
    graph->EraseMarker(mrk);
    return ret;
}

bool GraphDominatorsTreeAnalysisIsValid(ark::compiler::Graph *graph)
{
    if (!graph->GetAnalysis<ark::compiler::DominatorsTree>().IsValid()) {
        if (!graph->RunPass<ark::compiler::DominatorsTree>()) {
            LIBABCKIT_LOG(DEBUG) << ": ICDominatorsTree failed!\n";
            return false;
        }
    }
    return true;
}

std::string TypeToNameStatic(AbckitType *type)
{
    if (type->id == ABCKIT_TYPE_ID_REFERENCE) {
        if (!type->types.empty()) {
            return libabckit::StringUtil::GetTypeNameStr(type);
        }
        if (type->name != nullptr) {
            return type->name->impl.data();
        }
        if (type->GetClass() == nullptr) {
            return "";
        }
        return libabckit::NameUtil::GetFullName(type->GetClass());
    }
    switch (type->id) {
        case ABCKIT_TYPE_ID_VOID:
            return "void";
        case ABCKIT_TYPE_ID_U1:
            return "u1";
        case ABCKIT_TYPE_ID_U8:
            return "u8";
        case ABCKIT_TYPE_ID_I8:
            return "u8";
        case ABCKIT_TYPE_ID_U16:
            return "u16";
        case ABCKIT_TYPE_ID_I16:
            return "i16";
        case ABCKIT_TYPE_ID_U32:
            return "u32";
        case ABCKIT_TYPE_ID_I32:
            return "i32";
        case ABCKIT_TYPE_ID_U64:
            return "u64";
        case ABCKIT_TYPE_ID_I64:
            return "i64";
        case ABCKIT_TYPE_ID_F32:
            return "f32";
        case ABCKIT_TYPE_ID_F64:
            return "f64";
        case ABCKIT_TYPE_ID_ANY:
            return "any";
        case ABCKIT_TYPE_ID_STRING:
            return "std.core.String";
        default:
            return "invalid";
    }
}

AbckitTypeId ArkPandasmTypeToAbckitTypeId(const ark::pandasm::Type &type)
{
    switch (type.GetId()) {
        case ark::panda_file::Type::TypeId::VOID:
            return ABCKIT_TYPE_ID_VOID;
        case ark::panda_file::Type::TypeId::U1:
            return ABCKIT_TYPE_ID_U1;
        case ark::panda_file::Type::TypeId::I8:
            return ABCKIT_TYPE_ID_I8;
        case ark::panda_file::Type::TypeId::U8:
            return ABCKIT_TYPE_ID_U8;
        case ark::panda_file::Type::TypeId::I16:
            return ABCKIT_TYPE_ID_I16;
        case ark::panda_file::Type::TypeId::U16:
            return ABCKIT_TYPE_ID_U16;
        case ark::panda_file::Type::TypeId::F32:
            return ABCKIT_TYPE_ID_F32;
        case ark::panda_file::Type::TypeId::I32:
            return ABCKIT_TYPE_ID_I32;
        case ark::panda_file::Type::TypeId::U32:
            return ABCKIT_TYPE_ID_U32;
        case ark::panda_file::Type::TypeId::F64:
            return ABCKIT_TYPE_ID_F64;
        case ark::panda_file::Type::TypeId::I64:
            return ABCKIT_TYPE_ID_I64;
        case ark::panda_file::Type::TypeId::U64:
            return ABCKIT_TYPE_ID_U64;
        case ark::panda_file::Type::TypeId::REFERENCE:
            if (type.IsArray()) {
                return ArkPandasmTypeToAbckitTypeId(type.GetComponentType());
            } else if (type.GetName() == "std.core.String") {
                return ABCKIT_TYPE_ID_STRING;
            }
            return ABCKIT_TYPE_ID_REFERENCE;
        default:
            return ABCKIT_TYPE_ID_INVALID;
    }
}

ark::pandasm::Record *GetStaticImplRecord(
    const std::variant<AbckitCoreModule *, AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreInterface *,
                       AbckitCoreEnum *, AbckitCoreAnnotationInterface *> &coreObject)
{
    return std::visit(
        [](auto *obj) -> ark::pandasm::Record * {
            if constexpr (std::is_same_v<decltype(obj), AbckitCoreModule *>) {
                return obj->GetArkTSImpl()->impl.GetStatModule().record;
            } else if constexpr (std::is_same_v<decltype(obj), AbckitCoreAnnotationInterface *>) {
                return obj->GetArkTSImpl()->GetStaticImpl();
            } else if constexpr (std::is_same_v<decltype(obj), AbckitCoreNamespace *> ||
                                 std::is_same_v<decltype(obj), AbckitCoreClass *> ||
                                 std::is_same_v<decltype(obj), AbckitCoreInterface *> ||
                                 std::is_same_v<decltype(obj), AbckitCoreEnum *>) {
                return obj->GetArkTSImpl()->impl.GetStaticClass();
            }
            return nullptr;
        },
        coreObject);
}

}  // namespace libabckit
