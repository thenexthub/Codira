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
 * This file declares the DefaultCompilerInstance, which performs the default compile flow.
 */

#ifndef CODIRA_FRONTEND_DEFAULTCOMPILERINSTANCE_H
#define CODIRA_FRONTEND_DEFAULTCOMPILERINSTANCE_H

#include "Codira/Frontend/CompilerInstance.h"

namespace Codira {
namespace CodeGen {
    class CGModule;
}
class DefaultCompilerInstance : public CompilerInstance {
public:
    DefaultCompilerInstance(CompilerInvocation& invocation, DiagnosticEngine& diag);
    ~DefaultCompilerInstance() override;
    bool PerformParse() override;
    bool PerformImportPackage() override;
    bool PerformConditionCompile() override;
    bool PerformMacroExpand() override;
    bool PerformSema() override;
    bool PerformOverflowStrategy() override;
    bool PerformDesugarAfterSema() override;
    bool PerformGenericInstantiation() override;
    bool PerformCHIRCompilation() override;
    bool PerformCodeGen() override;
    bool PerformCodeoAndBchirSaving() override;

    bool PerformMangling() override;
    void DumpDepPackage();

protected:
    bool SaveCodeoAndBchir(AST::Package& pkg) const;
    bool SaveCodeo(const AST::Package& pkg) const;
    void RearrangeImportedPackageDependence() const;
    bool CodegenOnePackage(AST::Package& pkg, bool enableIncrement) const;

private:
    class DefaultCIImpl* impl;
};
} // namespace Codira

#endif // CODIRA_FRONTEND_DEFAULTCOMPILERINSTANCE_H
