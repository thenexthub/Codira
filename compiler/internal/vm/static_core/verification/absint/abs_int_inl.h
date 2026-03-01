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

#ifndef PANDA_VERIFICATION_ABSINT_ABS_INT_INL_H
#define PANDA_VERIFICATION_ABSINT_ABS_INT_INL_H

#include "abs_int_inl_compat_checks.h"
#include "libarkbase/utils/small_vector.h"
#include "libarkbase/utils/span.h"
#include "libarkfile/file_items.h"
#include "include/mem/panda_containers.h"
#include "include/method.h"
#include "include/runtime.h"
#include "libarkfile/type_helper.h"
#include "libarkbase/macros.h"
#include "runtime/include/class.h"
#include "runtime/include/thread_scopes.h"
#include "type/type_system.h"
#include "libarkbase/utils/logger.h"
#include "type/type_type.h"
#include "util/str.h"
#include "verification/config/debug_breakpoint/breakpoint.h"
#include "verification_context.h"
#include "verification/type/type_system.h"
#include "verification_status.h"
#include "verifier_messages.h"

// CC-OFFNXT(G.PRE.06) solid logic
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_INST()                                               \
    do {                                                         \
        if (!GetInst().IsValid()) {                              \
            SHOW_MSG(InvalidInstruction)                         \
            LOG_VERIFIER_INVALID_INSTRUCTION();                  \
            END_SHOW_MSG();                                      \
            SET_STATUS_FOR_MSG(InvalidInstruction, WARNING);     \
            return false; /* CC-OFF(G.PRE.05) code generation */ \
        }                                                        \
        if (job_->Options().ShowContext()) {                     \
            DumpRegs(ExecCtx().CurrentRegContext());             \
        }                                                        \
        SHOW_MSG(DebugAbsIntLogInstruction)                      \
        LOG_VERIFIER_DEBUG_ABS_INT_LOG_INSTRUCTION(GetInst());   \
        END_SHOW_MSG();                                          \
    } while (0)

#ifndef NDEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DBGBRK()                                                                      \
    if (debug_) {                                                                     \
        DBG_MANAGED_BRK(debugCtx, job_->JobMethod()->GetUniqId(), inst_.GetOffset()); \
    }
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DBGBRK()
#endif

// CC-OFFNXT(G.PRE.02) namespace member
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SHOW_MSG(Message) if (!job_->Options().IsHidden(VerifierMessage::Message)) {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define END_SHOW_MSG() }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SET_STATUS_FOR_MSG(Message, AtLeast)                                                       \
    do {                                                                                           \
        SetStatusAtLeast(VerificationStatus::AtLeast); /* CC-OFF(G.PRE.02) namespace member */     \
        /* CC-OFFNXT(G.PRE.02) namespace member */                                                 \
        SetStatusAtLeast(MsgClassToStatus(job_->Options().MsgClassFor(VerifierMessage::Message))); \
    } while (0)

/*
NOTE(vdyadov): add AddrMap to verification context where put marks on all checked bytes.
after absint process ends, check this AddrMap for holes.
holes are either dead byte-code or issues with absint cflow handling.
*/

// NOTE(vdyadov): refactor this file, all utility functions put in separate/other files

/*
NOTE(vdyadov): IMPORTANT!!! (Done)
Current definition of context incompatibility is NOT CORRECT one!
There are situations when verifier will rule out fully correct programs.
For instance:
....
movi v0,0
ldai 0
jmp label1
....
lda.str ""
sta v0
ldai 0
jmp label1
.....
label1:
  return

Here we have context incompatibility on label1, but it does not harm, because conflicting reg is
not used anymore.

Solutions:
1(current). Conflicts are reported as warnings, conflicting regs are removed fro resulting context.
So, on attempt of usage of conflicting reg, absint will fail with message of undefined reg.
May be mark them as conflicting? (done)
2. On each label/checkpoint compute set of registers that will be used in next conputations and process
conflicting contexts modulo used registers set. It is complex solution, but very preciese.
*/

// NOTE(vdyadov): regain laziness, strict evaluation is very expensive!

namespace ark::verifier {

class AbsIntInstructionHandler {
public:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    static constexpr int ACC = -1;
    static constexpr int INVALID_REG = -2;  // NOTE(vdyadov): may be use Index<..> here?

    using TypeId = panda_file::Type::TypeId;
    using Builtin = Type::Builtin;

    Type const top_ {Builtin::TOP};
    Type const bot_ {Builtin::BOT};
    Type const u1_ {Builtin::U1};
    Type const i8_ {Builtin::I8};
    Type const u8_ {Builtin::U8};
    Type const i16_ {Builtin::I16};
    Type const u16_ {Builtin::U16};
    Type const i32_ {Builtin::I32};
    Type const u32_ {Builtin::U32};
    Type const i64_ {Builtin::I64};
    Type const u64_ {Builtin::U64};
    Type const f32_ {Builtin::F32};
    Type const f64_ {Builtin::F64};
    Type const integral32_ {Builtin::INTEGRAL32};
    Type const integral64_ {Builtin::INTEGRAL64};
    Type const float32_ {Builtin::FLOAT32};
    Type const float64_ {Builtin::FLOAT64};
    Type const bits32_ {Builtin::BITS32};
    Type const bits64_ {Builtin::BITS64};
    Type const primitive_ {Builtin::PRIMITIVE};
    Type const refType_ {Builtin::REFERENCE};
    Type const nullRefType_ {Builtin::NULL_REFERENCE};
    Type const objectType_ {Builtin::OBJECT};
    Type const arrayType_ {Builtin::ARRAY};

    Job const *job_;
    debug::DebugContext const *debugCtx;
    Config const *config;

    // NOLINTEND(misc-non-private-member-variables-in-classes)

    AbsIntInstructionHandler(VerificationContext &verifCtx, const uint8_t *pc, EntryPointType codeType)
        : job_ {verifCtx.GetJob()},
          debugCtx {&job_->GetService()->debugCtx},
          config {GetServiceConfig(job_->GetService())},
          inst_(pc, verifCtx.CflowInfo().GetAddrStart(), verifCtx.CflowInfo().GetAddrEnd()),
          context_ {verifCtx},
          codeType_ {codeType}
    {
#ifndef NDEBUG
        if (config->opts.mode == VerificationMode::DEBUG) {
            const auto &method = job_->JobMethod();
            debug_ = debug::ManagedBreakpointPresent(debugCtx, method->GetUniqId());
            if (debug_) {
                LOG(DEBUG, VERIFIER) << "Debug mode for method " << method->GetFullName() << " is on";
            }
        }
#endif
    }

    ~AbsIntInstructionHandler() = default;
    NO_MOVE_SEMANTIC(AbsIntInstructionHandler);
    NO_COPY_SEMANTIC(AbsIntInstructionHandler);

    VerificationStatus GetStatus()
    {
        return status_;
    }

    uint8_t GetPrimaryOpcode()
    {
        return inst_.GetPrimaryOpcode();
    }

    uint8_t GetSecondaryOpcode()
    {
        return inst_.GetSecondaryOpcode();
    }

    bool IsPrimaryOpcodeValid() const
    {
        return inst_.IsPrimaryOpcodeValid();
    }

    bool IsRegDefined(int reg);

    PandaString ToString(Type tp) const
    {
        return tp.ToString(GetTypeSystem());
    }

    PandaString ToString(PandaVector<Type> const &types) const
    {
        PandaString result {"[ "};
        bool comma = false;
        for (const auto &type : types) {
            if (comma) {
                result += ", ";
            }
            result += ToString(type);
            comma = true;
        }
        result += " ]";
        return result;
    }

    PandaString ToString(AbstractTypedValue const *atv) const
    {
        return atv->ToString(GetTypeSystem());
    }

    static PandaString StringDataToString(panda_file::File::StringData sd)
    {
        PandaString res {reinterpret_cast<char *>(const_cast<uint8_t *>(sd.data))};
        return res;
    }

    bool CheckType(Type type, Type tgtType)
    {
        return IsSubtype(type, tgtType, GetTypeSystem());
    }

    bool CheckRegType(int reg, Type tgtType);

    bool CheckRegTypeNotNullRef(int reg);

    bool CheckRegTypes(std::initializer_list<int> const &regs, std::initializer_list<Type> const &tgtTypes);

    const AbstractTypedValue &GetReg(int regIdx);

    Type GetRegType(int regIdx);

    void SetReg(int regIdx, const AbstractTypedValue &val);
    void SetReg(int regIdx, Type type);

    void SetRegAndOthersOfSameOrigin(int regIdx, const AbstractTypedValue &val);
    void SetRegAndOthersOfSameOrigin(int regIdx, Type type);

    const AbstractTypedValue &GetAcc();

    Type GetAccType();

    void SetAcc(const AbstractTypedValue &val);
    void SetAcc(Type type);

    void SetAccAndOthersOfSameOrigin(const AbstractTypedValue &val);
    void SetAccAndOthersOfSameOrigin(Type type);

    AbstractTypedValue MkVal(Type t);

    TypeSystem *GetTypeSystem() const
    {
        return context_.GetTypeSystem();
    }

    Type TypeOfClass(Class const *klass);

    Type ReturnType();

    ExecContext &ExecCtx();

    void DumpRegs(const RegContext &ctx);

    bool CheckCtxCompatibility(const RegContext &src, const RegContext &dst);

    void Sync();

    void AssignRegToReg(int dst, int src)
    {
        auto atv = GetReg(src);
        if (!atv.GetOrigin().IsValid()) {
            // generate new origin and set all values to be originated at it
            AbstractTypedValue newAtv {atv, inst_};
            SetReg(src, newAtv);
            SetReg(dst, newAtv);
        } else {
            SetReg(dst, atv);
        }
    }

    void AssignAccToReg(int src)
    {
        AssignRegToReg(ACC, src);
    }

    void AssignRegToAcc(int dst)
    {
        AssignRegToReg(dst, ACC);
    }

    // Names meanings: vs - v_source, vd - v_destination
    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMov()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegType(vs, bits32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignRegToReg(vd, vs);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMovWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegType(vs, bits64_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignRegToReg(vd, vs);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMovObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        if (!CheckRegType(vs, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignRegToReg(vd, vs);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMovDyn()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        LOG(ERROR, VERIFIER) << "Verifier error: instruction is not implemented";
        status_ = VerificationStatus::ERROR;
        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNop()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMovi()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        SetReg(vd, i32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMoviWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        SetReg(vd, i64_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmovi()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        SetReg(vd, f32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmoviWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        SetReg(vd, f64_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMovNull()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        SetReg(vd, nullRefType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLda()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(vs, bits32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignAccToReg(vs);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaDyn();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaiDyn();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(vs, bits64_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignAccToReg(vs);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(vs, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignAccToReg(vs);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdai()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        SetAcc(i32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaiWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        SetAcc(i64_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFldai()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        SetAcc(f32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFldaiWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        SetAcc(f64_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFldaiDyn();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaStr()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        Type cachedType = GetCachedType();
        if (!cachedType.IsConsistent()) {
            SET_STATUS_FOR_MSG(CannotResolveClassId, OK);
            return false;
        }
        SetAcc(cachedType);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaConst()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        Sync();
        Type cachedType = GetCachedType();
        if (!cachedType.IsConsistent()) {
            // NOTE(vdyadov): refactor to verifier-messages
            LOG(ERROR, VERIFIER) << "Verifier error: HandleLdaConst cache error";
            status_ = VerificationStatus::ERROR;
            return false;
        }
        SetReg(vd, cachedType);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaType()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        Type cachedType = GetCachedType();
        if (!cachedType.IsConsistent()) {
            SET_STATUS_FOR_MSG(CannotResolveClassId, OK);
            return false;
        }
        if (cachedType != GetTypeSystem()->ClassClass()) {
            LOG(ERROR, VERIFIER) << "LDA_TYPE type must be Class.";
            return false;
        }
        SetAcc(GetTypeSystem()->ClassClass());
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdaNull()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        SetAcc(nullRefType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSta()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(ACC, bits32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignRegToAcc(vd);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStaDyn();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStaWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(ACC, bits64_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignRegToAcc(vd);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStaObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(ACC, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        AssignRegToAcc(vd);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJmp()
    {
        LOG_INST();
        DBGBRK();
        int32_t imm = inst_.GetImm<FORMAT>();
        Sync();
        ProcessBranching(imm);
        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCmpWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleUcmp();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleUcmpWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFcmpl();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFcmplWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFcmpg();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFcmpgWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJeqz();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJeqzObj()
    {
        LOG_INST();
        DBGBRK();
        auto imm = inst_.GetImm<FORMAT>();

        Sync();

        if (!CheckRegType(ACC, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        // NOTE(vdyadov): think of two-pass absint, where we can catch const-null cases

        auto type = GetRegType(ACC);
        SetAccAndOthersOfSameOrigin(nullRefType_);

        if (!ProcessBranching(imm)) {
            return false;
        }

        SetAccAndOthersOfSameOrigin(type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJnez();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJnezObj()
    {
        LOG_INST();
        DBGBRK();
        auto imm = inst_.GetImm<FORMAT>();
        Sync();

        if (!CheckRegType(ACC, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!ProcessBranching(imm)) {
            return false;
        }

        SetAccAndOthersOfSameOrigin(nullRefType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJltz();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJgtz();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJlez();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJgez();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJeq();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJeqObj()
    {
        LOG_INST();
        DBGBRK();
        auto imm = inst_.GetImm<FORMAT>();
        uint16_t vs = inst_.GetVReg<FORMAT>();

        Sync();

        if (!CheckRegType(ACC, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!CheckRegType(vs, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!ProcessBranching(imm)) {
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJne();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJneObj()
    {
        LOG_INST();
        DBGBRK();
        auto imm = inst_.GetImm<FORMAT>();
        uint16_t vs = inst_.GetVReg<FORMAT>();

        Sync();

        if (!CheckRegType(ACC, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!CheckRegType(vs, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!ProcessBranching(imm)) {
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJlt();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJgt();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJle();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleJge();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAdd2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAdd2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFadd2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFadd2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSub2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSub2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFsub2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFsub2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMul2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMul2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmul2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmul2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFdiv2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFdiv2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmod2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmod2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnd2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnd2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOr2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOr2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXor2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXor2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShl2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShl2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShr2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShr2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshr2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshr2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDiv2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDiv2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMod2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMod2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDivu2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDivu2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModu2();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModu2Wide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAdd2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAdd2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFadd2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFadd2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSub2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSub2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFsub2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFsub2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMul2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMul2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmul2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmul2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFdiv2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFdiv2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmod2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFmod2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnd2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnd2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOr2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOr2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXor2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXor2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShl2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShl2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShr2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShr2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshr2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshr2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDiv2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDiv2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMod2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMod2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDivu2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDivu2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModu2v();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModu2vWide();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAdd();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSub();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMul();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnd();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOr();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXor();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShl();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShr();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshr();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDiv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMod();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAddv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSubv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMulv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAndv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOrv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXorv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShlv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShrv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshrv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDivv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAddi();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSubi();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMuli();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAndi();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOri();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXori();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShli();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShri();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshri();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDivi();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModi();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAddiv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleSubiv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMuliv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAndiv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleOriv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleXoriv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShliv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleShriv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAshriv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleDiviv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleModiv();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNeg()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return CheckUnaryOp<FORMAT>(integral32_, i32_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNegWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return CheckUnaryOp<FORMAT>(integral64_, i64_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFneg()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return CheckUnaryOp<FORMAT>(f32_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFnegWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return CheckUnaryOp<FORMAT>(f64_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNot()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return CheckUnaryOp<FORMAT>(integral32_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNotWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return CheckUnaryOp<FORMAT>(integral64_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleInci()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vx = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(vx, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32toi64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32toi16();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32tou16();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32toi8();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32tou8();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32tou1();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32tof32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI32tof64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32toi64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32toi16();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32tou16();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32toi8();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32tou8();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32tou1();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32tof32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU32tof64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI64toi32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI64tou1();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI64tof32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleI64tof64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU64toi32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU64tou32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU64tou1();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU64tof32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleU64tof64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF32tof64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF32toi32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF32toi64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF32tou32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF32tou64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF64tof32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF64toi64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF64toi32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF64tou64();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleF64tou32();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallPolymorphicShort()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallPolymorphic()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallPolymorphicRange()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        // It is a runtime (not verifier) responibility to check the MethodHandle.invoke()
        // parameter types and throw the WrongMethodTypeException if need
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallePolymorphicShort()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallePolymorphic()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallePolymorphicRange()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        // It is a runtime (not verifier) responibility to check the  MethodHandle.invokeExact()
        // parameter types and throw the WrongMethodTypeException if need
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarr8()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {u1_, i8_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarr16()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {i16_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarr()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {i32_, u32_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarrWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {i64_, u64_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarru8()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {u1_, u8_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarru16()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {u16_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFldarr32()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {f32_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFldarrWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return CheckArrayLoad<FORMAT>(vs, {f64_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdarrObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(ACC, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!CheckRegType(vs, arrayType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        auto regType = GetRegType(vs);
        if (regType == nullRefType_) {
            // NOTE(vdyadov): redesign next code, after support exception handlers,
            //                treat it as always throw NPE
            SHOW_MSG(AlwaysNpe)
            LOG_VERIFIER_ALWAYS_NPE(vs);
            END_SHOW_MSG();
            SetAcc(top_);
            SET_STATUS_FOR_MSG(AlwaysNpe, OK);
            return false;
        }

        auto arrEltType = regType.GetArrayElementType(GetTypeSystem());
        if (!IsSubtype(arrEltType, refType_, GetTypeSystem())) {
            SHOW_MSG(BadArrayElementType)
            LOG_VERIFIER_BAD_ARRAY_ELEMENT_TYPE(ToString(arrEltType), ToString(refType_));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(BadArrayElementType, WARNING);
            return false;
        }
        SetAcc(arrEltType);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStarr8()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStoreExact<FORMAT>(v1, v2, integral32_, {u1_, i8_, u8_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStarr16()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStoreExact<FORMAT>(v1, v2, integral32_, {i16_, u16_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStarr()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStoreExact<FORMAT>(v1, v2, integral32_, {i32_, u32_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStarrWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStoreExact<FORMAT>(v1, v2, integral64_, {i64_, u64_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFstarr32()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStoreExact<FORMAT>(v1, v2, float32_, {f32_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleFstarrWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStoreExact<FORMAT>(v1, v2, float64_, {f64_});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStarrObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        return CheckArrayStore<FORMAT>(v1, v2, refType_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLenarr()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        if (!CheckRegType(vs, arrayType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetAcc(i32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNewarr()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        if (!CheckRegType(vs, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        Type type = GetCachedType();
        if (!type.IsConsistent()) {
            SET_STATUS_FOR_MSG(CannotResolveClassId, OK);
            return false;
        }
        SHOW_MSG(DebugType)
        LOG_VERIFIER_DEBUG_TYPE(ToString(type));
        END_SHOW_MSG();
        if (!IsSubtype(type, arrayType_, GetTypeSystem())) {
            // NOTE(vdyadov): implement StrictSubtypes function to not include array_type_ in output
            SHOW_MSG(ArrayOfNonArrayType)
            LOG_VERIFIER_ARRAY_OF_NON_ARRAY_TYPE(ToString(type));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(ArrayOfNonArrayType, WARNING);
            return false;
        }
        SetReg(vd, type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleNewobj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        Sync();
        Type cachedType = GetCachedType();
        if (!cachedType.IsConsistent()) {
            LOG(ERROR, VERIFIER) << "Verifier error: HandleNewobj cache error";
            status_ = VerificationStatus::ERROR;
            return false;
        }
        SHOW_MSG(DebugType)
        LOG_VERIFIER_DEBUG_TYPE(ToString(cachedType));
        END_SHOW_MSG();
        if (!IsSubtype(cachedType, objectType_, GetTypeSystem())) {
            SHOW_MSG(ObjectOfNonObjectType)
            LOG_VERIFIER_OBJECT_OF_NON_OBJECT_TYPE(ToString(cachedType));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(ObjectOfNonObjectType, WARNING);
            return false;
        }
        SetReg(vd, cachedType);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckCallCtor(Method const *ctor, Span<int> regs)
    {
        Type objType = TypeOfClass(ctor->GetClass());

        // NOTE(vdyadov): put under NDEBUG?
        {
            if (debugCtx->SkipVerificationOfCall(ctor->GetUniqId())) {
                SetAcc(objType);
                MoveToNextInst<FORMAT>();
                return true;
            }
        }

        auto ctorNameGetter = [&ctor]() { return ctor->GetFullName(); };

        auto const &formalArgs = GetTypeSystem()->GetMethodSignature(ctor)->args;
        bool check = CheckMethodArgs(ctorNameGetter, formalArgs, regs, objType);
        if (check) {
            SetAcc(objType);
            MoveToNextInst<FORMAT>();
        }
        return check;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckCtor(Span<int> regs)
    {
        Type type = GetCachedType();
        if (UNLIKELY(type.IsClass() && type.GetClass()->IsArrayClass())) {
            if (job_->IsMethodPresentForOffset(inst_.GetOffset())) {
                // Array constructors are synthetic methods; ClassLinker does not provide them.
                LOG(ERROR, VERIFIER) << "Verifier internal error: ArrayCtor should not be instantiated as method";
                return false;
            }
            SHOW_MSG(DebugArrayConstructor)
            LOG_VERIFIER_DEBUG_ARRAY_CONSTRUCTOR();
            END_SHOW_MSG();
            return CheckArrayCtor<FORMAT>(type, regs);
        }

        Method const *ctor = GetCachedMethod();

        if (!type.IsConsistent() || ctor == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveMethodId, OK);
            SET_STATUS_FOR_MSG(CannotResolveClassId, OK);
            return false;
        }

        PandaString expectedName = ark::panda_file::GetCtorName(ctor->GetClass()->GetSourceLang());
        if (!ctor->IsConstructor() || ctor->IsStatic() || expectedName != StringDataToString(ctor->GetName())) {
            SHOW_MSG(InitobjCallsNotConstructor)
            LOG_VERIFIER_INITOBJ_CALLS_NOT_CONSTRUCTOR(ctor->GetFullName());
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(InitobjCallsNotConstructor, WARNING);
            return false;
        }

        SHOW_MSG(DebugConstructor)
        LOG_VERIFIER_DEBUG_CONSTRUCTOR(ctor->GetFullName());
        END_SHOW_MSG();

        return CheckCallCtor<FORMAT>(ctor, regs);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleInitobj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        uint16_t vs3 = inst_.GetVReg<FORMAT, 0x02>();
        uint16_t vs4 = inst_.GetVReg<FORMAT, 0x03>();
        Sync();
        std::array<int, 4UL> regs {vs1, vs2, vs3, vs4};
        return CheckCtor<FORMAT>(Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleInitobjShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();
        std::array<int, 2UL> regs {vs1, vs2};
        return CheckCtor<FORMAT>(Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleInitobjRange()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x00>();
        Sync();
        std::vector<int> regs;
        for (auto regIdx = vs; ExecCtx().CurrentRegContext().IsRegDefined(regIdx); regIdx++) {
            regs.push_back(regIdx);
        }
        return CheckCtor<FORMAT>(Span {regs});
    }

    Type GetFieldType()
    {
        const Field *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return {};
        }

        ScopedChangeThreadStatus st {ManagedThread::GetCurrent(), ThreadStatus::RUNNING};
        Job::ErrorHandler handler;
        auto typeCls = field->ResolveTypeClass(&handler);
        if (typeCls == nullptr) {
            return Type {};
        }
        return Type {typeCls};
    }

    Type GetFieldObject()
    {
        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return {};
        }
        return TypeOfClass(field->GetClass());
    }

    bool CheckFieldAccess(int regIdx, Type expectedFieldType, bool isStatic, bool isVolatile)
    {
        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }

        if (!CheckFieldAccessStaticVolatile(isStatic, isVolatile, field)) {
            return false;
        }

        Type fieldObjType = GetFieldObject();
        Type fieldType = GetFieldType();
        if (!fieldType.IsConsistent()) {
            LOG_VERIFIER_CANNOT_RESOLVE_FIELD_TYPE(GetFieldName(field));
            return false;
        }

        if (!isStatic) {
            if (!IsRegDefined(regIdx)) {
                SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
                return false;
            }
            Type objType = GetRegType(regIdx);
            if (objType == nullRefType_) {
                // NOTE(vdyadov): redesign next code, after support exception handlers,
                //                treat it as always throw NPE
                SHOW_MSG(AlwaysNpe)
                LOG_VERIFIER_ALWAYS_NPE(regIdx);
                END_SHOW_MSG();
                SET_STATUS_FOR_MSG(AlwaysNpe, OK);
                return false;
            }
            if (!IsSubtype(objType, fieldObjType, GetTypeSystem())) {
                SHOW_MSG(InconsistentRegisterAndFieldTypes)
                LOG_VERIFIER_INCONSISTENT_REGISTER_AND_FIELD_TYPES(GetFieldName(field), regIdx, ToString(objType),
                                                                   ToString(fieldObjType));
                END_SHOW_MSG();
                SET_STATUS_FOR_MSG(InconsistentRegisterAndFieldTypes, WARNING);
            }
        }

        if (!IsSubtype(fieldType, expectedFieldType, GetTypeSystem())) {
            SHOW_MSG(UnexpectedFieldType)
            LOG_VERIFIER_UNEXPECTED_FIELD_TYPE(GetFieldName(field), ToString(fieldType), ToString(expectedFieldType));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(UnexpectedFieldType, WARNING);
            return false;
        }

        return CheckFieldAccessPlugin(field);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessFieldLoad(int regDest, int regSrc, Type expectedFieldType, bool isStatic, bool isVolatile = false)
    {
        if (!CheckFieldAccess(regSrc, expectedFieldType, isStatic, isVolatile)) {
            return false;
        }
        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }

        auto type = GetFieldType();
        if (!type.IsConsistent()) {
            return false;
        }
        SetReg(regDest, type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessFieldLoad(int regIdx, Type expectedFieldType, bool isStatic)
    {
        return ProcessFieldLoad<FORMAT>(ACC, regIdx, expectedFieldType, isStatic);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessFieldLoadVolatile(int regDest, int regSrc, Type expectedFieldType, bool isStatic)
    {
        return ProcessFieldLoad<FORMAT>(regDest, regSrc, expectedFieldType, isStatic, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessFieldLoadVolatile(int regIdx, Type expectedFieldType, bool isStatic)
    {
        return ProcessFieldLoadVolatile<FORMAT>(ACC, regIdx, expectedFieldType, isStatic);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoad<FORMAT>(vs, bits32_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoad<FORMAT>(vs, bits64_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoad<FORMAT>(vs, refType_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjV()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessFieldLoad<FORMAT>(vd, vs, bits32_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessFieldLoad<FORMAT>(vd, vs, bits64_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessFieldLoad<FORMAT>(vd, vs, refType_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVolatile()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(vs, bits32_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVolatileWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(vs, bits64_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVolatileObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(vs, refType_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVolatileV()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(vd, vs, bits32_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVolatileVWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(vd, vs, bits64_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdobjVolatileVObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0>();
        uint16_t vs = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(vd, vs, refType_, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT, bool IS_VOLATILE = false, typename Check>
    bool ProcessStoreField(int vs, int vd, Type expectedFieldType, bool isStatic, Check check)
    {
        if (!CheckRegType(vs, expectedFieldType)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!CheckFieldAccess(vd, expectedFieldType, isStatic, IS_VOLATILE)) {
            return false;
        }

        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }
        Type fieldType = GetFieldType();
        if (!fieldType.IsConsistent()) {
            return false;
        }

        Type vsType = GetRegType(vs);

        CheckResult const &result = check(fieldType.ToTypeId(), vsType.ToTypeId());
        if (result.status != VerificationStatus::OK) {
            LOG_VERIFIER_DEBUG_STORE_FIELD(GetFieldName(field), ToString(fieldType), ToString(vsType));
            status_ = result.status;
            if (result.IsError()) {
                return false;
            }
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobj(int vs, int vd, bool isStatic)
    {
        return ProcessStoreField<FORMAT>(vs, vd, bits32_, isStatic, CheckStobj);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobj(int vd, bool isStatic)
    {
        return ProcessStobj<FORMAT>(ACC, vd, isStatic);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjVolatile(int vs, int vd, bool isStatic)
    {
        return ProcessStoreField<FORMAT, true>(vs, vd, bits32_, isStatic, CheckStobj);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjVolatile(int vd, bool isStatic)
    {
        return ProcessStobjVolatile<FORMAT>(ACC, vd, isStatic);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();

        return ProcessStobj<FORMAT>(vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjV()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0>();
        uint16_t vd = inst_.GetVReg<FORMAT, 1>();
        Sync();

        return ProcessStobj<FORMAT>(vs, vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVolatile()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();

        return ProcessStobjVolatile<FORMAT>(vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVolatileV()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0>();
        uint16_t vd = inst_.GetVReg<FORMAT, 1>();
        Sync();

        return ProcessStobjVolatile<FORMAT>(vs, vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjWide(int vs, int vd, bool isStatic)
    {
        return ProcessStoreField<FORMAT>(vs, vd, bits64_, isStatic, CheckStobjWide);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjWide(int vd, bool isStatic)
    {
        return ProcessStobjWide<FORMAT>(ACC, vd, isStatic);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjVolatileWide(int vs, int vd, bool isStatic)
    {
        return ProcessStoreField<FORMAT, true>(vs, vd, bits64_, isStatic, CheckStobjWide);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjVolatileWide(int vd, bool isStatic)
    {
        return ProcessStobjVolatileWide<FORMAT>(ACC, vd, isStatic);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();

        return ProcessStobjWide<FORMAT>(vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0>();
        uint16_t vd = inst_.GetVReg<FORMAT, 1>();
        Sync();

        return ProcessStobjWide<FORMAT>(vs, vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVolatileWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();

        return ProcessStobjVolatileWide<FORMAT>(vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVolatileVWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0>();
        uint16_t vd = inst_.GetVReg<FORMAT, 1>();
        Sync();

        return ProcessStobjVolatileWide<FORMAT>(vs, vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjObj(int vs, int vd, bool isStatic, bool isVolatile = false)
    {
        if (!CheckFieldAccess(vd, refType_, isStatic, isVolatile)) {
            return false;
        }

        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }

        Type fieldType = GetFieldType();
        if (!fieldType.IsConsistent()) {
            return false;
        }

        if (!CheckRegType(vs, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        Type vsType = GetRegType(vs);
        if (!IsSubtype(vsType, fieldType, GetTypeSystem())) {
            SHOW_MSG(BadAccumulatorType)
            LOG_VERIFIER_BAD_ACCUMULATOR_TYPE(ToString(vsType), ToString(fieldType));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(BadAccumulatorType, WARNING);
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjObj(int vd, bool isStatic)
    {
        return ProcessStobjObj<FORMAT>(ACC, vd, isStatic);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjVolatileObj(int vs, int vd, bool isStatic)
    {
        return ProcessStobjObj<FORMAT>(vs, vd, isStatic, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjVolatileObj(int vd, bool isStatic)
    {
        return ProcessStobjVolatileObj<FORMAT>(ACC, vd, isStatic);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessStobjObj<FORMAT>(vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0>();
        uint16_t vd = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessStobjObj<FORMAT>(vs, vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVolatileObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessStobjVolatileObj<FORMAT>(vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStobjVolatileVObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0>();
        uint16_t vd = inst_.GetVReg<FORMAT, 1>();
        Sync();
        return ProcessStobjVolatileObj<FORMAT>(vs, vd, false);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdstatic()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessFieldLoad<FORMAT>(INVALID_REG, bits32_, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdstaticWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessFieldLoad<FORMAT>(INVALID_REG, bits64_, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdstaticObj()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessFieldLoad<FORMAT>(INVALID_REG, refType_, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdstaticVolatile()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(INVALID_REG, bits32_, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdstaticVolatileWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(INVALID_REG, bits64_, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleLdstaticVolatileObj()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessFieldLoadVolatile<FORMAT>(INVALID_REG, refType_, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStstatic()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessStobj<FORMAT>(INVALID_REG, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStstaticWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessStobjWide<FORMAT>(INVALID_REG, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStstaticObj()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessStobjObj<FORMAT>(INVALID_REG, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStstaticVolatile()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessStobjVolatile<FORMAT>(INVALID_REG, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStstaticVolatileWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessStobjVolatileWide<FORMAT>(INVALID_REG, true);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleStstaticVolatileObj()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        return ProcessStobjVolatileObj<FORMAT>(INVALID_REG, true);
    }

    template <typename Check>
    bool CheckReturn(Type retType, Type accType, Check check)
    {
        TypeId retTypeId = retType.ToTypeId();

        PandaVector<Type> compatibleAccTypes;
        // NOTE (gogabr): why recompute each time?
        for (size_t accIdx = 0; accIdx < static_cast<size_t>(TypeId::REFERENCE) + 1; ++accIdx) {
            auto accTypeId = static_cast<TypeId>(accIdx);
            const CheckResult &info = check(retTypeId, accTypeId);
            if (!info.IsError()) {
                compatibleAccTypes.push_back(Type::FromTypeId(accTypeId));
            }
        }

        if (!CheckType(accType, primitive_) || accType == primitive_) {
            LOG_VERIFIER_BAD_ACCUMULATOR_RETURN_VALUE_TYPE(ToString(accType));
            SET_STATUS_FOR_MSG(BadAccumulatorReturnValueType, WARNING);
            return false;
        }

        TypeId accTypeId = accType.ToTypeId();

        const auto &result = check(retTypeId, accTypeId);

        if (!result.IsOk()) {
            LogInnerMessage(result);
            if (result.IsError()) {
                LOG_VERIFIER_DEBUG_FUNCTION_RETURN_AND_ACCUMULATOR_TYPES_WITH_COMPATIBLE_TYPES(
                    ToString(ReturnType()), ToString(accType), ToString(compatibleAccTypes));
            } else {
                LOG_VERIFIER_DEBUG_FUNCTION_RETURN_AND_ACCUMULATOR_TYPES(ToString(ReturnType()), ToString(accType));
            }
        }

        status_ = result.status;
        return status_ != VerificationStatus::ERROR;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleReturn()
    {
        LOG_INST();
        DBGBRK();
        Sync();

        if (!CheckType(ReturnType(), bits32_)) {
            LOG_VERIFIER_BAD_RETURN_INSTRUCTION_TYPE("", ToString(ReturnType()), ToString(bits32_));
            SET_STATUS_FOR_MSG(BadReturnInstructionType, WARNING);
            return false;
        }

        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        // NOTE(vdyadov): handle LUB of compatible primitive types
        if (!CheckType(GetAccType(), bits32_)) {
            LOG_VERIFIER_BAD_ACCUMULATOR_RETURN_VALUE_TYPE(ToString(GetAccType()));
            SET_STATUS_FOR_MSG(BadAccumulatorReturnValueType, WARNING);
        }
        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleReturnDyn();

    template <bool IS_LOAD>
    bool CheckFieldAccessByName(int regIdx, [[maybe_unused]] Type expectedFieldType)
    {
        Field const *rawField = GetCachedField();

        if (rawField == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }

        if (rawField->IsStatic()) {
            SHOW_MSG(ExpectedStaticOrInstanceField)
            LOG_VERIFIER_EXPECTED_STATIC_OR_INSTANCE_FIELD(false);
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(ExpectedStaticOrInstanceField, WARNING);
            return false;
        }

        if (!GetFieldType().IsConsistent()) {
            LOG_VERIFIER_CANNOT_RESOLVE_FIELD_TYPE(GetFieldName(rawField));
            return false;
        }

        if (!CheckRegType(regIdx, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (GetRegType(regIdx) == nullRefType_) {
            // NOTE(vdyadov): redesign next code, after support exception handlers,
            //                treat it as always throw NPE
            SHOW_MSG(AlwaysNpe)
            LOG_VERIFIER_ALWAYS_NPE(regIdx);
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(AlwaysNpe, OK);
            return false;
        }
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessFieldLoadByName(int regSrc, Type expectedFieldType)
    {
        if (!CheckFieldAccessByName<true>(regSrc, expectedFieldType)) {
            return false;
        }
        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }

        auto type = GetFieldType();
        if (!type.IsConsistent()) {
            return false;
        }
        SetReg(ACC, type);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLdobjName()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoadByName<FORMAT>(vs, bits32_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLdobjNameWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoadByName<FORMAT>(vs, bits64_);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLdobjNameObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessFieldLoadByName<FORMAT>(vs, refType_);
    }

    template <BytecodeInstructionSafe::Format FORMAT, typename Check>
    bool ProcessStoreFieldByName(int vd, Type expectedFieldType, Check check)
    {
        if (!CheckRegType(ACC, expectedFieldType)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckFieldAccessByName<false>(vd, expectedFieldType)) {
            return false;
        }

        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }
        Type fieldType = GetFieldType();
        if (!fieldType.IsConsistent()) {
            return false;
        }

        Type vsType = GetRegType(ACC);

        CheckResult const &result = check(fieldType.ToTypeId(), vsType.ToTypeId());
        if (result.status != VerificationStatus::OK) {
            LOG_VERIFIER_DEBUG_STORE_FIELD(GetFieldName(field), ToString(fieldType), ToString(vsType));
            status_ = result.status;
            if (result.IsError()) {
                return false;
            }
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool ProcessStobjObjByName(int vd)
    {
        if (!CheckFieldAccessByName<false>(vd, refType_)) {
            return false;
        }

        Field const *field = GetCachedField();

        if (field == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveFieldId, OK);
            return false;
        }

        Type fieldType = GetFieldType();
        if (!fieldType.IsConsistent()) {
            return false;
        }

        if (!CheckRegType(ACC, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        Type vsType = GetRegType(ACC);
        if (!IsSubtype(vsType, fieldType, GetTypeSystem())) {
            SHOW_MSG(BadAccumulatorType)
            LOG_VERIFIER_BAD_ACCUMULATOR_TYPE(ToString(vsType), ToString(fieldType));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(BadAccumulatorType, WARNING);
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsStobjName()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();

        return ProcessStoreFieldByName<FORMAT>(vd, bits32_, CheckStobj);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsStobjNameWide()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();

        return ProcessStoreFieldByName<FORMAT>(vd, bits64_, CheckStobjWide);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsStobjNameObj()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        return ProcessStobjObjByName<FORMAT>(vd);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckEtsCall(Method const *method, Span<int> regs)
    {
        if (method == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveMethodId, OK);
            return false;
        }

        if (!CheckMethodAccessViolation(method)) {
            return false;
        }

        if (!debugCtx->SkipVerificationOfCall(method->GetUniqId())) {
            auto methodNameGetter = [method]() { return method->GetFullName(); };
            auto formalArgs = GetTypeSystem()->GetMethodSignature(method)->args;

            // Verifier checks only if 1st elem is Subtype of `reference type`
            // as it is supposed to be `this`. All other checks related to ets.call.name instructions
            // should be done in interpreter.
            if (!IsSubtype(GetRegType(regs[0]), refType_, GetTypeSystem())) {
                LOG_VERIFIER_BAD_REGISTER_TYPE(regs[0], ToString(GetRegType(regs[0])), ToString(refType_));
                status_ = VerificationStatus::ERROR;
                return false;
            }
            formalArgs.erase(formalArgs.begin());
            regs = regs.SubSpan(1, regs.Size() - 1);
            if (!CheckMethodArgs(methodNameGetter, formalArgs, regs, {})) {
                return false;
            }
        }

        SetAcc(GetTypeSystem()->GetMethodSignature(method)->result);

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsCallNameShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        Sync();
        std::array<int, 2UL> regs {vs1, vs2};
        return CheckEtsCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsCallName()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        uint16_t vs3 = inst_.GetVReg<FORMAT, 0x02>();
        uint16_t vs4 = inst_.GetVReg<FORMAT, 0x03>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        Sync();
        std::array<int, 4UL> regs {vs1, vs2, vs3, vs4};
        return CheckEtsCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsCallNameRange()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x00>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        Sync();
        constexpr size_t initSize = 5;
        SmallVector<int, initSize> regs;
        for (auto regIdx = vs; ExecCtx().CurrentRegContext().IsRegDefined(regIdx); regIdx++) {
            regs.push_back(regIdx);
        }
        return CheckEtsCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsLdnullvalue()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        SetAcc(GetCachedType());
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsMovnullvalue()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT>();
        Sync();
        SetReg(vd, GetCachedType());
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsIsnullvalue()
    {
        LOG_INST();
        DBGBRK();
        Sync();

        if (!CheckRegType(ACC, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetAcc(i32_);

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsEqualityHelper(uint16_t v1, uint16_t v2)
    {
        if (!CheckRegType(v1, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegType(v2, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetAcc(i32_);

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsEquals()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        return HandleEtsEqualityHelper<FORMAT>(v1, v2);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsStrictequals()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        return HandleEtsEqualityHelper<FORMAT>(v1, v2);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsTypeof()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        Sync();

        if (!CheckRegType(v1, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetAcc(GetTypeSystem()->StringClass());

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsIstrue()
    {
        LOG_INST();
        DBGBRK();
        uint16_t v = inst_.GetVReg<FORMAT, 0x00>();
        Sync();

        if (!CheckRegType(v, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetAcc(i32_);

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleEtsNullcheck()
    {
        LOG_INST();
        DBGBRK();
        Sync();

        if (!CheckType(GetAccType(), refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleReturnWide()
    {
        LOG_INST();
        DBGBRK();
        Sync();

        if (!CheckType(ReturnType(), bits64_)) {
            LOG_VERIFIER_BAD_RETURN_INSTRUCTION_TYPE(".64", ToString(ReturnType()), ToString(bits64_));
            status_ = VerificationStatus::ERROR;
            return false;
        }

        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!CheckType(GetAccType(), bits64_)) {
            LOG_VERIFIER_BAD_ACCUMULATOR_RETURN_VALUE_TYPE(ToString(GetAccType()));
            status_ = VerificationStatus::ERROR;
        }
        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleReturnObj()
    {
        LOG_INST();
        DBGBRK();
        Sync();

        if (!CheckType(ReturnType(), refType_)) {
            LOG_VERIFIER_BAD_RETURN_INSTRUCTION_TYPE(".obj", ToString(ReturnType()), ToString(refType_));
            status_ = VerificationStatus::ERROR;
            return false;
        }

        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        auto accType = GetAccType();
        if (!CheckType(accType, ReturnType())) {
            LOG_VERIFIER_BAD_ACCUMULATOR_RETURN_VALUE_TYPE_WITH_SUBTYPE(ToString(accType), ToString(ReturnType()));
            // NOTE(vdyadov) : after solving issues with set of types in LUB, uncomment next line
            status_ = VerificationStatus::WARNING;
        }

        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleReturnVoid()
    {
        LOG_INST();
        DBGBRK();
        // NOTE(vdyadov): think of introducing void as of separate type, like null
        Sync();

        if (ReturnType() != Type::Top()) {
            LOG_VERIFIER_BAD_RETURN_VOID_INSTRUCTION_TYPE(ToString(ReturnType()));
            status_ = VerificationStatus::ERROR;
        }

        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCheckcast()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        Type cachedType = GetCachedType();
        if (!CheckCastArrayObjectRegDef(cachedType)) {
            return false;
        }
        auto accType = GetAccType();
        // NOTE(vdyadov): remove this check after #2365
        if (!CheckCastRefArrayType(accType)) {
            return false;
        }

        if (IsSubtype(accType, nullRefType_, GetTypeSystem())) {
            LOG_VERIFIER_ACCUMULATOR_ALWAYS_NULL();
            SET_STATUS_FOR_MSG(AccumulatorAlwaysNull, OK);
            // Don't set types for "others of the same origin" when origin is null: n = null, a = n, b = n, a =
            // (NewType)x
            MoveToNextInst<FORMAT>();
            return true;
        }

        if (IsSubtype(accType, cachedType, GetTypeSystem())) {
            LOG_VERIFIER_REDUNDANT_CHECK_CAST(ToString(accType), ToString(cachedType));
            SET_STATUS_FOR_MSG(RedundantCheckCast, OK);
            // Do not update register type to parent type as we loose details and can get errors on further flow
            MoveToNextInst<FORMAT>();
            return true;
        }

        if (IsSubtype(cachedType, arrayType_, GetTypeSystem())) {
            auto eltType = cachedType.GetArrayElementType(GetTypeSystem());
            if (!IsSubtype(accType, arrayType_, GetTypeSystem()) && !IsSubtype(cachedType, accType, GetTypeSystem())) {
                LOG_VERIFIER_IMPOSSIBLE_CHECK_CAST(ToString(accType));
                status_ = VerificationStatus::WARNING;
            } else if (IsSubtype(accType, arrayType_, GetTypeSystem())) {
                auto accEltType = accType.GetArrayElementType(GetTypeSystem());
                if (accEltType.IsConsistent() && !IsSubtype(accEltType, eltType, GetTypeSystem()) &&
                    !IsSubtype(eltType, accEltType, GetTypeSystem())) {
                    LOG_VERIFIER_IMPOSSIBLE_ARRAY_CHECK_CAST(ToString(accEltType));
                    SET_STATUS_FOR_MSG(ImpossibleArrayCheckCast, OK);
                }
            }
        } else if (TpIntersection(cachedType, accType, GetTypeSystem()) == bot_) {
            LOG_VERIFIER_INCOMPATIBLE_ACCUMULATOR_TYPE(ToString(accType));
            SET_STATUS_FOR_MSG(IncompatibleAccumulatorType, OK);
        }

        if (status_ == VerificationStatus::ERROR) {
            SetAcc(top_);
            return false;
        }

        SetAccAndOthersOfSameOrigin(TpIntersection(cachedType, accType, GetTypeSystem()));

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleIsinstance()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        Type cachedType = GetCachedType();
        if (!CheckInstanceConsistentArrayObjectRegDef(cachedType)) {
            return false;
        }

        auto *plugin = job_->JobPlugin();
        auto const *jobMethod = job_->JobMethod();

        if (!CheckInstanceClass(cachedType, plugin, jobMethod)) {
            return false;
        }

        auto accType = GetAccType();
        // NOTE(vdyadov): remove this check after #2365
        if (!IsSubtype(accType, refType_, GetTypeSystem()) && !IsSubtype(accType, arrayType_, GetTypeSystem())) {
            LOG_VERIFIER_NON_OBJECT_ACCUMULATOR_TYPE();
            status_ = VerificationStatus::ERROR;
            return false;
        }

        if (IsSubtype(accType, nullRefType_, GetTypeSystem())) {
            LOG_VERIFIER_ACCUMULATOR_ALWAYS_NULL();
            SET_STATUS_FOR_MSG(AccumulatorAlwaysNull, OK);
        } else if (IsSubtype(accType, cachedType, GetTypeSystem())) {
            LOG_VERIFIER_REDUNDANT_IS_INSTANCE(ToString(accType), ToString(cachedType));
            SET_STATUS_FOR_MSG(RedundantIsInstance, OK);
        } else if (IsSubtype(cachedType, arrayType_, GetTypeSystem())) {
            auto eltType = cachedType.GetArrayElementType(GetTypeSystem());
            auto accEltType = accType.GetArrayElementType(GetTypeSystem());
            bool accEltTypeIsEmpty = accEltType.IsConsistent();
            if (!IsSubtype(accEltType, eltType, GetTypeSystem()) && !IsSubtype(eltType, accEltType, GetTypeSystem())) {
                if (accEltTypeIsEmpty) {
                    LOG_VERIFIER_IMPOSSIBLE_IS_INSTANCE(ToString(accType));
                    SET_STATUS_FOR_MSG(ImpossibleIsInstance, OK);
                } else {
                    LOG_VERIFIER_IMPOSSIBLE_ARRAY_IS_INSTANCE(ToString(accEltType));
                    SET_STATUS_FOR_MSG(ImpossibleArrayIsInstance, OK);
                }
            }
        } else if (TpIntersection(cachedType, accType, GetTypeSystem()) == bot_) {
            LOG_VERIFIER_IMPOSSIBLE_IS_INSTANCE(ToString(accType));
            SET_STATUS_FOR_MSG(ImpossibleIsInstance, OK);
        }  // else {
        // NOTE(vdyadov): here we may increase precision to concrete values in some cases
        SetAcc(i32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <typename NameGetter>
    bool CheckMethodArgs(NameGetter nameGetter, const PandaVector<Type> &formalArgs, Span<int> regs,
                         Type constructedType = Type {})
    {
        if (formalArgs.empty()) {
            return true;
        }

        size_t regsNeeded = !constructedType.IsNone() ? formalArgs.size() - 1 : formalArgs.size();
        if (regs.size() < regsNeeded) {
            return CheckMethodArgsTooFewParmeters<NameGetter>(nameGetter);
        }
        auto sigIter = formalArgs.cbegin();
        auto regsIter = regs.cbegin();

        for (size_t argnum = 0; argnum < formalArgs.size(); argnum++) {
            auto regNum = (!constructedType.IsNone() && sigIter == formalArgs.cbegin()) ? INVALID_REG : *(regsIter++);
            auto formalType = *(sigIter++);

            if (regNum != INVALID_REG && !IsRegDefined(regNum)) {
                LOG_VERIFIER_BAD_CALL_UNDEFINED_REGISTER(nameGetter(), regNum);
                SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
                return false;
            }
            Type actualType = regNum == INVALID_REG ? constructedType : GetRegType(regNum);
            // arg: NormalizedTypeOf(actual_type) <= norm_type
            // check of physical compatibility
            bool incompatibleTypes = false;
            if (CheckMethodArgsNotFit(formalType, actualType, regNum, incompatibleTypes)) {
                continue;
            }
            if (incompatibleTypes) {
                return CheckMethodArgsIncompatibleTypes<NameGetter>(nameGetter, regNum, actualType, formalType);
            }
            if (formalType == Type::Bot()) {
                return CheckMethodArgsBot<NameGetter>(nameGetter, actualType);
            }
            if (formalType == Type::Top()) {
                return CheckMethodArgsTop(actualType);
            }
            if (IsSubtype(formalType, primitive_, GetTypeSystem())) {
                if (!CheckMethodArgsSubtypePrimitive(nameGetter, formalType, actualType, regNum)) {
                    return false;
                }
                continue;
            }
            if (!CheckType(actualType, formalType)) {
                CheckMethodArgsCheckType<NameGetter>(nameGetter, actualType, formalType, regNum);
                if (!config->opts.debug.allow.wrongSubclassingInMethodArgs) {
                    status_ = VerificationStatus::ERROR;
                    return false;
                }
            }
        }
        return true;
    }

    bool CheckMethodAccessViolation(Method const *method)
    {
        auto *plugin = job_->JobPlugin();
        auto const *jobMethod = job_->JobMethod();
        auto result = plugin->CheckMethodAccessViolation(method, jobMethod, GetTypeSystem());
        if (!result.IsOk()) {
            const auto &verifOpts = config->opts;
            if (verifOpts.debug.allow.methodAccessViolation && result.IsError()) {
                result.status = VerificationStatus::WARNING;
            }
            LogInnerMessage(result);
            LOG_VERIFIER_DEBUG_CALL_FROM_TO(job_->JobMethod()->GetFullName(), method->GetFullName());
            status_ = result.status;
            if (status_ == VerificationStatus::ERROR) {
                return false;
            }
        }
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckCall(Method const *method, Span<int> regs)
    {
        if (method == nullptr) {
            SET_STATUS_FOR_MSG(CannotResolveMethodId, OK);
            return false;
        }

        if (!CheckMethodAccessViolation(method)) {
            return false;
        }

        if (!debugCtx->SkipVerificationOfCall(method->GetUniqId())) {
            auto methodNameGetter = [method]() { return method->GetFullName(); };
            auto formalArgs = GetTypeSystem()->GetMethodSignature(method)->args;
            if (!CheckMethodArgs(methodNameGetter, formalArgs, regs, {})) {
                return false;
            }
        }

        SetAcc(GetTypeSystem()->GetMethodSignature(method)->result);

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        if (method != nullptr && method->IsAbstract()) {
            LOG_VERIFIER_BAD_CALL_STATICALLY_ABSTRACT_METHOD(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticallyAbstractMethod, WARNING);
            return false;
        }

        Sync();
        std::array<int, 2UL> regs {vs1, vs2};
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallAccShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        auto accPos = static_cast<unsigned>(inst_.GetImm<FORMAT, 0x00>());
        static constexpr auto NUM_ARGS = 2;
        if (accPos >= NUM_ARGS) {
            LOG_VERIFIER_ACCUMULATOR_POSITION_IS_OUT_OF_RANGE();
            SET_STATUS_FOR_MSG(AccumulatorPositionIsOutOfRange, WARNING);
            return status_ != VerificationStatus::ERROR;
        }
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        if (method != nullptr && method->IsAbstract()) {
            LOG_VERIFIER_BAD_CALL_STATICALLY_ABSTRACT_METHOD(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticallyAbstractMethod, WARNING);
            return false;
        }

        Sync();
        std::array<int, NUM_ARGS> regs {};
        if (accPos == 0) {
            regs = {ACC, vs1};
        } else {
            regs = {vs1, ACC};
        }
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCalliDynShort();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCalliDyn();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCalliDynRange();

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCall()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        uint16_t vs3 = inst_.GetVReg<FORMAT, 0x02>();
        uint16_t vs4 = inst_.GetVReg<FORMAT, 0x03>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        if (method != nullptr && method->IsAbstract()) {
            LOG_VERIFIER_BAD_CALL_STATICALLY_ABSTRACT_METHOD(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticallyAbstractMethod, WARNING);
            return false;
        }

        Sync();
        std::array<int, 4UL> regs {vs1, vs2, vs3, vs4};
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallAcc()
    {
        LOG_INST();
        DBGBRK();
        auto accPos = static_cast<unsigned>(inst_.GetImm<FORMAT, 0x0>());
        static constexpr auto NUM_ARGS = 4;
        if (accPos >= NUM_ARGS) {
            LOG_VERIFIER_ACCUMULATOR_POSITION_IS_OUT_OF_RANGE();
            SET_STATUS_FOR_MSG(AccumulatorPositionIsOutOfRange, WARNING);
            return status_ != VerificationStatus::ERROR;
        }
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        if (method != nullptr && method->IsAbstract()) {
            LOG_VERIFIER_BAD_CALL_STATICALLY_ABSTRACT_METHOD(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticallyAbstractMethod, WARNING);
            return false;
        }

        Sync();
        std::array<int, NUM_ARGS> regs {};
        auto regIdx = 0;
        for (unsigned i = 0; i < NUM_ARGS; ++i) {
            if (i == accPos) {
                regs[i] = ACC;
            } else {
                regs[i] = static_cast<int>(inst_.GetVReg(regIdx++));
            }
        }
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallRange()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x00>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }

        if (method != nullptr && method->IsAbstract()) {
            LOG_VERIFIER_BAD_CALL_STATICALLY_ABSTRACT_METHOD(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticallyAbstractMethod, WARNING);
            return false;
        }

        Sync();
        std::vector<int> regs;
        for (auto regIdx = vs; ExecCtx().CurrentRegContext().IsRegDefined(regIdx); regIdx++) {
            regs.push_back(regIdx);
        }
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallVirtShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }
        if (method != nullptr && method->IsStatic()) {
            LOG_VERIFIER_BAD_CALL_STATIC_METHOD_AS_VIRTUAL(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticMethodAsVirtual, WARNING);
            return false;
        }

        Sync();
        std::array<int, 2UL> regs {vs1, vs2};
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallVirtAccShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        auto accPos = static_cast<unsigned>(inst_.GetImm<FORMAT, 0x00>());
        static constexpr auto NUM_ARGS = 2;
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }
        if (method != nullptr && method->IsStatic()) {
            LOG_VERIFIER_BAD_CALL_STATIC_METHOD_AS_VIRTUAL(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticMethodAsVirtual, WARNING);
            return false;
        }
        Sync();
        std::array<int, NUM_ARGS> regs {};
        if (accPos == 0) {
            regs = {ACC, vs1};
        } else {
            regs = {vs1, ACC};
        }
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallVirt()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        uint16_t vs3 = inst_.GetVReg<FORMAT, 0x02>();
        uint16_t vs4 = inst_.GetVReg<FORMAT, 0x03>();
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }
        if (method != nullptr && method->IsStatic()) {
            LOG_VERIFIER_BAD_CALL_STATIC_METHOD_AS_VIRTUAL(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticMethodAsVirtual, WARNING);
            return false;
        }

        Sync();
        std::array<int, 4UL> regs {vs1, vs2, vs3, vs4};
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallVirtAcc()
    {
        LOG_INST();
        DBGBRK();
        auto accPos = static_cast<unsigned>(inst_.GetImm<FORMAT, 0x0>());
        static constexpr auto NUM_ARGS = 4;
        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }
        if (method != nullptr && method->IsStatic()) {
            LOG_VERIFIER_BAD_CALL_STATIC_METHOD_AS_VIRTUAL(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticMethodAsVirtual, WARNING);
            return false;
        }
        Sync();
        std::array<int, NUM_ARGS> regs {};
        auto regIdx = 0;
        for (unsigned i = 0; i < NUM_ARGS; ++i) {
            if (i == accPos) {
                regs[i] = ACC;
            } else {
                regs[i] = static_cast<int>(inst_.GetVReg(regIdx++));
            }
        }
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallVirtRange()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x00>();

        Method const *method = GetCachedMethod();
        if (method != nullptr) {
            LOG_VERIFIER_DEBUG_METHOD(method->GetFullName());
        }
        if (method != nullptr && method->IsStatic()) {
            LOG_VERIFIER_BAD_CALL_STATIC_METHOD_AS_VIRTUAL(method->GetFullName());
            SET_STATUS_FOR_MSG(BadCallStaticMethodAsVirtual, WARNING);
            return false;
        }

        Sync();
        std::vector<int> regs;
        for (auto regIdx = vs; ExecCtx().CurrentRegContext().IsRegDefined(regIdx); regIdx++) {
            regs.push_back(regIdx);
        }
        return CheckCall<FORMAT>(method, Span {regs});
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyCall0()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();

        if (!CheckRegType(vs, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyCallRange()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegTypes({vs1, vs2}, {refType_, refType_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs1)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyCallShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegTypes({vs1, vs2}, {refType_, refType_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs1)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyCallThis0()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();

        if (!CheckRegType(vs, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyCallThisRange()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegTypes({vs1, vs2}, {refType_, refType_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs1)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyCallThisShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegTypes({vs1, vs2}, {refType_, refType_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs1)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyCallNew0()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        Sync();

        if (!CheckRegType(vs, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyCallNewRange()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegTypes({vs1, vs2}, {refType_, refType_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs1)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyCallNewShort()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegTypes({vs1, vs2}, {refType_, refType_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs1)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleThrow()
    {
        LOG_INST();
        DBGBRK();
        ExecCtx().SetCheckPoint(inst_.GetAddress());
        Sync();
        uint16_t vs = inst_.GetVReg<FORMAT>();
        if (!CheckRegType(vs, GetTypeSystem()->Throwable())) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        // possible implementation:
        // on stage of building checkpoints:
        // - add all catch block starts as checkpoints/entries
        // on absint stage:
        // - find corresponding catch block/checkpoint/entry
        // - add context to checkpoint/entry
        // - add entry to entry list
        // - stop absint
        return false;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMonitorenter()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        Type accType = GetAccType();
        if (accType == nullRefType_) {
            // NOTE(vdyadov): redesign next code, after support exception handlers,
            //                treat it as always throw NPE
            SHOW_MSG(AlwaysNpeAccumulator)
            LOG_VERIFIER_ALWAYS_NPE_ACCUMULATOR();
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(AlwaysNpeAccumulator, OK);
            return false;
        }
        if (!CheckType(accType, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleMonitorexit()
    {
        LOG_INST();
        DBGBRK();
        Sync();
        Type accType = GetAccType();
        if (accType == nullRefType_) {
            // NOTE(vdyadov): redesign next code, after support exception handlers,
            //                treat it as always throw NPE
            SHOW_MSG(AlwaysNpeAccumulator)
            LOG_VERIFIER_ALWAYS_NPE_ACCUMULATOR();
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(AlwaysNpeAccumulator, OK);
            return false;
        }
        if (!CheckType(accType, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        MoveToNextInst<FORMAT>();
        return true;
    }

    BytecodeInstructionSafe GetInst()
    {
        return inst_;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleCallspecRange()
    {
        // CallspecRange has the same semantics as CallRange in terms of
        // input and output for verification
        return HandleCallRange<FORMAT>();
    }

    static PandaString RegisterName(int regIdx, bool capitalize = false)
    {
        if (regIdx == ACC) {
            return capitalize ? "Accumulator" : "accumulator";
        }
        return PandaString {capitalize ? "Register v" : "register v"} + NumToStr(regIdx);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyLdbyname()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x00>();
        Sync();

        if (!CheckRegType(vs, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyLdbynameV()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vd = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegType(vs, refType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs)) {
            SetAcc(top_);
            return false;
        }

        SetReg(vd, objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyStbyname()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x00>();
        Sync();

        if (!CheckRegTypes({vs, ACC}, {refType_, refType_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs)) {
            SetAcc(top_);
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyStbynameV()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegTypes({vs1, vs2}, {refType_, refType_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs1)) {
            SetAcc(top_);
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyLdbyidx()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x00>();
        Sync();

        if (!CheckRegTypes({vs, ACC}, {refType_, f64_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyStbyidx()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegTypes({vs1, vs2, ACC}, {refType_, f64_, refType_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs1)) {
            SetAcc(top_);
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyLdbyval()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegTypes({vs1, vs2}, {refType_, refType_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs1)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(objectType_);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyStbyval()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs2 = inst_.GetVReg<FORMAT, 0x01>();
        Sync();

        if (!CheckRegTypes({vs1, vs2, ACC}, {refType_, refType_, refType_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs1)) {
            SetAcc(top_);
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleAnyIsinstance()
    {
        LOG_INST();
        DBGBRK();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x00>();
        Sync();

        if (!CheckRegTypes({vs, ACC}, {refType_, refType_})) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegTypeNotNullRef(vs)) {
            SetAcc(top_);
            return false;
        }

        SetAcc(i32_);
        MoveToNextInst<FORMAT>();
        return true;
    }

private:
    Type GetCachedType() const
    {
        auto offset = inst_.GetOffset();
        if (!job_->IsTypePresentForOffset(offset)) {
            SHOW_MSG(CacheMissForClassAtOffset)
            LOG_VERIFIER_CACHE_MISS_FOR_CLASS_AT_OFFSET(offset);
            END_SHOW_MSG();

            SHOW_MSG(CannotResolveClassId)
            LOG_VERIFIER_CANNOT_RESOLVE_CLASS_ID(inst_.GetId().AsFileId().GetOffset());
            END_SHOW_MSG();
            return Type {};
        }
        return job_->GetCachedType(offset);
    }

    Method const *GetCachedMethod() const
    {
        auto offset = inst_.GetOffset();
        if (!job_->IsMethodPresentForOffset(offset)) {
            SHOW_MSG(CacheMissForMethodAtOffset)
            LOG_VERIFIER_CACHE_MISS_FOR_METHOD_AT_OFFSET(offset);
            END_SHOW_MSG();

            SHOW_MSG(CannotResolveMethodId)
            LOG_VERIFIER_CANNOT_RESOLVE_METHOD_ID(inst_.GetId().AsFileId().GetOffset());
            END_SHOW_MSG();
            return nullptr;
        }
        return job_->GetCachedMethod(offset);
    }

    Field const *GetCachedField() const
    {
        auto offset = inst_.GetOffset();
        if (!job_->IsFieldPresentForOffset(offset)) {
            SHOW_MSG(CacheMissForFieldAtOffset)
            LOG_VERIFIER_CACHE_MISS_FOR_FIELD_AT_OFFSET(offset);
            END_SHOW_MSG();

            SHOW_MSG(CannotResolveFieldId)
            LOG_VERIFIER_CANNOT_RESOLVE_FIELD_ID(inst_.GetId().AsFileId().GetOffset());
            END_SHOW_MSG();
            return nullptr;
        }
        return job_->GetCachedField(offset);
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    void MoveToNextInst()
    {
        inst_ = inst_.GetNext<FORMAT>();
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckArrayStore(int v1, int v2, Type expectedEltType)
    {
        /*
        main rules:
        1. instruction should be strict in array element size, so for primitive types type equality is used
        2. accumulator may be subtype of array element type (under question)
        */
        if (!CheckRegType(v2, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegType(v1, arrayType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        Type regType = GetRegType(v1);
        if (regType == nullRefType_) {
            // NOTE(vdyadov): redesign next code, after support exception handlers,
            //                treat it as always throw NPE
            SHOW_MSG(AlwaysNpe)
            LOG_VERIFIER_ALWAYS_NPE(v1);
            END_SHOW_MSG();
            SetAcc(top_);
            SET_STATUS_FOR_MSG(AlwaysNpe, OK);
            return false;
        }

        auto arrEltType = regType.GetArrayElementType(GetTypeSystem());
        if (!IsSubtype(arrEltType, expectedEltType, GetTypeSystem())) {
            SHOW_MSG(BadArrayElementType2)
            LOG_VERIFIER_BAD_ARRAY_ELEMENT_TYPE2(ToString(arrEltType), ToString(expectedEltType));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(BadArrayElementType2, WARNING);
            return false;
        }

        Type accType = GetAccType();

        // NOTE(dvyadov): think of subtyping here. Can we really write more precise type into array?
        // since there is no problems with storage (all refs are of the same size)
        // and no problems with information losses, it seems fine at first sight.
        bool res = !IsSubtype(accType, arrEltType, GetTypeSystem());
        if (res) {
            // accumulator is of wrong type
            SHOW_MSG(BadAccumulatorType)
            LOG_VERIFIER_BAD_ACCUMULATOR_TYPE(ToString(accType), ToString(arrEltType));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(BadAccumulatorType, WARNING);
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckArrayStoreExact(int v1, int v2, Type accSupertype, std::initializer_list<Type> const &expectedEltTypes)
    {
        if (!CheckRegType(v2, integral32_) || !CheckRegType(v1, arrayType_) || !IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        Type regType = GetRegType(v1);
        if (regType == nullRefType_) {
            SHOW_MSG(AlwaysNpe)
            LOG_VERIFIER_ALWAYS_NPE(v1);
            END_SHOW_MSG();
            SetAcc(top_);
            SET_STATUS_FOR_MSG(AlwaysNpe, OK);
            return false;
        }

        auto arrEltType = regType.GetArrayElementType(GetTypeSystem());

        auto find = [&expectedEltTypes](auto type) {
            // CC-OFFNXT(G.FMT.12) false positive
            return std::find(expectedEltTypes.begin(), expectedEltTypes.end(), type) != expectedEltTypes.end();
        };
        if (!find(arrEltType)) {
            // array elt type is not expected one
            PandaVector<Type> expectedTypesVec;
            for (auto et : expectedEltTypes) {
                expectedTypesVec.push_back(et);
            }
            LOG_VERIFIER_BAD_ARRAY_ELEMENT_TYPE3(ToString(arrEltType), ToString(expectedTypesVec));
            SET_STATUS_FOR_MSG(BadArrayElementType, WARNING);
            return false;
        }

        Type accType = GetAccType();
        if (!IsSubtype(accType, accSupertype, GetTypeSystem())) {
            LOG_VERIFIER_BAD_ACCUMULATOR_TYPE2(ToString(accType));
            SET_STATUS_FOR_MSG(BadArrayElementType, WARNING);
            return false;
        }

        if (!find(accType)) {
            // array elt type is not expected one
            PandaVector<Type> expectedTypesVec;
            expectedTypesVec.insert(expectedTypesVec.end(), expectedEltTypes.begin(), expectedEltTypes.end());
            LOG_VERIFIER_BAD_ACCUMULATOR_TYPE3(ToString(accType), ToString(expectedTypesVec));
            if (status_ != VerificationStatus::ERROR) {
                status_ = VerificationStatus::WARNING;
            }
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT, bool REG_DST = false>
    bool CheckBinaryOp2(Type accIn, Type regIn, Type out)
    {
        uint16_t vs;
        if constexpr (REG_DST) {
            vs = inst_.GetVReg<FORMAT, 0x01>();
        } else {
            vs = inst_.GetVReg<FORMAT>();
        }
        if (!CheckRegType(ACC, accIn)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegType(vs, regIn)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if constexpr (REG_DST) {
            SetReg(inst_.GetVReg<FORMAT, 0x00>(), out);
        } else {
            SetAcc(out);
        }
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT, bool REG_DST = false>
    bool CheckBinaryOp2(Type accIn, Type regIn)
    {
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        return CheckBinaryOp2<FORMAT, REG_DST>(accIn, regIn, GetAccType());
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckBinaryOpImm(Type in, Type out)
    {
        uint16_t vd = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t vs = inst_.GetVReg<FORMAT, 0x01>();
        if (!CheckRegType(vs, in)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetReg(vd, out);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckBinaryOp2Imm(Type accIn, Type accOut)
    {
        if (!CheckRegType(ACC, accIn)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetAcc(accOut);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckBinaryOp2Imm(Type accIn)
    {
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        return CheckBinaryOp2Imm<FORMAT>(accIn, GetAccType());
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckUnaryOp(Type accIn, Type accOut)
    {
        if (!CheckRegType(ACC, accIn)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetAcc(accOut);
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckUnaryOp(Type accIn)
    {
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        return CheckUnaryOp<FORMAT>(accIn, GetAccType());
    }

    template <BytecodeInstructionSafe::Format FORMAT, bool REG_DST = false>
    bool CheckBinaryOp(Type v1In, Type v2In, Type out)
    {
        uint16_t v1 = inst_.GetVReg<FORMAT, 0x00>();
        uint16_t v2 = inst_.GetVReg<FORMAT, 0x01>();
        if (!CheckRegType(v1, v1In)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegType(v2, v2In)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if constexpr (REG_DST) {
            SetReg(v1, out);
        } else {
            SetAcc(out);
        }
        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckBinaryOp(Type vs1In, Type vs2In)
    {
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        return CheckBinaryOp<FORMAT>(vs1In, vs2In, GetAccType());
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool HandleConversion(Type from, Type to)
    {
        if (!CheckRegType(ACC, from)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        SetAcc(to);
        MoveToNextInst<FORMAT>();
        return true;
    }

    bool IsConcreteArrayType(Type type)
    {
        return IsSubtype(type, arrayType_, GetTypeSystem()) && type != arrayType_;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckArrayLoad(int vs, std::initializer_list<Type> const &expectedEltTypes)
    {
        if (!CheckRegType(ACC, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }
        if (!CheckRegType(vs, arrayType_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        Type regType = GetRegType(vs);
        if (regType == nullRefType_) {
            // NOTE(vdyadov): redesign next code, after support exception handlers,
            //                treat it as always throw NPE
            SHOW_MSG(AlwaysNpe)
            LOG_VERIFIER_ALWAYS_NPE(vs);
            END_SHOW_MSG();
            SetAcc(top_);
            SET_STATUS_FOR_MSG(AlwaysNpe, OK);
            return false;
        }
        auto &&arrEltType = regType.GetArrayElementType(GetTypeSystem());
        auto find = [&expectedEltTypes](auto type) {
            for (Type t : expectedEltTypes) {
                if (type == t) {
                    return true;
                }
            }
            return false;
        };
        auto res = find(arrEltType);
        if (!res) {
            PandaVector<Type> expectedTypesVec;
            for (auto et : expectedEltTypes) {
                expectedTypesVec.push_back(et);
            }
            LOG_VERIFIER_BAD_ARRAY_ELEMENT_TYPE3(ToString(arrEltType), ToString(expectedTypesVec));
            SET_STATUS_FOR_MSG(BadArrayElementType, WARNING);
            return false;
        }
        SetAcc(arrEltType);
        MoveToNextInst<FORMAT>();
        return true;
    }

    bool ProcessBranching(int32_t offset)
    {
        auto newInst = inst_.JumpTo(offset);
        const uint8_t *target = newInst.GetAddress();
        if (!context_.CflowInfo().IsAddrValid(target) ||
            !context_.CflowInfo().IsFlagSet(target, CflowMethodInfo::INSTRUCTION)) {
            LOG_VERIFIER_INCORRECT_JUMP();
            status_ = VerificationStatus::ERROR;
            return false;
        }

#ifndef NDEBUG
        ExecCtx().ProcessJump(
            inst_.GetAddress(), target,
            [this, printHdr = true](int regIdx, const auto &srcReg, const auto &dstReg) mutable {
                if (printHdr) {
                    LOG_VERIFIER_REGISTER_CONFLICT_HEADER();
                    printHdr = false;
                }
                LOG_VERIFIER_REGISTER_CONFLICT(RegisterName(regIdx), ToString(srcReg.GetAbstractType()),
                                               ToString(dstReg.GetAbstractType()));
                return true;
            },
            codeType_);
#else
        ExecCtx().ProcessJump(inst_.GetAddress(), target, codeType_);
#endif
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT, template <typename OpT> class Op>
    bool HandleCondJmpz()
    {
        auto imm = inst_.GetImm<FORMAT>();

        if (!CheckRegType(ACC, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!ProcessBranching(imm)) {
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT, template <typename OpT> class Op>
    bool HandleCondJmp()
    {
        auto imm = inst_.GetImm<FORMAT>();
        uint16_t vs = inst_.GetVReg<FORMAT>();

        if (!CheckRegType(ACC, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!CheckRegType(vs, integral32_)) {
            SET_STATUS_FOR_MSG(BadRegisterType, WARNING);
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        if (!ProcessBranching(imm)) {
            return false;
        }

        MoveToNextInst<FORMAT>();
        return true;
    }

    template <BytecodeInstructionSafe::Format FORMAT>
    bool CheckArrayCtor(Type klass, Span<int> regNums)
    {
        if (!klass.IsConsistent() || !klass.IsClass() || !klass.GetClass()->IsArrayClass()) {
            return false;
        }
        auto argsNum = GetArrayNumDimensions(klass.GetClass());
        bool result = false;
        for (auto reg : regNums) {
            if (!IsRegDefined(reg)) {
                SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
                result = false;
                break;
            }
            result = CheckRegType(reg, i32_);
            if (!result) {
                LOG(ERROR, VERIFIER) << "Verifier error: ArrayCtor type error";
                status_ = VerificationStatus::ERROR;
                break;
            }
            --argsNum;
            if (argsNum == 0) {
                break;
            }
        };
        if (result && argsNum > 0) {
            SHOW_MSG(TooFewArrayConstructorArgs)
            LOG_VERIFIER_TOO_FEW_ARRAY_CONSTRUCTOR_ARGS(argsNum);
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(TooFewArrayConstructorArgs, WARNING);
            result = false;
        }
        if (result) {
            SetAcc(klass);
            MoveToNextInst<FORMAT>();
        }
        return result;
    }

    void LogInnerMessage(const CheckResult &elt)
    {
        if (elt.IsError()) {
            LOG(ERROR, VERIFIER) << "Error: " << elt.msg;
        } else if (elt.IsWarning()) {
            LOG(WARNING, VERIFIER) << "Warning: " << elt.msg;
        }
    }

    size_t GetArrayNumDimensions(Class const *klass)
    {
        size_t res = 0;
        while (klass->IsArrayClass()) {
            res++;
            klass = klass->GetComponentType();
        }
        return res;
    }

    bool CheckFieldAccessStaticVolatile(bool isStatic, bool isVolatile, Field const *&field)
    {
        if (isStatic != field->IsStatic()) {
            SHOW_MSG(ExpectedStaticOrInstanceField)
            LOG_VERIFIER_EXPECTED_STATIC_OR_INSTANCE_FIELD(isStatic);
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(ExpectedStaticOrInstanceField, WARNING);
            return false;
        }

        if (isVolatile != field->IsVolatile()) {
            // if the inst is volatile but the field is not
            if (isVolatile) {
                SHOW_MSG(ExpectedVolatileField)
                LOG_VERIFIER_EXPECTED_VOLATILE_FIELD();
                END_SHOW_MSG();
                SET_STATUS_FOR_MSG(ExpectedVolatileField, WARNING);
                return false;
            }
            // if the instruction is not volatile but the field is
            SHOW_MSG(ExpectedInstanceField)
            LOG_VERIFIER_EXPECTED_INSTANCE_FIELD();
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(ExpectedInstanceField, ERROR);
            return false;
        }

        return true;
    }

    bool CheckFieldAccessPlugin(Field const *&field)
    {
        auto *plugin = job_->JobPlugin();
        auto const *jobMethod = job_->JobMethod();
        auto result = plugin->CheckFieldAccessViolation(field, jobMethod, GetTypeSystem());
        if (!result.IsOk()) {
            const auto &verifOpts = config->opts;
            if (verifOpts.debug.allow.fieldAccessViolation && result.IsError()) {
                result.status = VerificationStatus::WARNING;
            }
            LogInnerMessage(result);
            LOG_VERIFIER_DEBUG_FIELD2(GetFieldName(field));
            status_ = result.status;
            return status_ != VerificationStatus::ERROR;
        }

        return !result.IsError();
    }

    bool CheckCastArrayObjectRegDef(Type &cachedType)
    {
        if (!cachedType.IsConsistent()) {
            return false;
        }
        LOG_VERIFIER_DEBUG_TYPE(ToString(cachedType));
        if (!IsSubtype(cachedType, objectType_, GetTypeSystem()) &&
            !IsSubtype(cachedType, arrayType_, GetTypeSystem())) {
            LOG_VERIFIER_CHECK_CAST_TO_NON_OBJECT_TYPE(ToString(cachedType));
            SET_STATUS_FOR_MSG(CheckCastToNonObjectType, WARNING);
            return false;
        }
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        return true;
    }

    bool CheckCastRefArrayType(Type &accType)
    {
        if (!IsSubtype(accType, refType_, GetTypeSystem()) && !IsSubtype(accType, arrayType_, GetTypeSystem())) {
            LOG_VERIFIER_NON_OBJECT_ACCUMULATOR_TYPE();
            SET_STATUS_FOR_MSG(NonObjectAccumulatorType, WARNING);
            return false;
        }

        return true;
    }

    bool CheckInstanceConsistentArrayObjectRegDef(Type &cachedType)
    {
        if (!cachedType.IsConsistent()) {
            return false;
        }
        LOG_VERIFIER_DEBUG_TYPE(ToString(cachedType));
        if (!IsSubtype(cachedType, objectType_, GetTypeSystem()) &&
            !IsSubtype(cachedType, arrayType_, GetTypeSystem())) {
            // !(type <= Types().ArrayType()) is redundant, because all arrays
            // are subtypes of either panda.Object <: ObjectType or java.lang.Object <: ObjectType
            // depending on selected language context
            LOG_VERIFIER_BAD_IS_INSTANCE_INSTRUCTION(ToString(cachedType));
            SET_STATUS_FOR_MSG(BadIsInstanceInstruction, WARNING);
            return false;
        }
        if (!IsRegDefined(ACC)) {
            SET_STATUS_FOR_MSG(UndefinedRegister, WARNING);
            return false;
        }

        return true;
    }

    bool CheckInstanceClass(Type &cachedType, const plugin::Plugin *&plugin, Method const *&jobMethod)
    {
        auto result = CheckResult::ok;
        if (cachedType.IsClass()) {
            result = plugin->CheckClassAccessViolation(cachedType.GetClass(), jobMethod, GetTypeSystem());
        }
        if (!result.IsOk()) {
            LogInnerMessage(CheckResult::protected_class);
            LOG_VERIFIER_DEBUG_CALL_FROM_TO(job_->JobMethod()->GetClass()->GetName(), ToString(cachedType));
            status_ = VerificationStatus::ERROR;
            return false;
        }

        return true;
    }

    template <typename NameGetter>
    bool CheckMethodArgsSubtypePrimitive(NameGetter &nameGetter, Type &formalType, Type &actualType, int regNum)
    {
        // check implicit conversion of primitive types
        TypeId formalId = formalType.ToTypeId();
        CheckResult checkResult = CheckResult::ok;

        if (!IsSubtype(actualType, primitive_, GetTypeSystem())) {
            return false;
        }
        // !!!!!! NOTE: need to check all possible TypeId-s against formal_id
        TypeId actualId = actualType.ToTypeId();
        if (actualId != TypeId::INVALID) {
            checkResult = ark::verifier::CheckMethodArgs(formalId, actualId);
        } else {
            // special case, where type after contexts LUB operation is inexact one, like
            // integral32_Type()
            if ((IsSubtype(formalType, integral32_, GetTypeSystem()) &&
                 IsSubtype(actualType, integral32_, GetTypeSystem())) ||
                (IsSubtype(formalType, integral64_, GetTypeSystem()) &&
                 IsSubtype(actualType, integral64_, GetTypeSystem())) ||
                (IsSubtype(formalType, float64_, GetTypeSystem()) &&
                 IsSubtype(actualType, float64_, GetTypeSystem()))) {
                SHOW_MSG(CallFormalActualDifferent)
                LOG_VERIFIER_CALL_FORMAL_ACTUAL_DIFFERENT(ToString(formalType), ToString(actualType));
                END_SHOW_MSG();
            } else {
                checkResult = ark::verifier::CheckMethodArgs(formalId, actualId);
            }
        }
        if (!checkResult.IsOk()) {
            SHOW_MSG(DebugCallParameterTypes)
            LogInnerMessage(checkResult);
            LOG_VERIFIER_DEBUG_CALL_PARAMETER_TYPES(
                nameGetter(),
                (regNum == INVALID_REG ? "" : PandaString {"Actual parameter in "} + RegisterName(regNum) + ". "),
                ToString(actualType), ToString(formalType));
            END_SHOW_MSG();
            status_ = checkResult.status;
            if (status_ == VerificationStatus::ERROR) {
                return false;
            }
        }

        return true;
    }

    template <typename NameGetter>
    bool CheckMethodArgsIncompatibleTypes(NameGetter &nameGetter, int regNum, Type &actualType, Type &formalType)
    {
        auto const normType = GetTypeSystem()->NormalizedTypeOf(formalType);
        Type normActualType = GetTypeSystem()->NormalizedTypeOf(actualType);

        PandaString regOrParam = regNum == INVALID_REG ? "Actual parameter" : RegisterName(regNum, true);
        SHOW_MSG(BadCallIncompatibleParameter)
        LOG_VERIFIER_BAD_CALL_INCOMPATIBLE_PARAMETER(nameGetter(), regOrParam, ToString(normActualType),
                                                     ToString(normType));
        END_SHOW_MSG();
        SET_STATUS_FOR_MSG(BadCallIncompatibleParameter, WARNING);
        return false;
    }

    template <typename NameGetter>
    bool CheckMethodArgsBot(NameGetter &nameGetter, Type &actualType)
    {
        if (actualType == Type::Bot()) {
            LOG_VERIFIER_CALL_FORMAL_ACTUAL_BOTH_BOT_OR_TOP("Bot");
            return true;
        }

        SHOW_MSG(BadCallFormalIsBot)
        LOG_VERIFIER_BAD_CALL_FORMAL_IS_BOT(nameGetter(), ToString(actualType));
        END_SHOW_MSG();
        SET_STATUS_FOR_MSG(BadCallFormalIsBot, WARNING);
        return false;
    }

    bool CheckMethodArgsTop(Type &actualType)
    {
        if (actualType == Type::Top()) {
            LOG_VERIFIER_CALL_FORMAL_ACTUAL_BOTH_BOT_OR_TOP("Top");
            return true;
        }
        SHOW_MSG(CallFormalTop)
        LOG_VERIFIER_CALL_FORMAL_TOP();
        END_SHOW_MSG();
        return true;
    }

    template <typename NameGetter>
    void CheckMethodArgsCheckType(NameGetter &nameGetter, Type &actualType, Type &formalType, int regNum)
    {
        if (regNum == INVALID_REG) {
            SHOW_MSG(BadCallWrongParameter)
            LOG_VERIFIER_BAD_CALL_WRONG_PARAMETER(nameGetter(), ToString(actualType), ToString(formalType));
            END_SHOW_MSG();
            SET_STATUS_FOR_MSG(BadCallWrongParameter, WARNING);
            return;
        }
        SHOW_MSG(BadCallWrongRegister)
        LOG_VERIFIER_BAD_CALL_WRONG_REGISTER(nameGetter(), regNum);
        END_SHOW_MSG();
        SET_STATUS_FOR_MSG(BadCallWrongRegister, WARNING);
    }

    template <typename NameGetter>
    bool CheckMethodArgsTooFewParmeters(NameGetter &nameGetter)
    {
        SHOW_MSG(BadCallTooFewParameters)
        LOG_VERIFIER_BAD_CALL_TOO_FEW_PARAMETERS(nameGetter());
        END_SHOW_MSG();
        SET_STATUS_FOR_MSG(BadCallTooFewParameters, WARNING);
        return false;
    }

    bool CheckMethodArgsNotFit(Type &formalType, Type &actualType, int regNum, bool &incompatibleTypes)
    {
        auto const normType = GetTypeSystem()->NormalizedTypeOf(formalType);
        Type normActualType = GetTypeSystem()->NormalizedTypeOf(actualType);

        if (regNum != INVALID_REG && IsSubtype(formalType, refType_, GetTypeSystem()) && formalType != Type::Bot() &&
            IsSubtype(actualType, refType_, GetTypeSystem())) {
            if (IsSubtype(actualType, formalType, GetTypeSystem())) {
                return true;
            }
            if (!config->opts.debug.allow.wrongSubclassingInMethodArgs) {
                incompatibleTypes = true;
            }
        } else if (formalType != Type::Bot() && formalType != Type::Top() &&
                   !IsSubtype(normActualType, normType, GetTypeSystem())) {
            incompatibleTypes = true;
        }

        return false;
    }

private:
    BytecodeInstructionSafe inst_;
    VerificationContext &context_;
    VerificationStatus status_ {VerificationStatus::OK};
    // #ifndef NDEBUG
    bool debug_ {false};
    uint32_t debugOffset_ {0};
    // #endif
    EntryPointType codeType_;

    void SetStatusAtLeast(VerificationStatus newStatus)
    {
        status_ = std::max(status_, newStatus);
    }

    static inline VerificationStatus MsgClassToStatus(MethodOption::MsgClass msgClass)
    {
        switch (msgClass) {
            case MethodOption::MsgClass::HIDDEN:
                return VerificationStatus::OK;
            case MethodOption::MsgClass::WARNING:
                return VerificationStatus::WARNING;
            case MethodOption::MsgClass::ERROR:
                return VerificationStatus::ERROR;
            default:
                UNREACHABLE();
        }
    }

    static PandaString GetFieldName(Field const *field)
    {
        return PandaString {field->GetClass()->GetName()} + "." + utf::Mutf8AsCString(field->GetName().data);
    }
};
}  // namespace ark::verifier

#endif  // PANDA_VERIFICATION_ABSINT_ABS_INT_INL_H
