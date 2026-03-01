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
 * This file declares the TestCompilerInstance, used for unittest.
 */

#ifndef CODIRA_FRONTEND_TESTCOMPILERINSTANCE_H
#define CODIRA_FRONTEND_TESTCOMPILERINSTANCE_H

#include "Codira/Frontend/CompilerInstance.h"
#include "Codira/Modules/PackageManager.h"
#include "Codira/Sema/TypeManager.h"

namespace Codira {
class TestCompilerInstance : public CompilerInstance {
public:
    TestCompilerInstance(CompilerInvocation& invocation, DiagnosticEngine& diag) : CompilerInstance(invocation, diag)
    {
        compileOnePackageFromSrcFiles = false;
#ifdef PROJECT_SOURCE_DIR
        // Gets the absolute path of the project from the compile parameter.
        codiraHome = FileUtil::JoinPath(FileUtil::JoinPath(PROJECT_SOURCE_DIR, "build"), "build");
#else
        // Just in case, give it a default value.
        // Assume the initial is in the build directory.
        codiraHome = FileUtil::JoinPath(FileUtil::JoinPath(".", "build"), "build");
#endif
        // create modules dir.
        auto modulesName = FileUtil::JoinPath(codiraHome, "modules");
        auto libPathName = invocation.globalOptions.GetCodiraLibTargetPathName();
        auto codiraModules = FileUtil::JoinPath(modulesName, libPathName);
        if (!FileUtil::FileExist(codiraModules)) {
            FileUtil::CreateDirs(FileUtil::JoinPath(codiraModules, ""));
        }
        CODEC_NULLPTR_CHECK(compileStrategy); // Was created in ctor of 'CompilerInstance'.
    }
    bool PerformParse() override;
    bool PerformSema() override
    {
        return compileStrategy->Sema();
    }
    bool Compile(CompileStage stage = CompileStage::GENERIC_INSTANTIATION) override;
    const auto& GetBuildOrders()
    {
        return packageManager->GetBuildOrders();
    }
    /**
     * Input source code.
     */
    std::string code;

private:
    bool ParseCode();
};
} // namespace Codira
#endif // CODIRA_FRONTEND_TESTCOMPILERINSTANCE_H
