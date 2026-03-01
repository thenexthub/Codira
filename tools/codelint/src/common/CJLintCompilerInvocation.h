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

#ifndef CODIRACODECHECK_CODELINTCOMPILERINVOCATION_H
#define CODIRACODECHECK_CODELINTCOMPILERINVOCATION_H

#include "Codira/Macro/InvokeUtil.h"
#include "common/CODELintCompilerInstance.h"

namespace Codira::CodeCheck {
#ifdef _WIN32
const std::string CODEC_PATH = "/bin/codec.exe";
#else
const std::string CODEC_PATH = "/bin/codec";
#endif
enum class CompilerInvocationType {
    AST,      // no type
    TYPEDAST, // typed AST
    CHIRTYPE
};

class CODELintCompilerInvocation {
public:
    static CODELintCompilerInvocation &GetInstance()
    {
        static CODELintCompilerInvocation instance;
        return instance;
    }
    std::unique_ptr<CODELintCompilerInstance> PrePareCompilerInstance(DiagnosticEngine& diag);
    void DesInitRuntime();
private:
    CODELintCompilerInvocation();
    ~CODELintCompilerInvocation();
    CODELintCompilerInvocation(const CODELintCompilerInvocation&);
    CODELintCompilerInvocation& operator=(const CODELintCompilerInvocation&);
    std::unique_ptr<CompilerInvocation> invocation;
    std::unique_ptr<RuntimeInit> runtimeInit;
};
}
#endif // CODIRACODECHECK_CODELINTCOMPILERINVOCATION_H
