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

#include "abs_int_inl.h"
#include <algorithm>
#include "handle_gen.h"

namespace ark::verifier {

bool AbsIntInstructionHandler::IsRegDefined(int reg)
{
    bool isDefined = ExecCtx().CurrentRegContext().IsRegDefined(reg);
#ifndef NDEBUG
    if (!isDefined) {
        if (!ExecCtx().CurrentRegContext().WasConflictOnReg(reg)) {
            SHOW_MSG(UndefinedRegister)
            LOG_VERIFIER_UNDEFINED_REGISTER(RegisterName(reg, true));
            END_SHOW_MSG();
        } else {
            SHOW_MSG(RegisterTypeConflict)
            LOG_VERIFIER_REGISTER_TYPE_CONFLICT(RegisterName(reg, false));
            END_SHOW_MSG();
        }
    }
#endif
    return isDefined;
}

bool AbsIntInstructionHandler::CheckRegType(int reg, Type tgtType)
{
    if (!IsRegDefined(reg)) {
        return false;
    }
    auto type = GetRegType(reg);
    if (CheckType(type, tgtType)) {
        return true;
    }

    SHOW_MSG(BadRegisterType)
    LOG_VERIFIER_BAD_REGISTER_TYPE(RegisterName(reg, true), ToString(type), ToString(tgtType));
    END_SHOW_MSG();
    return false;
}

bool AbsIntInstructionHandler::CheckRegTypeNotNullRef(int reg)
{
    if (GetRegType(reg) == nullRefType_) {
        SHOW_MSG(AlwaysNpe)
        LOG_VERIFIER_ALWAYS_NPE(reg);
        END_SHOW_MSG();
        SET_STATUS_FOR_MSG(AlwaysNpe, OK);
        return false;
    }
    return true;
}

bool AbsIntInstructionHandler::CheckRegTypes(std::initializer_list<int> const &regs,
                                             std::initializer_list<Type> const &tgtTypes)
{
    bool result = std::equal(regs.begin(), regs.end(), tgtTypes.begin(),
                             [this](int reg, Type tgtType) { return CheckRegType(reg, tgtType); });

    return result;
}

const AbstractTypedValue &AbsIntInstructionHandler::GetReg(int regIdx)
{
    return context_.ExecCtx().CurrentRegContext()[regIdx];
}

Type AbsIntInstructionHandler::GetRegType(int regIdx)
{
    return GetReg(regIdx).GetAbstractType();
}

void AbsIntInstructionHandler::SetReg(int regIdx, const AbstractTypedValue &val)
{
    if (job_->Options().ShowRegChanges()) {
        PandaString prevAtvImage {"<none>"};
        if (ExecCtx().CurrentRegContext().IsRegDefined(regIdx)) {
            prevAtvImage = ToString(&GetReg(regIdx));
        }
        auto newAtvImage = ToString(&val);
        LOG_VERIFIER_DEBUG_REGISTER_CHANGED(RegisterName(regIdx), prevAtvImage, newAtvImage);
    }
    // RegContext extends flexibly for any number of registers, though on AbsInt instruction
    // handling stage it is not a time to add a new register! The regsiter set (vregs and aregs)
    // is normally initialized on PrepareVerificationContext.
    if (!context_.ExecCtx().CurrentRegContext().IsValid(regIdx)) {
        auto const *method = job_->JobMethod();
        auto numVregs = method->GetNumVregs();
        auto numAregs = GetTypeSystem()->GetMethodSignature(method)->args.size();
        if (regIdx > (int)(numVregs + numAregs)) {
            LOG_VERIFIER_UNDEFINED_REGISTER(RegisterName(regIdx, true));
            SET_STATUS_FOR_MSG(UndefinedRegister, ERROR);
        }
    }
    context_.ExecCtx().CurrentRegContext()[regIdx] = val;
}

void AbsIntInstructionHandler::SetReg(int regIdx, Type type)
{
    SetReg(regIdx, MkVal(type));
}

void AbsIntInstructionHandler::SetRegAndOthersOfSameOrigin(int regIdx, const AbstractTypedValue &val)
{
    context_.ExecCtx().CurrentRegContext().ChangeValuesOfSameOrigin(regIdx, val);
}

void AbsIntInstructionHandler::SetRegAndOthersOfSameOrigin(int regIdx, Type type)
{
    SetRegAndOthersOfSameOrigin(regIdx, MkVal(type));
}

const AbstractTypedValue &AbsIntInstructionHandler::GetAcc()
{
    return context_.ExecCtx().CurrentRegContext()[ACC];
}

Type AbsIntInstructionHandler::GetAccType()
{
    return GetAcc().GetAbstractType();
}

void AbsIntInstructionHandler::SetAcc(const AbstractTypedValue &val)
{
    SetReg(ACC, val);
}

void AbsIntInstructionHandler::SetAcc(Type type)
{
    SetReg(ACC, type);
}

void AbsIntInstructionHandler::SetAccAndOthersOfSameOrigin(const AbstractTypedValue &val)
{
    SetRegAndOthersOfSameOrigin(ACC, val);
}

void AbsIntInstructionHandler::SetAccAndOthersOfSameOrigin(Type type)
{
    SetRegAndOthersOfSameOrigin(ACC, type);
}

AbstractTypedValue AbsIntInstructionHandler::MkVal(Type t)
{
    return AbstractTypedValue {t, context_.NewVar(), GetInst()};
}

Type AbsIntInstructionHandler::TypeOfClass(Class const *klass)
{
    return Type {klass};
}

Type AbsIntInstructionHandler::ReturnType()
{
    return context_.ReturnType();
}

ExecContext &AbsIntInstructionHandler::ExecCtx()
{
    return context_.ExecCtx();
}

void AbsIntInstructionHandler::DumpRegs(const RegContext &ctx)
{
    LOG_VERIFIER_DEBUG_REGISTERS("registers =", ctx.DumpRegs(GetTypeSystem()));
}

void AbsIntInstructionHandler::Sync()
{
    auto addr = inst_.GetAddress();
    ExecContext &execCtx = ExecCtx();
    // NOTE(vdyadov): add verification options to show current context and contexts diff in case of incompatibility
#ifndef NDEBUG
    execCtx.StoreCurrentRegContextForAddr(
        addr, [this, printHdr = true](int regIdx, const auto &src, const auto &dst) mutable {
            if (printHdr) {
                LOG_VERIFIER_REGISTER_CONFLICT_HEADER();
                printHdr = false;
            }
            LOG_VERIFIER_REGISTER_CONFLICT(RegisterName(regIdx), ToString(src.GetAbstractType()),
                                           ToString(dst.GetAbstractType()));
            return true;
        });
#else
    execCtx.StoreCurrentRegContextForAddr(addr);
#endif
}

}  // namespace ark::verifier
