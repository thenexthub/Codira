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

#include "absint.h"
#include "exec_context.h"
#include "libarkfile/file_items.h"
#include "include/thread_scopes.h"
#include "plugins.h"
#include "verification_context.h"

#include "verification/jobs/job.h"

#include "value/abstract_typed_value.h"
#include "type/type_system.h"

#include "cflow/cflow_info.h"

#include "runtime/include/method.h"
#include "runtime/include/method-inl.h"

#include "libarkbase/macros.h"

#include <locale>
#include <optional>

#include "abs_int_inl.h"
#include "handle_gen.h"

#include "util/str.h"
#include "util/hash.h"

#include <utility>
#include <tuple>
#include <functional>

namespace ark::verifier {

#include "abs_int_inl_gen.h"

}  // namespace ark::verifier

namespace ark::verifier {

using TryBlock = panda_file::CodeDataAccessor::TryBlock;
using CatchBlock = panda_file::CodeDataAccessor::CatchBlock;

VerificationContext PrepareVerificationContext(TypeSystem *typeSystem, Job const *job)
{
    auto *method = job->JobMethod();

    auto *klass = method->GetClass();

    Type methodClassType {klass};

    VerificationContext verifCtx {typeSystem, job, methodClassType};
    // NOTE(vdyadov): ASSERT(cflow_info corresponds method)

    auto &cflowInfo = verifCtx.CflowInfo();
    auto &execCtx = verifCtx.ExecCtx();

    LOG_VERIFIER_DEBUG_METHOD_VERIFICATION(method->GetFullName());

    /*
    1. build initial regContext for the method entry
    */
    RegContext &regCtx = verifCtx.ExecCtx().CurrentRegContext();
    regCtx.Clear();

    auto numVregs = method->GetNumVregs();
    regCtx[numVregs] = AbstractTypedValue {};

    const auto &signature = typeSystem->GetMethodSignature(method);

    for (size_t idx = 0; idx < signature->args.size(); ++idx) {
        Type const &t = signature->args[idx];
        regCtx[numVregs++] = AbstractTypedValue {t, verifCtx.NewVar(), AbstractTypedValue::Start {}, idx};
    }
    LOG_VERIFIER_DEBUG_REGISTERS("registers =", regCtx.DumpRegs(typeSystem));

    verifCtx.SetReturnType(&signature->result);

    LOG_VERIFIER_DEBUG_RESULT(signature->result.ToString(typeSystem));

    /*
    3. Add checkpoint for exc. handlers
    */
    method->EnumerateTryBlocks([&](TryBlock const &tryBlock) {
        uint8_t const *code = method->GetInstructions();
        uint8_t const *tryStartPc = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(code) +
                                                                static_cast<uintptr_t>(tryBlock.GetStartPc()));
        uint8_t const *tryEndPc = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(tryStartPc) +
                                                              static_cast<uintptr_t>(tryBlock.GetLength()));
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (uint8_t const *pc = tryStartPc; pc < tryEndPc; pc++) {
            if (cflowInfo.IsFlagSet(pc, CflowMethodInfo::EXCEPTION_SOURCE)) {
                execCtx.SetCheckPoint(pc);
            }
        }
        return true;
    });

    /*
    3. add Start entry of method
    */

    const uint8_t *methodPcStartPtr = method->GetInstructions();

    verifCtx.ExecCtx().AddEntryPoint(methodPcStartPtr, EntryPointType::METHOD_BODY);
    verifCtx.ExecCtx().StoreCurrentRegContextForAddr(methodPcStartPtr);

    return verifCtx;
}

namespace {

VerificationStatus VerifyEntryPoints(VerificationContext &verifCtx, ExecContext &execCtx)
{
    const auto &debugOpts = GetServiceConfig(verifCtx.GetJob()->GetService())->opts.debug;

    /*
    Start main loop: get entry point with context, process, repeat
    */

    uint8_t const *entryPoint = nullptr;
    EntryPointType entryType;
    VerificationStatus worstSoFar = VerificationStatus::OK;

    while (execCtx.GetEntryPointForChecking(&entryPoint, &entryType) == ExecContext::Status::OK) {
#ifndef NDEBUG
        const void *codeStart = verifCtx.GetJob()->JobMethod()->GetInstructions();
        LOG_VERIFIER_DEBUG_CODE_BLOCK_VERIFICATION(
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entryPoint) - reinterpret_cast<uintptr_t>(codeStart)),
            (entryType == EntryPointType::METHOD_BODY ? "method body" : "exception handler"));
#endif
        auto result = AbstractInterpret(verifCtx, entryPoint, entryType);
        if (debugOpts.allow.errorInExceptionHandler && entryType == EntryPointType::EXCEPTION_HANDLER &&
            result == VerificationStatus::ERROR) {
            result = VerificationStatus::WARNING;
        }
        if (result == VerificationStatus::ERROR) {
            return result;
        }
        worstSoFar = std::max(worstSoFar, result);
    }

    return worstSoFar;
}

bool ComputeRegContext(Method const *method, TryBlock const *tryBlock, VerificationContext &verifCtx,
                       RegContext *regContext)
{
    auto &cflowInfo = verifCtx.CflowInfo();
    auto &execCtx = verifCtx.ExecCtx();
    auto *typeSystem = verifCtx.GetTypeSystem();
    auto start = reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(method->GetInstructions()) +
                                                   tryBlock->GetStartPc());
    auto end = reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(method->GetInstructions()) +
                                                 tryBlock->GetStartPc() + tryBlock->GetLength());

    LOG_VERIFIER_DEBUG_TRY_BLOCK_COMMON_CONTEXT_COMPUTATION(tryBlock->GetStartPc(),
                                                            tryBlock->GetStartPc() + tryBlock->GetLength());

    bool first = true;
    execCtx.ForContextsOnCheckPointsInRange(start, end, [&](const uint8_t *pc, const RegContext &ctx) {
        if (cflowInfo.IsFlagSet(pc, CflowMethodInfo::EXCEPTION_SOURCE)) {
            LOG_VERIFIER_DEBUG_REGISTERS("+", ctx.DumpRegs(typeSystem));
            if (first) {
                first = false;
                *regContext = ctx;
            } else {
                regContext->UnionWith(&ctx, typeSystem);
            }
        }
        return true;
    });
    LOG_VERIFIER_DEBUG_REGISTERS("=", regContext->DumpRegs(typeSystem));

    if (first) {
        // return false when the try block does not have instructions to throw exception
        return false;
    }

    regContext->RemoveInconsistentRegs();

#ifndef NDEBUG
    if (regContext->HasInconsistentRegs()) {
        LOG_VERIFIER_COMMON_CONTEXT_INCONSISTENT_REGISTER_HEADER();
        for (int regNum : regContext->InconsistentRegsNums()) {
            LOG(DEBUG, VERIFIER) << AbsIntInstructionHandler::RegisterName(regNum);
        }
    }
#endif

    return true;
}

VerificationStatus VerifyExcHandler([[maybe_unused]] TryBlock const *tryBlock, CatchBlock const *catchBlock,
                                    VerificationContext *verifCtx, RegContext *regContext)
{
    Method const *method = verifCtx->GetMethod();
    auto &execCtx = verifCtx->ExecCtx();
    auto exceptionIdx = catchBlock->GetTypeIdx();
    auto langCtx = LanguageContext(plugins::GetLanguageContextBase(method->GetClass()->GetSourceLang()));
    Class *exception = nullptr;
    if (exceptionIdx != panda_file::INVALID_INDEX) {
        ScopedChangeThreadStatus st {ManagedThread::GetCurrent(), ThreadStatus::RUNNING};
        exception = verifCtx->GetJob()->GetService()->classLinker->GetExtension(langCtx)->GetClass(
            *method->GetPandaFile(), method->GetClass()->ResolveClassIndex(exceptionIdx),
            method->GetClass()->GetLoadContext());
    }

    LOG(DEBUG, VERIFIER) << "Exception handler at " << std::hex << "0x" << catchBlock->GetHandlerPc()
                         << (exception != nullptr
                                 ? PandaString {", for exception '"} + PandaString {exception->GetName()} + "' "
                                 : PandaString {""})
                         << ", try block scope: [ "
                         << "0x" << tryBlock->GetStartPc() << ", "
                         << "0x" << (tryBlock->GetStartPc() + tryBlock->GetLength()) << " ]";

    Type exceptionType {};
    if (exception != nullptr) {
        exceptionType = Type {exception};
    } else {
        exceptionType = verifCtx->GetTypeSystem()->Throwable();
    }

    if (exceptionType.IsConsistent()) {
        const int acc = -1;
        (*regContext)[acc] = AbstractTypedValue {exceptionType, verifCtx->NewVar()};
    }

    auto startPc = reinterpret_cast<uint8_t const *>(
        reinterpret_cast<uintptr_t>(verifCtx->GetMethod()->GetInstructions()) + catchBlock->GetHandlerPc());

    execCtx.CurrentRegContext() = *regContext;
    execCtx.AddEntryPoint(startPc, EntryPointType::EXCEPTION_HANDLER);
    execCtx.StoreCurrentRegContextForAddr(startPc);

    return VerifyEntryPoints(*verifCtx, execCtx);
}

}  // namespace

VerificationStatus VerifyMethod(VerificationContext &verifCtx)
{
    VerificationStatus worstSoFar = VerificationStatus::OK;
    auto &execCtx = verifCtx.ExecCtx();

    worstSoFar = std::max(worstSoFar, VerifyEntryPoints(verifCtx, execCtx));
    if (worstSoFar == VerificationStatus::ERROR) {
        return worstSoFar;
    }

    // Need to have the try blocks sorted!
    verifCtx.GetMethod()->EnumerateTryBlocks([&](TryBlock &tryBlock) {
        bool tryBlockCanThrow = true;
        RegContext regContext;
        tryBlockCanThrow = ComputeRegContext(verifCtx.GetMethod(), &tryBlock, verifCtx, &regContext);
        if (!tryBlockCanThrow) {
            // catch block is unreachable
        } else {
            tryBlock.EnumerateCatchBlocks([&](CatchBlock const &catchBlock) {
                worstSoFar = std::max(worstSoFar, VerifyExcHandler(&tryBlock, &catchBlock, &verifCtx, &regContext));
                return (worstSoFar != VerificationStatus::ERROR);
            });
        }
        return true;
    });

    if (worstSoFar == VerificationStatus::ERROR) {
        return worstSoFar;
    }

    // NOTE(vdyadov): account for dead code
    const uint8_t *dummyEntryPoint;
    EntryPointType dummyEntryType;

    if (execCtx.GetEntryPointForChecking(&dummyEntryPoint, &dummyEntryType) ==
        ExecContext::Status::NO_ENTRY_POINTS_WITH_CONTEXT) {
        // NOTE(vdyadov): log remaining entry points as unreachable
        worstSoFar = std::max(worstSoFar, VerificationStatus::WARNING);
    }

    return worstSoFar;
}

}  // namespace ark::verifier
