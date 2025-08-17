//===- InstallAPI/Frontend.h -----------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
///
/// Top level wrappers for InstallAPI frontend operations.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_INSTALLAPI_FRONTEND_H
#define LANGUAGE_CORE_INSTALLAPI_FRONTEND_H

#include "language/Core/AST/ASTConsumer.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Frontend/FrontendActions.h"
#include "language/Core/InstallAPI/Context.h"
#include "language/Core/InstallAPI/DylibVerifier.h"
#include "language/Core/InstallAPI/Visitor.h"
#include "toolchain/ADT/Twine.h"
#include "toolchain/Support/MemoryBuffer.h"

namespace language::Core {
namespace installapi {

/// Create a buffer that contains all headers to scan
/// for global symbols with.
std::unique_ptr<toolchain::MemoryBuffer> createInputBuffer(InstallAPIContext &Ctx);

class InstallAPIAction : public ASTFrontendAction {
public:
  explicit InstallAPIAction(InstallAPIContext &Ctx) : Ctx(Ctx) {}

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    Ctx.Diags->getClient()->BeginSourceFile(CI.getLangOpts());
    Ctx.Verifier->setSourceManager(CI.getSourceManagerPtr());
    return std::make_unique<InstallAPIVisitor>(
        CI.getASTContext(), Ctx, CI.getSourceManager(), CI.getPreprocessor());
  }

private:
  InstallAPIContext &Ctx;
};
} // namespace installapi
} // namespace language::Core

#endif // LANGUAGE_CORE_INSTALLAPI_FRONTEND_H
