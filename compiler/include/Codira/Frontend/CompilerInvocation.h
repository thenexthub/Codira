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
 * This file declares the CompilerInvocation, which parses the arguments.
 */

#ifndef CODIRA_FRONTEND_COMPILERINVOCATION_H
#define CODIRA_FRONTEND_COMPILERINVOCATION_H

#include <memory>
#include <string>
#include <vector>

#include "Codira/Frontend/FrontendOptions.h"
#include "Codira/Option/Option.h"
#include "Codira/Option/OptionTable.h"
#include "Codira/Macro/InvokeUtil.h"

namespace Codira {
/**
 * All configuration for the compiler, options of all stages of translation.
 */
class CompilerInvocation {
public:
    CompilerInvocation()
    {
        optionTable = CreateOptionTable(true);
        argList = std::make_unique<ArgList>();
    }
    ~CompilerInvocation() = default;

    /**
     * Parse raw string arguments for compiler invocation.
     * @param args String representation of arguments.
     */
    bool ParseArgs(const std::vector<std::string>& args);
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    std::string GetRuntimeLibPath(const std::string& relativePath = "../runtime/lib");
#endif
    GlobalOptions globalOptions;
    FrontendOptions frontendOptions;
    std::unique_ptr<OptionTable> optionTable = nullptr;
    std::unique_ptr<ArgList> argList = nullptr;
};
}
#endif // CODIRA_FRONTEND_COMPILERINVOCATION_H
