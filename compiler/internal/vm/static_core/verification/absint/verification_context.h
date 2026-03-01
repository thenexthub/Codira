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

#ifndef PANDA_VERIFICATION_VERIFICATION_CONTEXT_HPP_
#define PANDA_VERIFICATION_VERIFICATION_CONTEXT_HPP_

#include "libarkbase/macros.h"
#include "runtime/include/method.h"
#include "runtime/include/method-inl.h"

#include "verification/absint/exec_context.h"
#include "verification/cflow/cflow_info.h"
#include "verification/jobs/job.h"
#include "verification/plugins.h"
#include "verification/type/type_system.h"
#include "verification/util/lazy.h"
#include "verification/util/callable.h"
#include "verification/value/variables.h"

#include <functional>

namespace ark::verifier {
using CallIntoRuntimeHandler = callable<void(callable<void()>)>;

class VerificationContext {
public:
    using Var = Variables::Var;

    VerificationContext(TypeSystem *typeSystem, Job const *job, Type methodClassType)
        : types_ {typeSystem},
          job_ {job},
          methodClassType_ {methodClassType},
          execCtx_ {CflowInfo().GetAddrStart(), CflowInfo().GetAddrEnd(), typeSystem},
          plugin_ {plugin::GetLanguagePlugin(job->JobMethod()->GetClass()->GetSourceLang())}
    {
        Method const *method = job->JobMethod();
        // set checkpoints for reg_context storage
        // start of method is checkpoint too
        ExecCtx().SetCheckPoint(CflowInfo().GetAddrStart());
        uint8_t const *start = CflowInfo().GetAddrStart();
        uint8_t const *end = CflowInfo().GetAddrEnd();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (uint8_t const *pc = start; pc < end; pc++) {
            if (CflowInfo().IsFlagSet(pc, CflowMethodInfo::JUMP_TARGET)) {
                ExecCtx().SetCheckPoint(pc);
            }
        }
        method->EnumerateCatchBlocks([&](uint8_t const *tryStart, uint8_t const *tryEnd,
                                         panda_file::CodeDataAccessor::CatchBlock const &catchBlock) {
            auto catchStart = reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(method->GetInstructions()) +
                                                                static_cast<uintptr_t>(catchBlock.GetHandlerPc()));
            ExecCtx().SetCheckPoint(tryStart);
            ExecCtx().SetCheckPoint(tryEnd);
            ExecCtx().SetCheckPoint(catchStart);
            return true;
        });
    }

    ~VerificationContext() = default;
    DEFAULT_MOVE_SEMANTIC(VerificationContext);
    DEFAULT_COPY_SEMANTIC(VerificationContext);

    Job const *GetJob() const
    {
        return job_;
    }

    const CflowMethodInfo &CflowInfo() const
    {
        return job_->JobMethodCflow();
    }

    Method const *GetMethod() const
    {
        return job_->JobMethod();
    }

    Type GetMethodClass() const
    {
        return methodClassType_;
    }

    ExecContext &ExecCtx()
    {
        return execCtx_;
    }

    const ExecContext &ExecCtx() const
    {
        return execCtx_;
    }

    TypeSystem *GetTypeSystem()
    {
        return types_;
    }

    Var NewVar()
    {
        return types_->NewVar();
    }

    Type ReturnType() const
    {
        return returnType_;
    }

    void SetReturnType(Type const *type)
    {
        returnType_ = *type;
    }

    plugin::Plugin const *GetPlugin()
    {
        return plugin_;
    }

private:
    TypeSystem *types_;
    Job const *job_;
    Type returnType_;
    Type methodClassType_;
    ExecContext execCtx_;
    plugin::Plugin const *plugin_;
};
}  // namespace ark::verifier

#endif  // !PANDA_VERIFICATION_VERIFICATION_CONTEXT_HPP_
