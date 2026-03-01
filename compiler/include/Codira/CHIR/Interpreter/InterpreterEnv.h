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

/**
 * @file
 *
 * This file declares an environment for the BCHIR interpreter.
 */

#ifndef CODIRA_CHIR_INTERRETER_INTERPRETERENV_H
#define CODIRA_CHIR_INTERRETER_INTERPRETERENV_H

#include "Codira/CHIR/Interpreter/BCHIR.h"
#include "Codira/CHIR/Interpreter/InterpreterValueUtils.h"

namespace Codira::CHIR::Interpreter {

struct Env {
    const static size_t LOCAL_ENV_DEFAULT_SIZE = 1024;
    Env(size_t sizeGlobalEnv) : numberOfGlobals(sizeGlobalEnv)
    {
        local.reserve(LOCAL_ENV_DEFAULT_SIZE);
        global.resize(numberOfGlobals, IInvalid());
    }

    void SetLocal(Bchir::VarIdx var, IVal&& node)
    {
        local[bp + var] = std::move(node);
    }

    void AllocateLocalVarsForFrame(size_t number)
    {
        // this should always be performed when entering the frame
        CODEC_ASSERT(local.size() == bp);
        local.resize(local.size() + number);
    }

    void SetGlobal(Bchir::VarIdx var, IVal&& node)
    {
        CODEC_ASSERT(var < global.size());
        global[var] = std::move(node);
    }

    const IVal& GetLocal(Bchir::VarIdx var)
    {
        CODEC_ASSERT(bp + var < local.size());
        CODEC_ASSERT(!std::holds_alternative<IInvalid>(local[bp + var]));
        return local[bp + var];
    }

    IVal& GetGlobal(Bchir::VarIdx var)
    {
        CODEC_ASSERT(var < global.size());
        // Global vars are initialized with IInvalid. global[var] can be IInvalid the first time we read it.
        return global[var];
    }

    const IVal& PeekGlobal(Bchir::VarIdx var) const
    {
        CODEC_ASSERT(var < global.size());
        // Global vars are initialized with IInvalid. global[var] can be IInvalid the first time we read it.
        return global[var];
    }

    void StartStackFrame()
    {
        bp = local.size();
    }

    /** @brief set base pointer to newBP and clean environment stack after bp. */
    void RestoreStackFrameTo(size_t newBP)
    {
        // we assume that the newBP is the preceeding stack frame of bp
        local.erase(local.begin() + static_cast<std::vector<Interpreter::IVal>::difference_type>(bp), local.end());
        bp = newBP;
    }

    size_t GetBP() const
    {
        return bp;
    }

private:
    size_t numberOfGlobals;
    /** @brief environment for global variables */
    std::vector<IVal> global;
    /** @brief environment for local variables */
    std::vector<IVal> local;
    /** @brief the base pointer in the local environment (an index of `local`) */
    size_t bp{0};
};

} // namespace Codira::CHIR::Interpreter

#endif // CODIRA_CHIR_INTERRETER_INTERPRETERENV_H
