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

#ifndef CODELINT_COMPILER_INSTANCE_H
#define CODELINT_COMPILER_INSTANCE_H

#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Frontend/CompilerInstance.h"
#include "Codira/Frontend/CompilerInvocation.h"
#include "common/ConfigContext.h"

namespace Codira::CodeCheck {
class CODELintCompilerInstance : public CompilerInstance {
public:
    CODELintCompilerInstance(CompilerInvocation& invocation, DiagnosticEngine& diag) : CompilerInstance(invocation, diag)
    {
        buildTrie = false;
        releaseCHIRMemory = false;
        needToOptString = true;
        needToOptGenericDecl = true;
        isCODELint = true;
    }
    bool PerformImportPackage() override;
    bool Compile(CompileStage stage) override;

    std::set<std::string> macroPath;

private:
    CompileStage currentStage = CompileStage::PARSE;
};
} // namespace Codira::CodeCheck

#endif // CODELINT_COMPILER_INSTANCE_H
