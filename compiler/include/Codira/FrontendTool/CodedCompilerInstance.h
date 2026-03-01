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
 * This file declares the CodedCompilerInstance, which performs the default compile flow.
 */

#ifndef CODIRA_FRONTEND_CODEDCOMPILERINSTANCE_H
#define CODIRA_FRONTEND_CODEDCOMPILERINSTANCE_H

#include "Codira/FrontendTool/DefaultCompilerInstance.h"

namespace Codira {
/// Compiler instance that compiles .code.d file. This ci skips all stages after sema, except that it still produces a
/// codeo file in CodeoAndBchirSaving.
class CodedCompilerInstance : public DefaultCompilerInstance {
public:
    CodedCompilerInstance(CompilerInvocation& invocation, DiagnosticEngine& diag)
        : DefaultCompilerInstance(invocation, diag)
    {
        buildTrie = false;
    }
    ~CodedCompilerInstance() override = default;
    ///@{
    /// After sema, skip all stages until codeo saving
    bool PerformDesugarAfterSema() override
    {
        return true;
    }
    bool PerformGenericInstantiation() override
    {
        return true;
    }
    bool PerformOverflowStrategy() override
    {
        return true;
    }
    // use DefaultCompilerInstance::PerformMangling
    bool PerformCHIRCompilation() override
    {
        return true;
    }
    bool PerformCodeGen() override
    {
        return true;
    }
    bool PerformCodeoAndBchirSaving() override
    {
        Utils::ProfileRecorder recorder("Main Stage", "Save codeo");
        bool ret = true;
        for (auto& srcPkg : GetSourcePackages()) {
            ret = ret && SaveCodeo(*srcPkg);
        }
        return ret;
    }
    ///@}
};
} // namespace Codira

#endif // CODIRA_FRONTEND_CODEDCOMPILERINSTANCE_H
